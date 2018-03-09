#include "quantize.h"

#ifndef CODEWORD_H
#define CODEWORD_H

uint32_t pack_codeword(Quant_data qd);
uint32_t pack_data(float a, float b, float c, float d, unsigned index_pr, 
                   unsigned index_pb);
uint64_t code_a_as_uint(float a);
int64_t code_bcd_as_int(float n);


Quant_data unpack_codeword(uint32_t codeword);
float decode_a(float a);
float decode_bcd(float n);

#endif