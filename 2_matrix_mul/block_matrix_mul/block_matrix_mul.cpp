#include "block_matrix_mul.h"
//#define DEBUG_LOAD_A
//#define DEBUG_PROVIDE_A
//#define DEBUG_LOAD_B
//#define DEBUG_PROVIDE_B
//#define DEBUG_ACCUM_C
void LoadACore(ap_uint<32 * PARAL_K> *a, int col, int row_idx, int col_idx,
               int local_a[BLOCK_A_HEIGHT][BLOCK_A_WIDTH], bool enable) {
#pragma HLS INLINE off
  if (!enable) return;
  for (int i = 0; i < BLOCK_A_HEIGHT; i++) {
    for (int j = 0; j < BLOCK_A_WIDTH / PARAL_K; j++) {
#pragma HLS PIPELINE II = 1
      ap_uint<32 *PARAL_K> tmp =
          a[(row_idx + i) * col / PARAL_K + col_idx / PARAL_K + j];
      for (int k = 0; k < PARAL_K; k++) {
#pragma HLS UNROLL
        local_a[i][j * PARAL_K + k] = tmp.range(k * 32 + 31, k * 32);
      }
    }
  }
#ifdef DEBUG_LOAD_A
  for (int i = 0; i < BLOCK_A_HEIGHT; i++) {
    for (int j = 0; j < BLOCK_A_WIDTH; j++) {
      printf("%d ", local_a[i][j]);
    }
    printf("\n");
  }
#endif
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
#ifdef DEBUG_PROVIDE_A
            printf("%d ", local_a[i + ii][k + kk]);
#endif
          }
#ifdef DEBUG_PROVIDE_A
          printf("\n");
#endif
        }
        matrix_a_strm.write(a_sub_block);
      }
    }
  }
}

void LoadMatrixA(ap_uint<32 * PARAL_K> *a, int a_row, int a_col, int b_col,
                 hls::stream<PARAL_A_DT> &matrix_a_strm) {
  int local_a[2][BLOCK_A_HEIGHT][BLOCK_A_WIDTH];
#pragma HLS ARRAY_PARTITION variable = local_a complete dim = 1
#pragma HLS ARRAY_PARTITION variable = local_a cyclic factor = \
    local_a_dim2_factor dim = 2
#pragma HLS ARRAY_PARTITION variable = local_a cyclic factor = \
    local_a_dim3_factor dim = 3
  bool pingpang = true;
  for (int i = 0; i < a_row; i = i + BLOCK_A_HEIGHT) {
    for (int r = 0; r < b_col; r = r + BLOCK_B_WIDTH) {
      for (int j = 0; j < a_col; j = j + BLOCK_A_WIDTH) {
        if (pingpang) {
          LoadACore(a, a_col, i, j, local_a[0], true);
          ProvideACore(local_a[1], matrix_a_strm,
                       !(i == 0 && j == 0 && r == 0));
        } else {
          LoadACore(a, a_col, i, j, local_a[1], true);
          ProvideACore(local_a[0], matrix_a_strm,
                       !(i == 0 && j == 0 && r == 0));
        }
        pingpang = !pingpang;
      }
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
      ap_uint<32 *PARAL_N> tmp =
          b[(row_idx + i) * col / PARAL_N + col_idx / PARAL_N + j];
      for (int k = 0; k < PARAL_N; k++) {
#pragma HLS UNROLL
        local_b[i][j * PARAL_N + k] = tmp.range(k * 32 + 31, k * 32);
      }
    }
  }
#ifdef DEBUG_LOAD_B
  for (int i = 0; i < BLOCK_B_HEIGHT; i++) {
    for (int j = 0; j < BLOCK_B_WIDTH; j++) {
      printf("%d ", local_b[i][j]);
    }
    printf("\n");
  }
#endif
}

void ProvideBCore(int local_b[BLOCK_B_HEIGHT][BLOCK_B_WIDTH],
                  hls::stream<PARAL_B_DT> &matrix_b_strm, bool enable) {
#pragma HLS INLINE off
  if (!enable) return;
  for (int j = 0; j < BLOCK_A_HEIGHT; j = j + PARAL_M) {
    for (int k = 0; k < BLOCK_B_WIDTH; k = k + PARAL_N) {     // A width
      for (int i = 0; i < BLOCK_B_HEIGHT; i = i + PARAL_K) {  // A heigth
#pragma HLS PIPELINE II = 1
        PARAL_B_DT b_sub_block;
        for (int ii = 0; ii < PARAL_N; ii++) {
          for (int kk = 0; kk < PARAL_K; kk++) {
            b_sub_block.v[kk * PARAL_N + ii] = local_b[i + kk][k + ii];
#ifdef DEBUG_PROVIDE_B
            printf("%d ", local_b[i + kk][k + ii]);
#endif
          }
#ifdef DEBUG_PROVIDE_B
          printf("\n");
#endif
        }
        matrix_b_strm.write(b_sub_block);
      }
    }
  }
}

void LoadMatrixB(ap_uint<32 * PARAL_N> *b, int b_row, int b_col, int a_row,
                 hls::stream<PARAL_B_DT> &matrix_b_strm) {
  int local_b[2][BLOCK_B_HEIGHT][BLOCK_B_WIDTH];
#pragma HLS ARRAY_PARTITION variable = local_b complete dim = 1
#pragma HLS ARRAY_PARTITION variable = local_b cyclic factor = \
    local_b_dim2_factor dim = 2
#pragma HLS ARRAY_PARTITION variable = local_b cyclic factor = \
    local_b_dim3_factor dim = 3
  bool pingpang = true;
  for (int r = 0; r < a_row; r = r + BLOCK_A_HEIGHT) {
    for (int j = 0; j < b_col; j = j + BLOCK_B_WIDTH) {
      for (int i = 0; i < b_row; i = i + BLOCK_B_HEIGHT) {
        if (pingpang) {
          LoadBCore(b, b_col, i, j, local_b[0], true);
          ProvideBCore(local_b[1], matrix_b_strm,
                       !(i == 0 && j == 0 && r == 0));
        } else {
          LoadBCore(b, b_col, i, j, local_b[1], true);
          ProvideBCore(local_b[0], matrix_b_strm,
                       !(i == 0 && j == 0 && r == 0));
        }
        pingpang = !pingpang;
      }
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

void InitCache(PARAL_C_DT cache_reg[4], int cache_idx_i[4],
               int cache_idx_j[4]) {
#pragma HLS INLINE
#pragma HLS DATA_PACK variable = cache_reg
#pragma HLS ARRAY_PARTITION variable = cache_reg complete dim = 1
#pragma HLS ARRAY_PARTITION variable = cache_idx_i complete dim = 1
#pragma HLS ARRAY_PARTITION variable = cache_idx_j complete dim = 1
  for (int i = 0; i < 4; i++) {
#pragma HLS UNROLL
    cache_idx_i[i] = 0;
    cache_idx_j[i] = 0;
  }
}
PARAL_C_DT ReadCachedRam(int accum_c[BLOCK_C_HEIGHT][BLOCK_C_WIDTH],
                         PARAL_C_DT cache_reg[4], int cache_idx_i[4],
                         int cache_idx_j[4], int i, int j) {
#pragma HLS INLINE
  PARAL_C_DT rdata;
  for (int t = 0; t < 4; t++) {
#pragma HLS UNROLL
    if (cache_idx_i[t] == i && cache_idx_j[t] == j) {
      rdata = cache_reg[t];
      return rdata;
    }
  }
  for (int r = 0; r < PARAL_M * PARAL_N; r++) {
#pragma HLS UNROLL
    rdata.v[r] = accum_c[i*PARAL_M + r / PARAL_N][j*PARAL_N + r % PARAL_N];
  }
  return rdata;
}

void WriteCachedRam(int accum_c[BLOCK_C_HEIGHT][BLOCK_C_WIDTH],
                    PARAL_C_DT cache_reg[4], int cache_idx_i[4],
                    int cache_idx_j[4], int i, int j, PARAL_C_DT wdata) {
#pragma HLS INLINE
  for (int r = 3; r > 0; r--) {
#pragma HLS UNROLL
    cache_reg[r] = cache_reg[r - 1];
    cache_idx_i[r] = cache_idx_i[r - 1];
    cache_idx_j[r] = cache_idx_j[r - 1];
  }
  cache_reg[0] = wdata;
  cache_idx_i[0] = i;
  cache_idx_j[0] = j;
  for (int r = 0; r < PARAL_M * PARAL_N; r++) {
#pragma HLS UNROLL
    accum_c[i*PARAL_M + r / PARAL_N][j*PARAL_N + r % PARAL_N] = wdata.v[r];
  }
}
void BlockMulCore(hls::stream<PARAL_A_DT> &matrix_a_strm,
                  hls::stream<PARAL_B_DT> &matrix_b_strm, int a_col,
                  int accum_c[BLOCK_C_HEIGHT][BLOCK_C_WIDTH], bool enable) {
  if (!enable) return;
  PARAL_C_DT cache_reg[4];
  int cache_idx_i[4], cache_idx_j[4];
  InitCache(cache_reg, cache_idx_i, cache_idx_j);
  int t = 0, i = 0, j = 0, k = 0;
  while (t < a_col) {
// for (int t = 0; t < a_col; t = t + BLOCK_A_WIDTH) {
//   for (int i = 0; i < BLOCK_C_HEIGHT; i = i + PARAL_M) {
//     for (int j = 0; j < BLOCK_C_WIDTH; j = j + PARAL_N) {
//       for (int k = 0; k < BLOCK_A_WIDTH / PARAL_K; k++) {
#pragma HLS DEPENDENCE variable = accum_c inter false
#pragma HLS PIPELINE II = 1
    // read
    PARAL_A_DT a_sub_block = matrix_a_strm.read();
    PARAL_B_DT b_sub_block = matrix_b_strm.read();
    PARAL_C_DT update_data =
        ReadCachedRam(accum_c, cache_reg, cache_idx_i, cache_idx_j, i, j);
    PARAL_C_DT axb_sub_block = MicroMulCore(a_sub_block, b_sub_block);
#pragma HLS DATA_PACK variable = update_data
#pragma HLS DATA_PACK variable = axb_sub_block
    // accumlate
    for (int r = 0; r < PARAL_M * PARAL_N; r++) {
#pragma HLS UNROLL
      if (t == 0 && k == 0) {
        update_data.v[r] = axb_sub_block.v[r];
      } else {
        update_data.v[r] = update_data.v[r] + axb_sub_block.v[r];
      }
    }
    // write
    WriteCachedRam(accum_c, cache_reg, cache_idx_i, cache_idx_j, i, j,
                   update_data);

    if (k + 1 == BLOCK_A_WIDTH / PARAL_K) {
      k = 0;
      if (j + 1 >= BLOCK_C_WIDTH/PARAL_N) {
        j = 0;
        if (i + 1 >= BLOCK_C_HEIGHT/PARAL_M) {
          i = 0;
          t = t + BLOCK_A_WIDTH;
        } else {
          i = i + 1;
        }
      } else {
        j = j + 1;
      }
    } else {
      k++;
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
        c_sub_tile.range(k * 32 + 31, k * 32) = local_c[i][j * PARAL_N + k];
        // printf("%d ",local_c[i][j * PARAL_N + k]);
      }
      // printf("\n");
      c[(row_idx + i) * c_col / PARAL_N + col_idx / PARAL_N + j] = c_sub_tile;
    }
  }
}

void MatrixMulCore(hls::stream<PARAL_A_DT> &matrix_a_strm,
                   hls::stream<PARAL_B_DT> &matrix_b_strm, int c_row, int a_col,
                   int c_col, ap_uint<32 * PARAL_N> *c) {
  int accum_c[2][BLOCK_C_HEIGHT][BLOCK_C_WIDTH];
#pragma HLS ARRAY_PARTITION variable = accum_c complete dim = 1
#pragma HLS ARRAY_PARTITION variable = accum_c cyclic factor = \
    local_a_dim2_factor dim = 2
#pragma HLS ARRAY_PARTITION variable = accum_c cyclic factor = \
    local_b_dim3_factor dim = 3

  bool pingpang;
  BlockMulCore(matrix_a_strm, matrix_b_strm, a_col, accum_c[0],
               !(c_row == 0 || a_col == 0));
  pingpang = false;
  for (int m = 0; m < c_row; m = m + BLOCK_C_HEIGHT) {
    for (int n = 0; n < c_col; n = n + BLOCK_C_WIDTH) {
      bool almost_finish =
          (m + BLOCK_C_HEIGHT >= c_row) && (n + BLOCK_C_WIDTH >= c_col);
      if (pingpang) {
        BlockMulCore(matrix_a_strm, matrix_b_strm, a_col, accum_c[0],
                     !almost_finish);
        StoreCCore(accum_c[1], c_col, m, n, c, true);
      } else {
        BlockMulCore(matrix_a_strm, matrix_b_strm, a_col, accum_c[1],
                     !almost_finish);
        StoreCCore(accum_c[0], c_col, m, n, c, true);
      }
      pingpang = !pingpang;
    }
  }
}

void MatrixMul(ap_uint<32 * PARAL_K> *a, int a_row, int a_col,
               ap_uint<32 * PARAL_N> *b, int b_row, int b_col,
               ap_uint<32 * PARAL_N> *c) {
#pragma HLS INTERFACE m_axi depth=8192 port=a num_read_outstanding=8 num_write_outstanding=8
#pragma HLS INTERFACE m_axi depth=8192 port=b num_read_outstanding=8 num_write_outstanding=8
#pragma HLS INTERFACE m_axi depth=8192 port=c num_read_outstanding=8 num_write_outstanding=8
#pragma HLS DATAFLOW
  static hls::stream<PARAL_A_DT> matrix_a_strm;
  static hls::stream<PARAL_B_DT> matrix_b_strm;
#pragma HLS STREAM variable = matrix_a_strm depth = 16 dim = 1
#pragma HLS STREAM variable = matrix_b_strm depth = 16 dim = 1
  LoadMatrixA(a, a_row, a_col, b_col, matrix_a_strm);
  LoadMatrixB(b, b_row, b_col, a_row, matrix_b_strm);
  MatrixMulCore(matrix_a_strm, matrix_b_strm, a_row, a_col, b_col, c);
}
