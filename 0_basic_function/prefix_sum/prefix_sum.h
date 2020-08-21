#ifndef __PREFIX_SUM_H__
#define __PREFIX_SUM_H__

#include "ap_int.h"

void prefix_sum_seq(int *a, int num, int *b);
void prefix_sum_pipe(int *a, int num, int *b);
void prefix_sum_parallel(ap_uint<512> *a, int num, ap_uint<512> *b);

#endif