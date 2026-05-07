#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include "data_generation.h"
#include "histogram.h"
#include "build_limits.h"

int main(int argc, char **argv) {

    if (argc != 6) {
        fprintf(stderr, "Usage: %s <nelements> <npivots> <nbins> <nthreads> <nr>\n", argv[0]);
        return 1;
    }

    long long nelements = atoll(argv[1]);
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

    if (nthreads < 1) {
        fprintf(stderr, "Error: nthreads must be >= 1.\n");
        return 1;
    }

    if (nr < 1) {
        fprintf(stderr, "Error: nr must be >= 1.\n");
        return 1;
    }

    srand(time(NULL));

    long long *data = generate_data(nelements);

    if (data == NULL) {
        fprintf(stderr, "Error: could not allocate data array.\n");
        return 1;
    }

    long long *limits = malloc((nbins + 1) * sizeof(long long));

    if (limits == NULL) {
        fprintf(stderr, "Error: could not allocate limits array.\n");
        free(data);
        return 1;
    }

    if (build_limits(data, nelements, npivots, nbins, limits) != 0) {
        fprintf(stderr, "Error: build_limits failed.\n");
        free(data);
        free(limits);
        return 1;
    }

    long long *hist = malloc(nbins * sizeof(long long));

    if (hist == NULL) {
        fprintf(stderr, "Error: could not allocate histogram.\n");
        free(data);
        free(limits);
        return 1;
    }

    if (parallel_histogram(data, nelements, limits, nbins, hist, nthreads) != 0) {
        fprintf(stderr, "Error: parallel_histogram failed.\n");
        free(data);
        free(limits);
        free(hist);
        return 1;
    }

    printf("First bins:\n");
    for (int i = 0; i < nbins && i < 8; i++)
    {
        if (i == nbins - 1)
            printf("bin %d: [%lld, %lld] -> %lld\n", i, limits[i], limits[i + 1], hist[i]);
        else
            printf("bin %d: [%lld, %lld) -> %lld\n", i, limits[i], limits[i + 1], hist[i]);
    }

    free(data);
    free(limits);
    free(hist);

    return 0;
}