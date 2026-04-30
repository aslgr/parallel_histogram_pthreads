#include "histogram.h"

int serial_histogram(
    const long long *data,
    long long nelements,
    const long long *limits,
    int nbins,
    long long *hist) {

    for (int b = 0; b < nbins; b++)
        hist[b] = 0;

    for (long long i = 0; i < nelements; i++) 
    {
        long long v = data[i];

        for (int b = 0; b < nbins; b++) 
        {
            if (v >= limits[b] && v < limits[b + 1]) {
                hist[b]++;
                break;
            }
        }
    }

    return 0;
}