#ifndef DATA_GENERATION_H
#define DATA_GENERATION_H

unsigned long long random_63(void);

int generate_data(long long *data, long long nelements, int nbins, int balanced);

#endif