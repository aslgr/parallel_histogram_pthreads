#include <stdlib.h>
#include <limits.h>
#include "data_generation.h"
#include "build_limits.h"

static int cmp(const void *a, const void *b)
{
    long long x = *(const long long *)a;
    long long y = *(const long long *)b;

    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

int build_limits(const long long *data, long long nelements, int npivots,
                 int nbins, long long *pivots, long long *limits)
{

    if (data == NULL || pivots == NULL || limits == NULL)
        return -1;

    long long stride = nelements / npivots;

    for (int i = 0; i < npivots; i++)
    {
        long long jitter = (long long)(random_63() % (unsigned long long)stride);
        long long idx = (long long)i * stride + jitter;
        pivots[i] = data[idx];
    }

    qsort(pivots, npivots, sizeof(long long), cmp);

    limits[0] = LLONG_MIN;
    limits[nbins] = LLONG_MAX;

    for (int k = 1; k < nbins; k++)
    {
        int idx = (k * npivots) / nbins;

        if (idx >= npivots)
            idx = npivots - 1;

        limits[k] = pivots[idx];
    }

    for (int k = 1; k < nbins; k++)
    {
        if (limits[k] <= limits[k - 1]) {

            if (limits[k - 1] < LLONG_MAX - 1)
                limits[k] = limits[k - 1] + 1;
            else
                limits[k] = limits[k - 1];
        }
    }

    return 0;
}