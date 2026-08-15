#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "histogram.h"

static pthread_t hist_threads[MAX_THREADS];
static int hist_thread_id[MAX_THREADS];

static pthread_barrier_t hist_barrier;

static int hist_initialized = 0;
static int hist_pool_nthreads = 0;

static const long long *hist_pool_data = NULL;
static const long long *hist_pool_limits = NULL;
static long long hist_pool_nelements = 0;
static int hist_pool_nbins = 0;

static long long *hist_pool_private_hists = NULL;

static int find_bin(long long value, const long long *limits, int nbins)
{
    int left = 0;
    int right = nbins;

    while (left < right) {

        int mid = (left + right) / 2;

        if (value < limits[mid])
            right = mid;
        else
            left = mid + 1;
    }

    int bin = left - 1;

    if (bin < 0)
        bin = 0;

    if (bin >= nbins)
        bin = nbins - 1;

    return bin;
}

static void *hist_pool_worker(void *ptr) {

    int tid = *((int *)ptr);

    while (1) {

        pthread_barrier_wait(&hist_barrier);

        long long base_chunk =
            hist_pool_nelements / hist_pool_nthreads;

        long long remainder =
            hist_pool_nelements % hist_pool_nthreads;

        long long start;
        long long end;

        if (tid < remainder) {

            start = tid * (base_chunk + 1);
            end = start + base_chunk + 1;

        } else {

            start =
                remainder * (base_chunk + 1)
                + (tid - remainder) * base_chunk;

            end = start + base_chunk;
        }

        long long *local_hist =
            hist_pool_private_hists
            + ((long long)tid * hist_pool_nbins);

        for (int b = 0; b < hist_pool_nbins; b++)
            local_hist[b] = 0;

        for (long long i = start; i < end; i++) {

            int b = find_bin(
                hist_pool_data[i],
                hist_pool_limits,
                hist_pool_nbins);

            local_hist[b]++;
        }

        pthread_barrier_wait(&hist_barrier);

        if (tid == 0)
            return NULL;
    }

    return NULL;
}

static int hist_pool_init(int nthreads, int nbins) {

    hist_pool_nthreads = nthreads;
    hist_pool_nbins = nbins;

    hist_pool_private_hists =
        calloc(
            (size_t)nthreads * nbins,
            sizeof(long long));

    if (!hist_pool_private_hists)
        return -1;

    if (pthread_barrier_init(
            &hist_barrier,
            NULL,
            nthreads) != 0)
        return -1;

    hist_thread_id[0] = 0;

    for (int i = 1; i < nthreads; i++) {

        hist_thread_id[i] = i;

        if (pthread_create(
                &hist_threads[i],
                NULL,
                hist_pool_worker,
                &hist_thread_id[i]) != 0) {

            return -1;
        }
    }

    hist_initialized = 1;

    return 0;
}

int parallel_histogram(const long long *data, long long nelements, const long long *limits,
                       int nbins, long long *hist, int nthreads) 
{
    if (data == NULL || limits == NULL || hist == NULL)
        return -1;

    for (int i = 0; i < nbins; i++)
        hist[i] = 0;

    if (nthreads == 1) {
        for (long long i = 0; i < nelements; i++) 
        {
            int b = find_bin(data[i], limits, nbins);
            hist[b]++;
        }

        return 0;
    }

    if (!hist_initialized) {
        if (hist_pool_init(nthreads, nbins) != 0)
            return -1;
    }

    if (nthreads != hist_pool_nthreads || nbins != hist_pool_nbins)
        return -1;

    hist_pool_data = data;
    hist_pool_limits = limits;
    hist_pool_nelements = nelements;

    hist_pool_worker(&hist_thread_id[0]);

    for (int t = 0; t < nthreads; t++) 
    {
        long long *local_hist = hist_pool_private_hists + ((long long)t * nbins);

        for (int b = 0; b < nbins; b++)
            hist[b] += local_hist[b];
    }

    return 0;
}

int verify_histogram(const long long *data, long long nelements, const long long *limits,
                     int nbins, const long long *hist_1thr, const long long *hist_nthr)
{
    int s1_ok = 1;

    for (int b = 0; b < nbins; b++) 
    {
        if (hist_1thr[b] != hist_nthr[b]) {
            fprintf(stderr," VERIFY FAIL stage 1: bin %d 1thr=%lld Nthr=%lld\n",
                    b, hist_1thr[b], hist_nthr[b]);

            s1_ok = 0;
        }
    }

    if (!s1_ok)
        return 0;

    long long *recount = calloc(nbins, sizeof(long long));

    if (!recount) {
        perror("calloc recount");
        return 0;
    }

    for (long long i = 0; i < nelements; i++)
    {
        long long v = data[i];
        int b = 0;

        while (b < nbins - 1 && v >= limits[b + 1])
            b++;

        recount[b]++;
    }

    int s2_ok = 1;

    for (int b = 0; b < nbins; b++)
    {
        if (recount[b] != hist_nthr[b]) {

            fprintf(stderr," VERIFY FAIL stage 2: bin %d recount=%lld Nthr=%lld\n",
                    b, recount[b], hist_nthr[b]);

            s2_ok = 0;
        }
    }

    free(recount);

    if (!s2_ok)
        return 0;

    long long total = 0;

    for (int b = 0; b < nbins; b++)
        total += hist_nthr[b];

    if (total != nelements) {
        fprintf(stderr," VERIFY FAIL stage 3: sum=%lld expected=%lld\n",
                total, nelements);

        return 0;
    }

    return 1;
}