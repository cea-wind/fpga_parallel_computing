#include "vector_accum.h"

#include "stdio.h"
#include "stdlib.h"
int main() {
  int err = 0;
  int num = 204800;
  float *a, *b_seq, *b_pipe_bad, *b_pipe_good, *b_parallel;
  a = (float *)malloc(sizeof(float) * num);
  b_seq = (float *)malloc(sizeof(float) * 1);
  b_pipe_bad = (float *)malloc(sizeof(float) * 1);
  b_pipe_good = (float *)malloc(sizeof(float) * 1);
  b_parallel = (float *)malloc(sizeof(float) * 1);
  for (int i = 0; i < num; i++) {
    a[i] = rand() / 10000.f;
  }
  vector_accum_seq(a, num, b_seq);
  vector_accum_pipe_bad(a, num, b_pipe_bad);
  vector_accum_pipe_good(a, num, b_pipe_good);
  ap_uint<512> *a16;
  a16 = (ap_uint<512> *)a;
  vector_accum_parallel(a16, num, b_parallel);
  printf("seq       result = %f\n", b_seq[0]);
  printf("pipe_bad  result = %f\n", b_pipe_bad[0]);
  printf("pipe_good result = %f\n", b_pipe_good[0]);
  printf("parallel  result = %f\n", b_parallel[0]);
  return err;
}
