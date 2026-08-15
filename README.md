# Parallel Histogram with Pthreads

A parallel histogram implementation in C using **POSIX Threads (Pthreads)** and a **persistent thread pool**, with a complete benchmarking workflow for evaluating scalability, throughput, and correctness.

The project explores shared-memory parallelism by distributing a large input array across multiple threads while avoiding contention through private per-thread histograms.

## Overview

The program receives an array of `long long` values and divides the value space into multiple bins.

The bin boundaries are estimated from a set of sampled pivots, and each input element is then classified into one of those bins.

The implementation compares:

* a pure single-threaded histogram;
* a parallel histogram using multiple threads;
* execution time and speedup between both versions;
* throughput in millions of elements per second (MEPS);
* parallel efficiency;
* correctness of the generated histogram.

For parallel execution, the program uses a **persistent thread pool**, avoiding the overhead of repeatedly creating and destroying threads between benchmark rounds.

## Key Features

* Parallel histogram computation with Pthreads
* Persistent thread pool
* Main thread participates as worker thread 0
* Private histogram for each thread
* Final reduction of private histograms
* Binary search for bin classification
* Balanced workload distribution across threads
* Serial reference implementation
* Three-stage correctness verification
* Optional balanced input generation with `-tb2`
* Cache eviction before timed sections
* Multiple benchmark rounds
* Speedup, throughput, and parallel efficiency measurements
* Modular C codebase with separate responsibilities

## Project Structure

```
parallel-histogram/
├── docs/
│   └── planilhas_histo.ods
├── include/
│   ├── build_limits.h
│   ├── data_generation.h
│   └── histogram.h
├── scripts/
│   ├── run_4m.sh
│   ├── run_8m.sh
│   ├── run_16m_1-4.sh
│   └── run_16m_5-8.sh
├── src/
│   ├── build_limits.c
│   ├── data_generation.c
│   ├── histogram.c
│   └── main.c
└── Makefile
```

### `src/main.c`

Controls the complete execution and benchmarking workflow.

It is responsible for:

* parsing command-line arguments;
* validating parameters;
* allocating the required memory;
* generating input data;
* performing cache eviction;
* measuring execution time;
* calling the serial and parallel histogram implementations;
* calculating speedup, MEPS, and efficiency;
* printing benchmark results.

Timing is performed with `clock_gettime()` using `CLOCK_MONOTONIC`.

### `src/data_generation.c`

Implements the input generation strategies.

Without `-tb2`, the program generates pseudo-random signed `long long` values.

With `-tb2`, a balanced synthetic input is produced by:

1. filling the array with `0, 1, 2, ..., nelements - 1`;
2. shuffling the values using the Fisher-Yates algorithm;
3. reducing each value modulo `nbins`.

This creates a randomized input distributed across the interval `[0, nbins)` and was the mode used during the scalability experiments.

### `src/build_limits.c`

Builds the boundaries used to divide the input values into bins.

The algorithm:

1. divides the input into sampling regions;
2. selects one pivot from each region using a random jitter;
3. sorts the sampled pivots with `qsort`;
4. selects internal bin boundaries from the sorted pivots;
5. uses `LLONG_MIN` and `LLONG_MAX` as the outer boundaries;
6. adjusts repeated internal limits to preserve increasing boundaries.

The limits are constructed serially and measured independently from the histogram computation.

### `src/histogram.c`

Contains the main parallel implementation.

For a single thread, the input array is processed directly by the calling thread without using the thread pool.

For multiple threads, a persistent pool is initialized once and reused across benchmark rounds.

The main thread acts as worker thread 0, while `nthreads - 1` additional Pthreads are created.

Each worker receives a contiguous portion of the input array and maintains its own private histogram.

Because threads do not update the same counters during the main computation, no mutex is required for individual histogram increments.

After all threads finish, the private histograms are reduced into the final result.

## Parallel Execution

For `N` threads, the input is divided as evenly as possible.

If the number of elements is not exactly divisible by the number of threads, the remaining elements are distributed among the first workers.

Conceptually:

```
Input array
    |
    |-- Thread 0 -> private histogram 0
    |-- Thread 1 -> private histogram 1
    |-- Thread 2 -> private histogram 2
    |      ...
    `-- Thread N -> private histogram N
                         |
                         v
                 Final reduction
                         |
                         v
                   Final histogram
```

Two barriers coordinate each parallel operation:

1. the first barrier starts the current task;
2. the second barrier waits until every worker finishes processing its portion.

The worker threads then wait for the next task instead of being recreated for every benchmark round.

## Bin Classification

Each value is assigned to a bin using binary search over the calculated limits.

For a bin `b`, the interval follows:

```
limits[b] <= value < limits[b + 1]
```

Using binary search avoids scanning every bin for every input element.

## Correctness Verification

Correctness is checked after every benchmark round using three independent stages.

### Stage 1 - Serial vs parallel

Every bin generated by the parallel implementation is compared with the histogram produced using one thread.

### Stage 2 - Independent recount

The input is processed again using a separate serial recount.

This verification intentionally uses a linear search through the limits instead of the binary-search function used by the main histogram implementation.

### Stage 3 - Total element count

The final histogram is summed and checked against the original number of input elements.

A successful execution ends with:

```
Overall correctness : PASS
```

All configurations used in the benchmark experiments passed the correctness verification.

## Building

The project requires a C compiler with POSIX Threads support.

Compile using:

```bash
make
```

This generates:

```bash
parallel-histogram
```

To remove the executable:

```bash
make clean
```

## Usage

```
./parallel-histogram <nelements> <npivots> <nbins> <nthreads> <nr> [-tb2]
```

### Arguments

| Argument    | Description                                   |
| ----------- | --------------------------------------------- |
| `nelements` | Number of elements in the input array         |
| `npivots`   | Number of samples used to estimate bin limits |
| `nbins`     | Number of histogram bins                      |
| `nthreads`  | Number of threads used by the histogram       |
| `nr`        | Number of benchmark rounds                    |
| `-tb2`      | Optional balanced input generation mode       |

The implementation supports up to 64 threads.

### Example

Run the histogram using 8 million elements, 1024 pivots, 32 bins, 8 threads, and 10 rounds:

```bash
./parallel-histogram 8000000 1024 32 8 10 -tb2
```

## Performance Metrics

Each benchmark round measures three sections independently:

* `T(bl_ser)` - serial `build_limits` execution time;
* `T(1 thr)` - histogram execution time using one thread;
* `T(N thr)` - histogram execution time using the requested number of threads.

Histogram speedup is calculated as:

```
Speedup = T(1 thread) / T(N threads)
```

Parallel efficiency is calculated as:

```
Efficiency = Speedup / N
```

Throughput is reported in millions of elements processed per second:

```
MEPS = nelements / execution_time / 1,000,000
```

The serial limit-building phase is measured separately and is not included in the histogram speedup.

## Cache Control

Before each timed section, the program touches an 18 MiB buffer to reduce the influence of data left in cache by the previous measurement.

The benchmark uses a 6 MiB cache-size reference and an eviction buffer three times that size.

Cache eviction is performed before measuring:

* bin-limit construction;
* the single-threaded histogram;
* the parallel histogram.

## Benchmark Methodology

Scalability experiments were performed with:

| Parameter   | Values               |
| ----------- | -------------------- |
| Input sizes | 4M, 8M, 16M elements |
| Pivots      | 1024                 |
| Bins        | 32                   |
| Threads     | 1 to 8               |
| Rounds      | 10 per configuration |
| Input mode  | `-tb2`               |

The experiments were automated using shell scripts for the three input sizes.

The 16-million-element experiment was divided into two scripts covering threads 1-4 and 5-8 due to execution-time restrictions on the test environment.

Hardware information was recorded using `lscpu`, while the machine topology was captured using `lstopo`.

The benchmark runs were executed exclusively on a single machine to reduce interference from other workloads.

## Experimental Environment

The reported results were collected on the following machine:

| Component        | Configuration               |
| ---------------- | --------------------------- |
| Host             | `wz10`                      |
| Architecture     | x86_64                      |
| Processor        | Intel Xeon E5410 @ 2.33 GHz |
| CPUs             | 8                           |
| Sockets          | 2                           |
| Cores per socket | 4                           |
| Threads per core | 1                           |

## Benchmark Results

### Average Speedup

| Threads |        4M |        8M |       16M |
| ------: | --------: | --------: | --------: |
|       1 |     0.996 |     0.998 |     1.002 |
|       2 |     1.826 |     1.797 |     1.809 |
|       3 |     2.547 |     2.582 |     2.555 |
|       4 |     3.118 |     3.145 |     3.141 |
|       5 |     3.743 |     3.748 |     3.768 |
|       6 |     4.364 |     4.297 |     4.289 |
|       7 |     4.876 |     4.749 |     4.765 |
|       8 | **5.244** | **5.515** | **5.440** |

The program scales consistently as additional threads are introduced.

The best speedup observed was **5.515x**, using 8 threads with an input containing 8 million elements.

Scaling is sublinear, which is expected because parallel execution still includes synchronization, final histogram reduction, and shared-memory access costs.

### Average Execution Time

With 8 threads:

| Input size | Average time |
| ---------: | -----------: |
|         4M |   0.031565 s |
|         8M |   0.060272 s |
|        16M |   0.121887 s |

For comparison, the single-threaded executions were approximately:

* 0.165 s for 4M elements;
* 0.331 s for 8M elements;
* 0.660 s for 16M elements.

The largest improvements occur with the first additional threads. Performance continues to improve afterward, although each additional thread provides a smaller incremental gain.

### Throughput

Average throughput at 8 threads:

| Input size |      Throughput |
| ---------: | --------------: |
|         4M |     126.72 MEPS |
|         8M | **132.73 MEPS** |
|        16M |     131.27 MEPS |

The 8-million-element configuration achieved the highest measured throughput at **132.73 million elements per second**.

For comparison, the single-thread configurations processed approximately 24 MEPS.

## Results Analysis

The three input sizes showed very similar scalability behavior.

This is especially visible in the speedup results, where the curves remain close to one another as the number of threads increases.

At 8 threads, all three workloads reached speedups between approximately 5.2x and 5.5x.

The results indicate that:

* the parallel implementation successfully reduces histogram execution time;
* throughput increases consistently with additional threads;
* performance remains stable as the input size increases;
* the persistent thread pool avoids repeated thread-creation overhead;
* private histograms provide thread-safe counting without requiring synchronization on every increment;
* scalability eventually becomes limited by synchronization, reduction overhead, and memory-system behavior.

Most importantly, every measured configuration completed with a successful correctness check.

## Benchmark Data

The complete experimental spreadsheet contains:

* hardware information;
* raw results for each thread configuration;
* measurements from all benchmark rounds;
* average execution times;
* speedup calculations;
* parallel efficiency;
* throughput in MEPS;
* correctness results;
* summary tables;
* scalability charts.

The benchmark data was used to compare the behavior of the implementation across 4M, 8M, and 16M input sizes and from 1 to 8 threads.