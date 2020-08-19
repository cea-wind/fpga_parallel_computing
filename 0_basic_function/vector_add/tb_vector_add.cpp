#include "vector_add.h"

#include "stdio.h"
#include "stdlib.h"
int main() {
  int err = 0;
  int num = 2048;
  int *a, *b, *c_seq, *c_pipe, *c_parallel;
  a = (int *)malloc(sizeof(int) * num);
  b = (int *)malloc(sizeof(int) * num);
  c_seq = (int *)malloc(sizeof(int) * num);
  c_pipe = (int *)malloc(sizeof(int) * num);
  c_parallel = (int *)malloc(sizeof(int) * num);
  for (int i = 0; i < num; i++) {
    a[i] = rand();
    b[i] = rand();
  }
  vector_add_seq(a, b, num, c_seq);
  vector_add_pipe(a, b, num, c_pipe);
  ap_uint<512> *a16, *b16, *c16;
  a16 = (ap_uint<512> *)a;
  b16 = (ap_uint<512> *)b;
  c16 = (ap_uint<512> *)c_parallel;
  vector_add_parallel(a16, b16, num, c16);

  for (int i = 0; i < num; i++) {
    if (c_seq[i] != c_pipe[i] || c_pipe[i] != c_parallel[i]) {
      err++;
      printf("Error: idx = %d, c_seq = %d, c_pipe = %d, c_parallel = %d \n", i,
             c_seq[i], c_pipe[i], c_parallel[i]);
      return err;
    }
  }
  printf("Simulation PASSED\n");
  return err;
}
