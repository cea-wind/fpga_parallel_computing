#include "prefix_sum.h"

void prefix_sum_seq(int *a, int num, int *b) {
#pragma HLS INTERFACE m_axi depth = 2048 port = a
#pragma HLS INTERFACE m_axi depth = 2048 port = b
  b[0] = a[0];
  for (int i = 1; i < num; i++) {
    b[i] = b[i - 1] + a[i];
  }
}

void prefix_sum_pipe(int *a, int num, int *b) {
#pragma HLS INTERFACE m_axi depth = 2048 port = a
#pragma HLS INTERFACE m_axi depth = 2048 port = b
  int regsum = 0;
  for (int i = 0; i < num; i++) {
#pragma HLS PIPELINE II = 1
    regsum = regsum + a[i];
    b[i] = regsum;
  }
}

void prefix_sum_parallel(ap_uint<512> *a, int num, ap_uint<512> *b) {
#pragma HLS INTERFACE m_axi depth = 2048 port = a
#pragma HLS INTERFACE m_axi depth = 2048 port = b
  ap_uint<512> rega, regb;
  int regs1[16], regs2[16], regs3[16], regs4[16];
  int regprev = 0;
  int batch_num = (num + 15) / 16;
  for (int i = 0; i < batch_num; i = i + 1) {
#pragma HLS PIPELINE II = 1
    rega = a[i];
    for (int j = 0; j < 16; j++) {
      if (j >= 1)
        regs1[j] = rega(32 * j + 31, 32 * j) + rega(32 * j - 1, 32 * j - 32);
      else
        regs1[j] = rega(31, 0);
    }
    for (int j = 0; j < 16; j++) {
      if (j >= 2)
        regs2[j] = regs1[j] + regs1[j - 2];
      else
        regs2[j] = regs1[j];
    }
    for (int j = 0; j < 16; j++) {
      if (j >= 4)
        regs3[j] = regs2[j] + regs2[j - 4];
      else
        regs3[j] = regs2[j];
    }
    for (int j = 0; j < 16; j++) {
      if (j >= 8)
        regs4[j] = regs3[j] + regs3[j - 8];
      else
        regs4[j] = regs3[j];
    }
    for (int j = 0; j < 16; j++) {
      regb(32 * j + 31, 32 * j) = regs4[j] + regprev;
    }
    regprev = regb(511, 480);
    b[i] = regb;
  }
}
