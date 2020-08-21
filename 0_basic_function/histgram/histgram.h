#ifndef __HISTGRAM_H__
#define __HISTGRAM_H__

#include "ap_int.h"

void histgram_seq(int *a, int num, int *hist);

void histgram_pipe1(int *a, int num, int *hist);

void histgram_pipe2(int *a, int num, int *hist);

#endif