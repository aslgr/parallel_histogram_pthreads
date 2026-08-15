#ifndef LIMITS_H
#define LIMITS_H

int build_limits(const long long *data, long long nelements, int npivots,
                 int nbins, long long *pivots, long long *limits);

#endif