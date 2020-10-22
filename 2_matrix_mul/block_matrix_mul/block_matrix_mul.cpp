#include "block_matrix_mul.h"

#define BLOCK_A_HEIGHT 128
#define BLOCK_A_WIDTH 128
#define BLOCK_B_HEIGHT BLOCK_A_WIDTH
#define BLOCK_B_WIDTH 128
#define BLOCK_C_HEIGHT BLOCK_A_HEIGHT
#define BLOCK_C_WIDTH BLOCK_B_WIDTH

#define PARA_M 8
#define PARA_K 8
#define PARA_N 8

void LoadACore() {}

void ProvideACore(int local_a[BLOCK_A_HEIGHT][BLOCK_A_WIDTH],
                  hls::stream<ap_uint<PARA_M * PARA_K * 32> > &matrix_a_strm) {
  for (int i = 0; i < BLOCK_A_HEIGHT / PARA_M; i++) {  // A heigth
    // B width i.e A repeat times
    for (int j = 0; j < BLOCK_B_WIDTH / PARA_N; j++) {
      for (int k = 0; k < BLOCK_A_WIDTH / PARA_K; k++) {  // A width
#paragma HLS PIPELINE II = 1
        hls::stream<ap_uint<PARA_M * PARA_K * 32> > a_sub_block;
        for (int ii = 0; ii < PARA_M; ii++) {
          for (int kk = 0; kk < PARA_K; kk++) {
            a_sub_block.range(32 * (ii * PARA_K + kk) + 31,
                              32 * (ii * PARA_K + kk)) =
                local_a[i * PARA_M + ii][k * PARA_K + kk];
          }
        }
      }
    }
  }
}

void LoadMatrixA(int *a, int a_row, int a_col,
                 hls::stream<ap_uint<PARA_M * PARA_K * 32> > &matrix_a_strm) {
  int local_a[2][BLOCK_A_HEIGHT][BLOCK_A_WIDTH];
}

void LoadMatrixB() {}

ap_uint<PARA_M * PARA_N * 32> MicroMulCore(
    ap_uint<PARA_M * PARA_K * 32> a_sub_block,
    ap_uint<PARA_K * PARA_N * 32> b_sub_block) {
  ap_uint<PARA_M * PARA_K * 32> c_sub_block;
  for (int ii = 0; ii < PARA_M; ii++) {
#pragma HLS UNROLL
    for (int jj = 0; jj < PARA_N; jj++) {
#pragma HLS UNROLL
      int tmp = 0;
      for (int kk = 0; kk < PARA_K; kk++) {
#pragma HLS UNROLL
        tmp += int(a_sub_block.range((ii * PARA_K + kk) * 32 + 31,
                                     (ii * PARA_K + kk) * 32)) *
               int(b_sub_block.range((kk * PARA_N + jj) * 32 + 31,
                                     (kk * PARA_N + jj) * 32));
      }
      c_sub_block.range((ii * PARA_N + jj) * 32 + 31, (ii * PARA_N + jj) * 32) = tmp;
    }
  }
  return c_sub_block;
}

void BlockMulCore(hls::stream<ap_uint<PARA_M * PARA_K * 32> > &matrix_a_strm,
                  hls::stream<ap_uint<PARA_K * PARA_N * 32> > &matrix_b_strm,
                  hls::stream<ap_uint<PARA_M * PARA_N * 32> > &matrix_c_strm) {
#pragma HLS INLINE
  for (int i = 0; i < BLOCK_C_HEIGHT / PARA_M; i++) {
    for (int j = 0; j < BLOCK_C_WIDTH / PARA_N; j++) {
      int c[PARA_M * PARA_N];
      for (int k = 0; k < BLOCK_A_WIDTH / PARA_K; k++) {
#pragma HLS PIPELINE II = 1
        ap_uint<PARA_M *PARA_K * 32> a_sub_block = matrix_a_strm.read();
        ap_uint<PARA_K *PARA_N * 32> b_sub_block = matrix_b_strm.read();
        ap_uint<PARA_M *PARA_N * 32> axb_sub_block =
            MicroMulCore(a_sub_block, b_sub_block);
        for (int t = 0; t < PARA_M * PARA_K; t++) {
          if (k == 0) {
            c[t] = int(axb_sub_block.range(t * 32 + 31, t * 32));
          } else {
            c[t] = c[t] + int(axb_sub_block.range(t * 32 + 31, t * 32));
          }
        }
      }
      ap_uint<PARA_M * PARA_N * 32> c_sub_block;
      for (int t = 0; t < PARA_M * PARA_N; t++) {
#pragma HLS UNROLL
        c_sub_block.range(t * 32 + 31, t * 32) = c[t];
      }
      matrix_c_strm.write(c_sub_block);
    }
  }
}

void MatrixMulCore(hls::stream<ap_uint<PARA_M * PARA_K * 32> > &matrix_a_strm,
                   hls::stream<ap_uint<PARA_K * PARA_N * 32> > &matrix_b_strm,
                   int c_row, int c_col,
                   hls::stream<ap_uint<PARA_M * PARA_N * 32> > &matrix_c_strm) {
  for (int m = 0; m < c_row / BLOCK_C_HEIGHT; m++) {
    for (int n = 0; n < c_col / BLOCK_C_WIDTH; n++) {
      BlockMulCore(matrix_a_strm, matrix_b_strm, matrix_c_strm);
    }
  }
}

void StoreMatrixC(hls::stream<ap_uint<PARA_M * PARA_N * 32> > &matrix_c_strm,
                  int c_row, int c_col, ap_uint<PARA_N * 32> *c) {
  for (int m = 0; m < c_row / BLOCK_C_HEIGHT; m++) {
    for (int n = 0; n < c_col / BLOCK_C_WIDTH; n++) {
      BlockMulCore(matrix_a_strm，matrix_b_strm，matrix_c_strm);
    }
  }
}