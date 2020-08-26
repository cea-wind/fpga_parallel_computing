#include "histgram.h"

void histgram_seq(int *a, int num, int *hist) {
  int local_hist[1024];
  for (int i = 0; i < 1023; i++) {
    local_hist[i] = 0;
  }
  for (int i = 0; i < num; i++) {
    int rega = a[i];
    if (rega < 0) rega = 0;
    if (rega > 1023) rega = 1023;
    local_hist[rega]++;
  }
  for (int i = 0; i < 1023; i++) {
    hist[i] = local_hist[i];
  }
}

void histgram_pipe1(int *a, int num, int *hist) {
#pragma HLS INTERFACE m_axi depth = 2048 port = a
#pragma HLS INTERFACE m_axi depth = 2048 port = hist
  int local_hist[1024];
  for (int i = 0; i < 1024; i++) {
#pragma HLS PIPELINE II = 1
    local_hist[i] = 0;
  }
  int acc = 0;
  int old, cur;
  for (int i = 0; i < num; i++) {
#pragma HLS DEPENDENCE variable = local_hist array intra RAW false
#pragma HLS PIPELINE II = 1
    cur = a[i];
    if (cur < 0) cur = 0;
    if (cur > 1023) cur = 1023;
    if (i == 0 || old == cur) {
      acc = acc + 1;
    } else {
      local_hist[old] = acc;
      acc = local_hist[cur] + 1;
    }
    old = cur;
  }
  local_hist[old] = acc;
  for (int i = 0; i < 1024; i++) {
#pragma HLS PIPELINE II = 1
    hist[i] = local_hist[i];
  }
}

void cached_array_init(int local_hist[1024], int cache_reg[4],
                       int cache_idx[4]) {
#pragma HLS INLINE
  for (int i = 0; i < 1024; i++) {
#pragma HLS PIPELINE II = 1
    local_hist[i] = 0;
  }
  for (int i = 0; i < 4; i++) {
#pragma HLS UNROLL
    cache_reg[i] = 0;
    cache_idx[i] = 0;
  }
}
int cached_array_read(int local_hist[1024], int cache_reg[4], int cache_idx[4],
                      int idx) {
#pragma HLS INLINE
  for (int i = 0; i < 4; i++) {
#pragma HLS UNROLL
    if (idx == cache_idx[i]) return cache_reg[i];
  }
  return local_hist[idx];
}
void cached_array_write(int local_hist[1024], int cache_reg[4], int cache_idx[4],
                       int idx, int val) {
#pragma HLS INLINE
  for (int i = 3; i > 0; i--) {
#pragma HLS UNROLL
    cache_reg[i] = cache_reg[i - 1];
    cache_idx[i] = cache_idx[i - 1];
  }
  cache_reg[0] = val;
  cache_idx[0] = idx;
  local_hist[idx] = val;
}
void histgram_pipe2(int *a, int num, int *hist) {
#pragma HLS INTERFACE m_axi depth = 2048 port = a
#pragma HLS INTERFACE m_axi depth = 2048 port = hist
  int local_hist[1024];
#pragma HLS RESOURCE variable=local_hist core=RAM_S2P_BRAM latency=4
  int cache_reg[4];
  int cache_idx[4];
  cached_array_init(local_hist, cache_reg, cache_idx);
  int rega;
  for (int i = 0; i < num; i++) {
#pragma HLS DEPENDENCE variable=local_hist inter RAW distance=5 true
#pragma HLS PIPELINE II = 1
    rega = a[i];
    if (rega < 0) rega = 0;
    if (rega > 1023) rega = 1023;
    int update_val =
        cached_array_read(local_hist, cache_reg, cache_idx, rega) + 1;
    cached_array_write(local_hist, cache_reg, cache_idx, rega, update_val);
  }
  for (int i = 0; i < 1024; i++) {
#pragma HLS PIPELINE II = 1
    hist[i] = cached_array_read(local_hist, cache_reg, cache_idx, i);
  }
}
