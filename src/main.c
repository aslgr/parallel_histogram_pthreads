#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include "data_generation.h"
#include "histogram.h"
#include "build_limits.h"

static double get_time(void) {

    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

static void evict_cache(char *buffer, size_t size)
{
    for (size_t i = 0; i < size; i++)
        buffer[i]++;
}

int main(int argc, char **argv) {

    int flag_tb2 = 0;

    if (argc == 7) {
        if (strcmp(argv[6], "-tb2") == 0) {
            flag_tb2 = 1;
        } else {
            fprintf(stderr, "Error: option not recognized '%s'.\n", argv[6]);
            fprintf(stderr, "Usage: parallel-histogram <nelements> <npivots> "
                    "<nbins> <nthreads> <nr> [-tb2]\n");

            return 1;
        }
    } else if (argc != 6) {
        fprintf(stderr, "Usage: parallel-histogram <nelements> <npivots> "
                "<nbins> <nthreads> <nr> [-tb2]\n");

        return 1;
    }

    long long nelements = strtoll(argv[1], NULL, 10);
    int npivots = atoi(argv[2]);
    int nbins = atoi(argv[3]);
    int nthreads = atoi(argv[4]);
    int nr = atoi(argv[5]);

    if (nelements <= 0) {
        fprintf(stderr, "Error: nelements must be > 0.\n");
        return 1;
    }

    if (nbins < 1) {
        fprintf(stderr, "Error: nbins must be >= 1.\n");
        return 1;
    }

    if (npivots < 2 || npivots < nbins || npivots > nelements) {
        fprintf(stderr, "Error: npivots must be >= 2, >= nbins, and <= nelements.\n");
        return 1;
    }

    if (nthreads < 1 || nthreads > MAX_THREADS) {
        fprintf(stderr, "Error: nthreads must be >= 1 and <= %d.\n", MAX_THREADS);
        return 1;
    }

    if (nr < 1) {
        fprintf(stderr, "Error: nr must be >= 1.\n");
        return 1;
    }

    long long *data_1thr = malloc(nelements * sizeof(long long));
    long long *data_nthr = malloc(nelements * sizeof(long long));
    long long *limits = malloc((nbins + 1) * sizeof(long long));
    long long *hist_1thr = malloc(nbins * sizeof(long long));
    long long *hist_nthr = malloc(nbins * sizeof(long long));
    long long *pivots = malloc(npivots * sizeof(long long));

    size_t LLC = 6 * 1024 * 1024;
    size_t buf_size = 3 * LLC;

    char *evict_buf = malloc(buf_size);

    if (!data_1thr || !data_nthr || !limits || !hist_1thr ||
        !hist_nthr || !pivots || !evict_buf) {
        fprintf(stderr, "Error: could not allocate memory.\n");
        free(data_1thr);
        free(data_nthr);
        free(limits);
        free(hist_1thr);
        free(hist_nthr);
        free(pivots);
        free(evict_buf);
        return 1;
    }

    memset(evict_buf, 0, buf_size);

    double time_0, time_1, time_build_limits, time_1thr, time_nthr;
    double speedup;
    double sum_build, sum_1thr, sum_nthr, sum_speedup;
    int ok, global_ok;

    sum_build = sum_1thr = sum_nthr = sum_speedup = 0.0;
    ok = global_ok = 1;

    srand(time(NULL));

    printf("\n=== Parallel Histogram — Scalability Test (Persistent Thread Pool) ===\n");
    printf("Elements : %-10lld | Pivots : %-5d | Bins : %-3d | Threads : %-3d | "
           "Rounds : %-3d | Input : %s\n", nelements, npivots, nbins, nthreads, nr,
            flag_tb2 ? "uniform random in [0,nbins) (-tb2)" : "uniform random long long");
    printf("LLC size : 6 MiB | Eviction buffer : 18 MiB\n");

    for (int round = 1; round <= nr; round++)
    {
        if (generate_data(data_1thr, nelements, nbins, flag_tb2) != 0) {
            fprintf(stderr, "Error: generate_data failed.\n");
            free(data_1thr);
            free(data_nthr);
            free(limits);
            free(hist_1thr);
            free(hist_nthr);
            free(pivots);
            free(evict_buf);
            return 1;
        }

        memcpy(data_nthr, data_1thr, nelements * sizeof(long long));

        evict_cache(evict_buf, buf_size);

        time_0 = get_time();

        if (build_limits(data_1thr, nelements, npivots, nbins, pivots, limits) != 0) {
            fprintf(stderr, "Error: build_limits failed.\n");
            free(data_1thr);
            free(data_nthr);
            free(limits);
            free(hist_1thr);
            free(hist_nthr);
            free(pivots);
            free(evict_buf);
            return 1;
        }

        time_1 = get_time();

        time_build_limits = time_1 - time_0;

        evict_cache(evict_buf, buf_size);

        time_0 = get_time();

        if (parallel_histogram(data_1thr, nelements, limits, nbins, hist_1thr, 1) != 0) {
            fprintf(stderr, "Error: serial_histogram failed.\n");
            free(data_1thr);
            free(data_nthr);
            free(limits);
            free(hist_1thr);
            free(hist_nthr);
            free(pivots);
            free(evict_buf);
            return 1;
        }

        time_1 = get_time();

        time_1thr = time_1 - time_0;

        evict_cache(evict_buf, buf_size);

        time_0 = get_time();

        if (parallel_histogram(data_nthr, nelements, limits, nbins, hist_nthr, nthreads) != 0) {
            fprintf(stderr, "Error: parallel_histogram failed.\n");
            free(data_1thr);
            free(data_nthr);
            free(limits);
            free(hist_1thr);
            free(hist_nthr);
            free(pivots);
            free(evict_buf);
            return 1;
        }

        time_1 = get_time();

        time_nthr = time_1 - time_0;

        speedup = time_1thr / time_nthr;

        ok = verify_histogram(data_nthr, nelements, limits, nbins, hist_1thr, hist_nthr);

        sum_build += time_build_limits;
        sum_1thr += time_1thr;
        sum_nthr += time_nthr;
        sum_speedup += speedup;

        if (!ok)
            global_ok = 0;

        if (round == 1) {
            printf("\n--- Round 1: first 8 partitions ---\n");
            printf(" Bin ; %20s ; %20s ; %12s\n", "Lo (inclusive)", "Hi (exclusive)", "Count");

            int show = nbins < 8 ? nbins : 8;

            for (int b = 0; b < show; b++) {
                printf("%4d ; %20lld ; %20lld ; %12lld\n",
                       b, limits[b], limits[b + 1], hist_1thr[b]);
            }

            if (nbins > 8)
                printf(" ... (%d more bins not shown)\n", nbins - 8);

            printf("\nRound ; %12s ; %12s ; %12s ; %10s ; %-9s\n",
           "T(bl_ser) s", "T(1 thr) s", "T(N thr) s", "Speedup", "OK?");
            printf("----- ; ------------ ; ------------ ; ------------ ; "
                   "---------- ; ---------\n");
        }

        printf("%-5d ; %12.6f ; %12.6f ; %12.6f ; %10.3f ; %-9s\n",
               round, time_build_limits, time_1thr, time_nthr, 
               speedup, ok ? "OK" : "FAIL");
    }

    double avg_build = sum_build / nr;
    double avg_1thr = sum_1thr / nr;
    double avg_nthr = sum_nthr / nr;
    double avg_speedup = sum_speedup / nr;

    double meps_1thr = nelements / (avg_1thr * 1000000.0);
    double meps_nthr = nelements / (avg_nthr * 1000000.0);

    double efficiency = (avg_speedup / nthreads) * 100.0;

    printf("----- ; ------------ ; ------------ ; ------------ ; ---------- ; ---------\n");

    printf("%-5s ; %12.6f ; %12.6f ; %12.6f ; %10.3f ; %-9s\n",
           "AVG", avg_build, avg_1thr, avg_nthr, avg_speedup, global_ok ? "OK" : "FAIL");

    printf("\n=== Summary ===\n");
    printf("\n Avg build_limits serial : %.6f ; s ; "
           "%.1f%% ; of T(1 thr) | %.1f%% ; of T(N thr)\n",
           avg_build, (avg_build / avg_1thr) * 100.0, (avg_build / avg_nthr) * 100.0);

    printf("\n Avg time (1 thread ) : %.6f ; s ; %.2f ; MEPS\n", avg_1thr, meps_1thr);

    printf(" Avg time (%d threads) : %.6f ; s ; %.2f ; MEPS\n",
           nthreads, avg_nthr, meps_nthr);

    printf(" Avg histogram speedup : %.3fx\n", avg_speedup);

    printf("\n Parallel efficiency:\n");
    printf("  with nthreads (%d) : %.1f%%\n", nthreads, efficiency);

    printf("\n Overall correctness : %s\n\n", global_ok ? "PASS" : "FAIL");

    free(data_1thr);
    free(data_nthr);
    free(limits);
    free(hist_1thr);
    free(hist_nthr);
    free(pivots);
    free(evict_buf);

    return 0;
}