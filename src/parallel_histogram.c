#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

long long random_long_long(void) {

    unsigned long long value = 0;

    for (int i = 0; i < 5; i++)
        value = (value << 15) ^ (unsigned long long)(rand() & 0x7FFF);

    return (long long)value;
}

long long *generate_data(long long nelements) {

    long long *data = malloc(nelements * sizeof(long long));

    if (data == NULL)
        return NULL;

    for (long long i = 0; i < nelements; i++)
        data[i] = random_long_long();

    return data;
}

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

    srand(time(NULL));

    long long *data = generate_data(nelements);

    if (data == NULL) {
        fprintf(stderr, "Error: could not allocate data array.\n");
        return 1;
    }

    printf("First values:\n");
    for (int i = 0; i < 5 && i < nelements; i++) {
        printf("data[%d] = %lld\n", i, data[i]);
    }

    free(data);

    return 0;
}