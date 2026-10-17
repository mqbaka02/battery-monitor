#include <stdio.h>
#include <stdlib.h>

typedef struct {
    float *x;
    float *y;
    int count;
} PairData;

PairData read_pairs(const char *filename);
