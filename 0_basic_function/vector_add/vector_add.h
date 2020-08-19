
#ifndef __VECTOR_ADD_H__
#define __VECTOR_ADD_H__

#include "ap_int.h"

void vector_add_seq(int *a, int *b, int num, int *c);

void vector_add_pipe(int *a, int *b, int num, int *c);

void vector_add_parallel(ap_uint<512> *a, ap_uint<512> *b, int num,
                         ap_uint<512> *c);
#endif
