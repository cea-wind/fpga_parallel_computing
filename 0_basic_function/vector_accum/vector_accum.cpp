#include "vector_accum.h"

void vector_accum_seq(
    float *a, int num,
    float *b
) {
#pragma HLS INTERFACE m_axi depth=2048 port=a
#pragma HLS INTERFACE m_axi depth=2048 port=b
    b[0] = 0;
    for(int i=0;i<num;i++){
        b[0] += a[i];
    }
}

void vector_accum_pipe_bad(
    float *a, int num,
    float *b
) {
#pragma HLS INTERFACE m_axi depth=2048 port=a
#pragma HLS INTERFACE m_axi depth=2048 port=b
    float regb = 0;
    for(int i=0;i<num;i++){
    #pragma HLS PIPELINE II=1
        regb += a[i];
    }
    b[0] = regb;
}

void vector_accum_pipe_good(
    float *a, int num,
    float *b
) {
#pragma HLS INTERFACE m_axi depth=2048 port=a
#pragma HLS INTERFACE m_axi depth=2048 port=b
    float regb[8] = {0,0,0,0,0,0,0,0};
    for(int i=0;i<num;i++){
    #pragma HLS PIPELINE II=1
        regb[i%8] += a[i];
    }
    for(int i=1;i<8;i++){
    #pragma HLS PIPELINE II=8
        regb[0] += regb[i];
    }
    b[0] = regb[0];
}

void vector_accum_parallel(
    ap_uint<512> *a, int num,
	float *b
) {
#pragma HLS INTERFACE m_axi depth=256 port=a
#pragma HLS INTERFACE m_axi depth=256 port=b
    float regb[16][8];
    #pragma HLS ARRAY_PARTITION variable=regb complete dim=1
    // initial regb
    for(int i=0;i<8;i++){
    #pragma HLS PIPELINE II=1
        for(int j=0;j<16;j++){
        #pragma HLS UNROLL
            regb[j][i] = 0.0f;
        }
    }
    // load 16 data in parallel
    for(int i=0;i<(num+15)/16;i++){
    #pragma HLS PIPELINE II=1
    	ap_uint<512> rega;
        rega = a[i];
        for(int j=0;j<16;j++){
        #pragma HLS UNROLL
        	int temp = rega(32*j+31,32*j);
            regb[j][i%8] += *(float *)&temp;
        }
    }  
    // reduce 16 parallel result
    float regc = 0.0f;  
    for(int i=0;i<8;i++){
        for(int j=0;j<16;j++){
		#pragma HLS PIPELINE II=8
            regc += regb[j][i];
        }
    }
    b[0] = regc;
}
