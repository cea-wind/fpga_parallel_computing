
#ifndef __BLOCK_MATRIX_MUL_H__
#define __BLOCK_MATRIX_MUL_H__

#include "ap_int.h"
#include "hls_stream.h"
#include "assert.h"

#define BLOCK_A_HEIGHT 128
#define BLOCK_A_WIDTH 128
#define BLOCK_B_HEIGHT BLOCK_A_WIDTH
#define BLOCK_B_WIDTH 128
#define BLOCK_C_HEIGHT BLOCK_A_HEIGHT
#define BLOCK_C_WIDTH BLOCK_B_WIDTH

#define PARAL_M 8
#define PARAL_K 8
#define PARAL_N 8

const int local_a_dim2_factor = PARAL_M;
const int local_a_dim3_factor = PARAL_K;
const int local_b_dim2_factor = PARAL_K;
const int local_b_dim3_factor = PARAL_N;

struct PARAL_A_DT {
  int v[PARAL_M * PARAL_K];
};
struct PARAL_B_DT {
  int v[PARAL_K * PARAL_N];
};
struct PARAL_C_DT {
  int v[PARAL_M * PARAL_N];
};

#endif
