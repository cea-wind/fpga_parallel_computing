#include "filter_1d.h"

void filter_1d(int *a, int filter_kernel[3], int *b) {
#pragma HLS INTERFACE m_axi depth = 2048 port = a
#pragma HLS INTERFACE m_axi depth = 2048 port = b
#pragma HLS ARRAY PARTITION variable = filter_kernel complete dim = 1
  int reg[3] = { 0, 0, 0 } for (int i = 0; i < num; i++) {
#pragma HLS PIPELINE II = 1
    reg[0] = reg[1];
    reg[1] = reg[2];
    reg[2] = a[i];
    b[i] = filter_kernel[0] * reg[0] + filter_kernel[1] * reg[1] +
           filter_kernel[2] * reg[2];
  }
}

void filter_2d(uchar *a, int width, int height, int filter_kernel[3][3],
               uchar *b) {
#pragma HLS INTERFACE m_axi depth = 2048 port = a
#pragma HLS INTERFACE m_axi depth = 2048 port = b
#pragma HLS ARRAY_PARTITION variable = filter_kernel complete dim = 0
  uchar line_buffer[2][IMG_MAX_WIDTH];
  uchar reg[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
  for (int i = 0; i < width; i++) {
#pragma HLS PIPELINE II = 1
    line_buffer[0][i] = 0;
    line_buffer[1][i] = 0;
  }
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
#pragma HLS PIPELINE II = 1
      for (int k = 0; k < 2; k++) {
#pragma HLS UNROLL
        reg[0][k] = reg[0][k + 1];
        reg[1][k] = reg[1][k + 1];
        reg[2][k] = reg[2][k + 2];
      }
      reg[0][2] = line_buffer[(i + 0) % 2][i];
      reg[1][2] = line_buffer[(i + 1) % 2][i];
      reg[2][2] = a[i * width + j];

      line_buffer[(i + 0) % 2][i] = reg[2][2];
    }
  }
}