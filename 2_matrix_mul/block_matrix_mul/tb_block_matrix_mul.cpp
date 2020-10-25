#include "block_matrix_mul.h"

void MatrixMulRef(int *a, int a_row, int a_col, int *b, int b_row, int b_col,
                  int *c) {
  assert(a_col == b_row);
  for (int i = 0; i < a_row; i++) {
    for (int j = 0; j < b_col; j++) {
      int tmp = 0;
      for (int k = 0; k < a_col; k++) {
        tmp += a[i * a_col + k] * b[k * b_col + j];
      }
      c[i * b_col + j] = tmp;
    }
  }
}

int main() {
  int *a, *b, *c_ref, *c;
  int a_row = 128;
  int a_col = 128;
  int b_row = a_col;
  int b_col = 128;
  a = (int *)malloc(32768 * sizeof(int));
  b = (int *)malloc(32768 * sizeof(int));
  c = (int *)malloc(32768 * sizeof(int));
  c_ref = (int *)malloc(a_row * b_col * sizeof(int));
  // init a & b
  for (int i = 0; i < a_row; i++) {
    for (int j = 0; j < a_col; j++) {
      a[i * a_col + j] = rand()%256;
    }
  }
  for (int i = 0; i < b_row; i++) {
    for (int j = 0; j < b_col; j++) {
      b[i * b_col + j] = rand()%256;//i * b_col + j;
    }
  }
  MatrixMulRef(a, a_row, a_col, b, b_row, b_col, c_ref);
  MatrixMul((ap_uint<32 * PARAL_K> *)a, a_row, a_col,
            (ap_uint<32 * PARAL_N> *)b, b_row, b_col,
            (ap_uint<32 * PARAL_N> *)c);

  int err = 0;
  for(int i=0;i<a_row*b_col;i++){
    if(c[i]!=c_ref[i]){
        err++;
        printf("row %d, col %d error, result is %d, golden is %d\n",i/b_col,i%b_col,c[i],c_ref[i]);
    }
  }
  return err;
}
