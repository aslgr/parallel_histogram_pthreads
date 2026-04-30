#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include "data_generation.h"
#include "histogram.h"

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

    srand(time(NULL));

    long long *data = generate_data(nelements);

    if (data == NULL) {
        fprintf(stderr, "Error: could not allocate data array.\n");
        return 1;
    }

    printf("First values:\n");
    for (int i = 0; i < 5 && i < nelements; i++)
        printf("data[%d] = %lld\n", i, data[i]);

    long long limits[] = {LLONG_MIN, 0, LLONG_MAX};
    nbins = 2;
    long long *hist = malloc(nbins * sizeof(long long));

    serial_histogram(data, nelements, limits, nbins, hist);

    for (int i = 0; i < nbins; i++)
        printf("hist[%d] = %lld\n", i, hist[i]);

    
    free(data);
    free(hist);

    return 0;
}