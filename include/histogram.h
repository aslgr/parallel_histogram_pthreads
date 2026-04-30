#ifndef HISTOGRAM_H
#define HISTOGRAM_H

int serial_histogram(
    const long long *data,
    long long nelements,
    const long long *limits,
    int nbins,
    long long *hist);

#endif