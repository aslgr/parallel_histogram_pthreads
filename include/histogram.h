#ifndef HISTOGRAM_H
#define HISTOGRAM_H
#define MAX_THREADS 64

int parallel_histogram(const long long *data, long long nelements, const long long *limits,
                       int nbins, long long *hist, int nthreads);

int verify_histogram(const long long *data, long long nelements, const long long *limits,
                     int nbins, const long long *hist_1thr, const long long *hist_nthr);

#endif