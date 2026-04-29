#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

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

    printf("nelements = %lld\n", nelements);
    printf("npivots   = %d\n", npivots);
    printf("nbins     = %d\n", nbins);
    printf("nthreads  = %d\n", nthreads);
    printf("nr        = %d\n", nr);

    return 0;
}