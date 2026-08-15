#include <stdlib.h>
#include "data_generation.h"

unsigned long long random_63(void) {

    return ((unsigned long long)(unsigned)rand())
         | ((unsigned long long)(unsigned)rand() << 21)
         | ((unsigned long long)(unsigned)rand() << 42);
}

static long long random_long_long(void) {

    union {
        unsigned long long u;
        long long s;
    } x;

    x.u = ((unsigned long long)(unsigned)rand() << 49)
        | ((unsigned long long)(unsigned)rand() << 34)
        | ((unsigned long long)(unsigned)rand() << 19)
        | ((unsigned long long)(unsigned)rand() << 4)
        | ((unsigned long long)(unsigned)rand() & 0xF);

    return x.s;
}

static void swap_long_long(long long *a, long long *b) {

    long long tmp = *a;
    *a = *b;
    *b = tmp;
}

int generate_data(long long *data, long long nelements, int nbins, int balanced)
{
    if (data == NULL || nelements <= 0)
        return -1;

    if (!balanced) {
        for (long long i = 0; i < nelements; i++)
            data[i] = random_long_long();

        return 0;
    }

    if (nbins <= 0)
        return -1;

    for (long long i = 0; i < nelements; i++)
        data[i] = i;

    for (long long i = nelements - 1; i > 0; i--)
    {
        long long j = (long long)(random_63() % (unsigned long long)(i + 1));

        swap_long_long(&data[i], &data[j]);
    }

    for (long long i = 0; i < nelements; i++)
        data[i] %= nbins;

    return 0;
}