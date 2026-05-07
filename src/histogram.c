#include <stdlib.h>
#include <pthread.h>
#include "histogram.h"

typedef struct {
    const long long *data;
    long long start;
    long long end;
    const long long *limits;
    int nbins;
    long long *private_hist;
} ThreadArgs;

static int find_bin_binary(
    long long value,
    const long long *limits,
    int nbins) {
    
    int left = 0;
    int right = nbins - 1;

    while (left <= right)
    {
        int mid = left + (right - left) / 2;

        if (value < limits[mid]) {
            right = mid - 1;
        }
        else if (mid < nbins - 1 && value >= limits[mid + 1]) {
            left = mid + 1;
        }
        else {
            return mid;
        }
    }

    return -1;
}

static void compute_histogram_range(
    const long long *data,
    long long start,
    long long end,
    const long long *limits,
    int nbins,
    long long *hist) {
    
    for (int b = 0; b < nbins; b++)
        hist[b] = 0;

    for (long long i = start; i < end; i++)
    {
        int bin = find_bin_binary(data[i], limits, nbins);

        if (bin >= 0)
            hist[bin]++;
    }
}

static void *thread_histogram(void *arg) {

    ThreadArgs *args = (ThreadArgs *)arg;

    compute_histogram_range(
        args->data,
        args->start,
        args->end,
        args->limits,
        args->nbins,
        args->private_hist);

    return NULL;
}

int parallel_histogram(
    const long long *data,
    long long nelements,
    const long long *limits,
    int nbins,
    long long *hist,
    int nthreads) {
    
    if (data == NULL || limits == NULL || hist == NULL)
        return -1;

    if (nelements < 0 || nbins <= 0 || nthreads < 1)
        return -1;

    if (nthreads == 1) {
        compute_histogram_range(data, 0, nelements, limits, nbins, hist);
        return 0;
    }

    for (int b = 0; b < nbins; b++)
        hist[b] = 0;

    pthread_t *threads = malloc(nthreads * sizeof(pthread_t));
    ThreadArgs *args = malloc(nthreads * sizeof(ThreadArgs));
    long long *private_hists = malloc((long long)nthreads * nbins * sizeof(long long));

    if (threads == NULL || args == NULL || private_hists == NULL) {
        free(threads);
        free(args);
        free(private_hists);
        return -1;
    }

    long long base_chunk = nelements / nthreads;
    long long remainder = nelements % nthreads;

    long long start = 0;

    for (int t = 0; t < nthreads; t++)
    {
        long long chunk_size = base_chunk + (t < remainder ? 1 : 0);
        long long end = start + chunk_size;

        args[t].data = data;
        args[t].start = start;
        args[t].end = end;
        args[t].limits = limits;
        args[t].nbins = nbins;
        args[t].private_hist = private_hists + ((long long)t * nbins);

        if (pthread_create(&threads[t], NULL, thread_histogram, &args[t]) != 0) {
            free(threads);
            free(args);
            free(private_hists);
            return -1;
        }

        start = end;
    }

    for (int t = 0; t < nthreads; t++)
        pthread_join(threads[t], NULL);

    for (int t = 0; t < nthreads; t++)
    {
        long long *local_hist = private_hists + ((long long)t * nbins);

        for (int b = 0; b < nbins; b++)
            hist[b] += local_hist[b];
    }

    free(threads);
    free(args);
    free(private_hists);

    return 0;
}