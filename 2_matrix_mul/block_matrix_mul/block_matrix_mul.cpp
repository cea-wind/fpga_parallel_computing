#include "block_matrix_mul.h"

void LoadACore(ap_uint<32 * PARAL_K> *a, int col, int row_idx, int col_idx,
               int local_a[BLOCK_A_HEIGHT][BLOCK_A_WIDTH], bool enable) {
#pragma HLS INLINE off
  if (!enable) return;
  for (int i = 0; i < BLOCK_A_HEIGHT; i++) {
    for (int j = 0; j < BLOCK_A_WIDTH / PARAL_K; j++) {
#pragma HLS PIPELINE II = 1
      ap_uint<32 *PARAL_K> tmp = a[(row_idx + i) * col + col_idx + j];
      for (int k = 0; k < PARAL_K; k++) {
#pragma HLS UNROLL
        local_a[i][j * PARAL_K + k] = tmp.range(k * 32 + 31, k * 32);
      }
    }
  }
}

void ProvideACore(int local_a[BLOCK_A_HEIGHT][BLOCK_A_WIDTH],
                  hls::stream<PARAL_A_DT> &matrix_a_strm, bool enable) {
#pragma HLS INLINE off
  if (!enable) return;
  for (int i = 0; i < BLOCK_A_HEIGHT; i = i + PARAL_M) {  // A heigth
    // B width i.e A repeat times
    for (int j = 0; j < BLOCK_B_WIDTH; j = j + PARAL_N) {
      for (int k = 0; k < BLOCK_A_WIDTH; k = k + PARAL_K) {  // A width
#pragma HLS PIPELINE II = 1
        PARAL_A_DT a_sub_block;
        for (int ii = 0; ii < PARAL_M; ii++) {
          for (int kk = 0; kk < PARAL_K; kk++) {
            a_sub_block.v[ii * PARAL_K + kk] = local_a[i + ii][k + kk];
          }
        }
        matrix_a_strm.write(a_sub_block);
      }
    }
  }
}

void LoadMatrixA(ap_uint<32 * PARAL_K> *a, int a_row, int a_col,
                 hls::stream<PARAL_A_DT> &matrix_a_strm) {
  int local_a[2][BLOCK_A_HEIGHT][BLOCK_A_WIDTH];
#pragma HLS ARRAY_PARTITION variable = local_a complete dim = 1
#pragma HLS ARRAY_PARTITION variable = local_a cyclic factor = \
    local_a_dim2_factor dim = 2
#pragma HLS ARRAY_PARTITION variable = local_a cyclic factor = \
    local_a_dim3_factor dim = 3
  bool pingpang = true;
  for (int i = 0; i < a_row; i = i + BLOCK_A_HEIGHT) {
    for (int j = 0; j < a_col; j = j + BLOCK_A_WIDTH) {
      if (pingpang) {
        LoadACore(a, a_col, i, j, local_a[0], true);
        ProvideACore(local_a[1], matrix_a_strm, !(i == 0 && j == 0));
      } else {
        LoadACore(a, a_col, i, j, local_a[1], true);
        ProvideACore(local_a[0], matrix_a_strm, !(i == 0 && j == 0));
      }
      pingpang = !pingpang;
    }
  }
  if (pingpang)
    ProvideACore(local_a[1], matrix_a_strm, true);
  else
    ProvideACore(local_a[0], matrix_a_strm, true);
}

void LoadBCore(ap_uint<32 * PARAL_N> *b, int col, int row_idx, int col_idx,
               int local_b[BLOCK_B_HEIGHT][BLOCK_B_WIDTH], bool enable) {
#pragma HLS INLINE off
  if (!enable) return;
  for (int i = 0; i < BLOCK_B_HEIGHT; i++) {
    for (int j = 0; j < BLOCK_B_WIDTH / PARAL_N; j++) {
#pragma HLS PIPELINE II = 1
      ap_uint<32 *PARAL_N> tmp = b[(row_idx + i) * col + col_idx + j];
      for (int k = 0; k < PARAL_N; k++) {
#pragma HLS UNROLL
        local_b[i][j * PARAL_N + k] = tmp.range(k * 32 + 31, k * 32);
      }
    }
  }
}

void ProvideBCore(int local_b[BLOCK_B_HEIGHT][BLOCK_B_WIDTH],
                  hls::stream<PARAL_B_DT> &matrix_b_strm, bool enable) {
#pragma HLS INLINE off
  if (!enable) return;
  for (int j = 0; j < BLOCK_A_HEIGHT; j = j + PARAL_M) {
    for (int i = 0; i < BLOCK_B_HEIGHT; i = i + PARAL_K) {   // A heigth
      for (int k = 0; k < BLOCK_B_WIDTH; k = k + PARAL_N) {  // A width
#pragma HLS PIPELINE II = 1
        PARAL_B_DT b_sub_block;
        for (int ii = 0; ii < PARAL_M; ii++) {
          for (int kk = 0; kk < PARAL_K; kk++) {
            b_sub_block.v[ii * PARAL_K + kk] = local_b[i + kk][k + ii];
          }
        }
        matrix_b_strm.write(b_sub_block);
      }
    }
  }
}

void LoadMatrixB(ap_uint<32 * PARAL_N> *b, int b_row, int b_col,
                 hls::stream<PARAL_B_DT> &matrix_b_strm) {
  int local_b[2][BLOCK_B_HEIGHT][BLOCK_B_WIDTH];
#pragma HLS ARRAY_PARTITION variable = local_b complete dim = 1
#pragma HLS ARRAY_PARTITION variable = local_b cyclic factor = \
    local_b_dim2_factor dim = 2
#pragma HLS ARRAY_PARTITION variable = local_b cyclic factor = \
    local_b_dim3_factor dim = 3
  bool pingpang = true;
  for (int i = 0; i < b_row; i = i + BLOCK_B_HEIGHT) {
    for (int j = 0; j < b_col; j = j + BLOCK_B_WIDTH) {
      if (pingpang) {
        LoadBCore(b, b_col, i, j, local_b[0], true);
        ProvideBCore(local_b[1], matrix_b_strm, !(i == 0 && j == 0));
      } else {
        LoadBCore(b, b_col, i, j, local_b[1], true);
        ProvideBCore(local_b[0], matrix_b_strm, !(i == 0 && j == 0));
      }
      pingpang = !pingpang;
    }
  }
  if (pingpang)
    ProvideBCore(local_b[1], matrix_b_strm, true);
  else
    ProvideBCore(local_b[0], matrix_b_strm, true);
}

PARAL_C_DT MicroMulCore(PARAL_A_DT a_sub_block, PARAL_B_DT b_sub_block) {
  PARAL_C_DT c_sub_block;
  for (int ii = 0; ii < PARAL_M; ii++) {
#pragma HLS UNROLL
    for (int jj = 0; jj < PARAL_N; jj++) {
#pragma HLS UNROLL
      int tmp = 0;
      for (int kk = 0; kk < PARAL_K; kk++) {
#pragma HLS UNROLL
        tmp +=
            a_sub_block.v[ii * PARAL_K + kk] * b_sub_block.v[kk * PARAL_N + jj];
      }
      c_sub_block.v[ii * PARAL_N + jj] = tmp;
    }
  }
  return c_sub_block;
}

void BlockMulCore(hls::stream<PARAL_A_DT> &matrix_a_strm,
                  hls::stream<PARAL_B_DT> &matrix_b_strm,
                  hls::stream<PARAL_C_DT> &matrix_c_strm) {
#pragma HLS INLINE
  for (int i = 0; i < BLOCK_C_HEIGHT / PARAL_M; i++) {
    for (int j = 0; j < BLOCK_C_WIDTH / PARAL_N; j++) {
      int c[PARAL_M * PARAL_N];
      for (int k = 0; k < BLOCK_A_WIDTH / PARAL_K; k++) {
#pragma HLS PIPELINE II = 1
        PARAL_A_DT a_sub_block = matrix_a_strm.read();
        PARAL_B_DT b_sub_block = matrix_b_strm.read();
        PARAL_C_DT axb_sub_block = MicroMulCore(a_sub_block, b_sub_block);
        for (int t = 0; t < PARAL_M * PARAL_K; t++) {
          if (k == 0) {
            c[t] = axb_sub_block.v[t];
          } else {
            c[t] += axb_sub_block.v[t];
          }
        }
      }
      PARAL_C_DT c_sub_block;
      for (int t = 0; t < PARAL_M * PARAL_N; t++) {
#pragma HLS UNROLL
        c_sub_block.v[t] = c[t];
      }
      matrix_c_strm.write(c_sub_block);
    }
  }
}

void MatrixMulCore(hls::stream<PARAL_A_DT> &matrix_a_strm,
                   hls::stream<PARAL_B_DT> &matrix_b_strm, int c_row, int c_col,
                   hls::stream<PARAL_C_DT> &matrix_c_strm) {
  for (int m = 0; m < c_row / BLOCK_C_HEIGHT; m++) {
    for (int n = 0; n < c_col / BLOCK_C_WIDTH; n++) {
      BlockMulCore(matrix_a_strm, matrix_b_strm, matrix_c_strm);
    }
  }
}

void ReshapeMatrixC(hls::stream<PARAL_C_DT> &matrix_c_strm,
                    int local_c[BLOCK_C_HEIGHT][BLOCK_C_WIDTH], bool enable) {
#pragma HLS INLINE off
  if (!enable) return;
  for (int i = 0; i < BLOCK_C_HEIGHT; i = i + PARAL_M) {
    for (int j = 0; j < BLOCK_C_WIDTH; j = j + PARAL_N) {
#pragma HLS PIPELINE II = 1
      PARAL_C_DT tmp = matrix_c_strm.read();
      for (int ii = 0; ii < PARAL_M; ii++) {
        for (int jj = 0; jj < PARAL_N; jj++) {
          local_c[i + ii][j + jj] = tmp.v[ii * PARAL_N + jj];
        }
      }
    }
  }
}

void StoreCCore(int local_c[BLOCK_C_HEIGHT][BLOCK_C_WIDTH], int c_col,
                int row_idx, int col_idx, ap_uint<PARAL_N * 32> *c,
                bool enable) {
#pragma HLS INLINE off
  if (!enable) return;
  for (int i = 0; i < BLOCK_C_HEIGHT; i++) {
    for (int j = 0; j < BLOCK_C_WIDTH / PARAL_N; j++) {
#pragma HLS PIPELINE II = 1
      ap_uint<PARAL_N * 32> c_sub_tile;
      for (int k = 0; k < PARAL_N; k++) {
        c_sub_tile.range(k * 32 + 31, k + 32) = local_c[i][j * PARAL_N + k];
      }
      c[(row_idx + i) * c_col + j] = c_sub_tile;
    }
  }
}

void StoreMatrixC(hls::stream<PARAL_C_DT> &matrix_c_strm, int c_row, int c_col,
                  ap_uint<PARAL_N * 32> *c) {
  int local_c[2][BLOCK_A_HEIGHT][BLOCK_B_WIDTH];
#pragma HLS ARRAY_PARTITION variable = local_c complete dim = 1
#pragma HLS ARRAY_PARTITION variable = local_c cyclic factor = \
    local_a_dim2_factor dim = 2
#pragma HLS ARRAY_PARTITION variable = local_c cyclic factor = \
    local_b_dim3_factor dim = 3
  bool pingpang = false;
  ReshapeMatrixC(matrix_c_strm, local_c[0], true);
  for (int i = 0; i < c_row; i = i + BLOCK_C_HEIGHT) {
    for (int j = 0; j < c_col; j = j + BLOCK_C_WIDTH) {
      bool almost_finish =
          (i + BLOCK_C_HEIGHT >= c_row) && (j + BLOCK_C_WIDTH >= c_col);
      if (pingpang) {
        ReshapeMatrixC(matrix_c_strm, local_c[0], !almost_finish);
        StoreCCore(local_c[1], c_col, i, j, c, true);
      } else {
        ReshapeMatrixC(matrix_c_strm, local_c[1], !almost_finish);
        StoreCCore(local_c[0], c_col, i, j, c, true);
      }
      pingpang = !pingpang;
    }
  }
}

void MatrixMul(ap_uint<32 * PARAL_K> *a, int a_row, int a_col,
               ap_uint<32 * PARAL_N> *b, int b_row, int b_col,
               ap_uint<32 * PARAL_N> *c) {
#pragma HLS INTERFACE m_axi depth = 65536 port = a
#pragma HLS INTERFACE m_axi depth = 65536 port = b
#pragma HLS INTERFACE m_axi depth = 65536 port = c
#pragma HLS DATAFLOW
  static hls::stream<PARAL_A_DT> matrix_a_strm;
  static hls::stream<PARAL_B_DT> matrix_b_strm;
  static hls::stream<PARAL_C_DT> matrix_c_strm;
#pragma HLS STREAM variable = matrix_a_strm depth = 16 dim = 1
#pragma HLS STREAM variable = matrix_b_strm depth = 16 dim = 1
#pragma HLS STREAM variable = matrix_c_strm depth = 16 dim = 1
  LoadMatrixA(a, a_row, a_col, matrix_a_strm);
  LoadMatrixB(b, b_row, b_col, matrix_b_strm);
  MatrixMulCore(matrix_a_strm, matrix_b_strm, a_row, b_col, matrix_c_strm);
  StoreMatrixC(matrix_c_strm, a_row, b_col, c);
}
