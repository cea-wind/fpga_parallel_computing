
#ifndef __VECTOR_ADD_H__
#define __VECTOR_ADD_H__

#include "ap_int.h"

void vector_accum_seq(
    float *a, int num,
    float *b
);


void vector_accum_pipe_bad(
    float *a, int num,
    float *b
);

void vector_accum_pipe_good(
    float *a, int num,
    float *b
);

void vector_accum_parallel(
    ap_uint<512> *a, int num,
	float *b
);


#endif
