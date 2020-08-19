#include "prefix_sum.h"

#include "stdio.h"
#include "stdlib.h"

int main() {
  int err = 0;
  int num = 2048;
  int *a, *b_seq, *b_pipe, *b_parallel;
  a = (int *)malloc(sizeof(int) * num);
  b_seq = (int *)malloc(sizeof(int) * num);
  b_pipe = (int *)malloc(sizeof(int) * num);
  b_parallel = (int *)malloc(sizeof(int) * num);
  for (int i = 0; i < num; i++) {
    a[i] = i + 1;
  }
  prefix_sum_seq(a, num, b_seq);
  prefix_sum_pipe(a, num, b_pipe);
  ap_uint<512> *a16, *b16;
  a16 = (ap_uint<512> *)a;
  b16 = (ap_uint<512> *)b_parallel;
  prefix_sum_parallel(a16, num, b16);

  for (int i = 0; i < num; i++) {
    if (b_seq[i] != b_pipe[i] || b_pipe[i] != b_parallel[i]) {
      err++;
      printf("Error: idx = %d, b_seq = %d, b_pipe = %d, b_parallel = %d \n", i,
             b_seq[i], b_pipe[i], b_parallel[i]);
      return err;
    }
  }
  printf("Simulation PASSED\n");
  return err;
}
