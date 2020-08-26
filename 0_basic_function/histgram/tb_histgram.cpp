#include "histgram.h"

#include "stdio.h"
#include "stdlib.h"
int main() {
  int err = 0;
  int num = 2048;
  int *a;
  int hist_seq[1024], hist_pipe1[1024], hist_pipe2[1024];
  a = (int *)malloc(sizeof(int) * num);
  for (int i = 0; i < num; i++) {
    a[i] = rand()%6;
  }
  histgram_seq(a, num, hist_seq);
  histgram_pipe1(a, num, hist_pipe1);
  histgram_pipe2(a, num, hist_pipe2);

  printf("seq    result = %d\n", hist_seq[0]);
  printf("pipe1  result = %d\n", hist_pipe1[0]);
  printf("pipe2  result = %d\n", hist_pipe2[0]);
  if(hist_seq[0]!=hist_pipe1[0] || hist_pipe1[0] != hist_pipe2[0]){
      printf("Simulation Failed\n");
      return 1;
  }
  printf("Simulation Passed\n");
  return err;
}
