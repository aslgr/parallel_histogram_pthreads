#include <stdlib.h>
#include "data_generation.h"

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