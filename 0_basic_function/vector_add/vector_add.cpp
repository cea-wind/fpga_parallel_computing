#include "vector_add.h"

void vector_add_seq(
    int *a, int *b, int num,
    int *c
) {
#pragma HLS INTERFACE m_axi depth=2048 port=a
#pragma HLS INTERFACE m_axi depth=2048 port=b
#pragma HLS INTERFACE m_axi depth=2048 port=c
    for(int i=0;i<num;i++){
        c[i] = a[i] + b[i];
    }
}

void vector_add_pipe(
    int *a, int *b, int num,
    int *c
) {
#pragma HLS INTERFACE m_axi depth=2048 port=a
#pragma HLS INTERFACE m_axi depth=2048 port=b
#pragma HLS INTERFACE m_axi depth=2048 port=c
    for(int i=0;i<num;i++){
    #pragma HLS PIPELINE II=1
        c[i] = a[i] + b[i];
    }
}

void vector_add_parallel(
    ap_uint<512> *a, ap_uint<512> *b, int num,
	ap_uint<512> *c
) {
#pragma HLS INTERFACE m_axi depth=256 port=a
#pragma HLS INTERFACE m_axi depth=256 port=b
#pragma HLS INTERFACE m_axi depth=256 port=c

    for(int i=0;i<num;i=i+16){
    #pragma HLS PIPELINE II=1
    	ap_uint<512> rega,regb,regc;
        rega = a[i/16];
        regb = b[i/16];
        for(int j=0;j<16;j++){
        #pragma HLS UNROLL
            regc(32*j+31,32*j) = rega(32*j+31,32*j) +regb(32*j+31,32*j);
        }
        c[i/16] = regc;
    }
}
