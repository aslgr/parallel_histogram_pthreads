#include <stdlib.h>
#include <limits.h>
#include "build_limits.h"

static int cmp(const void *a, const void *b) {

    long long x = *(const long long *)a;
    long long y = *(const long long *)b;

    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

int build_limits(
    const long long *data,
    long long nelements,
    int npivots,
    int nbins,
    long long *limits) {

    long long *pivots = malloc(npivots * sizeof(long long));

    if (pivots == NULL) 
        return -1;

    long long stride = nelements / npivots;

    for (int i = 0; i < npivots; i++)
    {
        long long jitter = rand() % stride;
        long long idx = i * stride + jitter;
        pivots[i] = data[idx];
    }

    qsort(pivots, npivots, sizeof(long long), cmp);

    for (int i = 0; i <= nbins; i++)
    {
        int idx = i * (npivots - 1) / nbins;
        limits[i] = pivots[idx];
    }

    limits[0] = LLONG_MIN;
    limits[nbins] = LLONG_MAX;

    for (int i = 1; i <= nbins; i++)
    {
        if (limits[i] <= limits[i - 1])
            limits[i] = limits[i - 1] + 1;
    }
    
    free(pivots);
    return 0;
}