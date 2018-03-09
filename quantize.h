#include "colortrans.h"

#ifndef QUANTIZE_H 
#define QUANTIZE_H

/* Struct Quant_data contains the index of Pr and Pb values as unsigned and
 * a, b, c, and d as floats. Used for converting from component video values
 * to quantized data.
 */

typedef struct Quant_data {
        unsigned index_pr, index_pb;
        float a, b, c, d;
} *Quant_data;

Quant_data ypptoquant(Ypp_data yppd);
void dct_calculate(Quant_data qd, Ypp_data yppd);
void quantize_chroma(Quant_data qd, Ypp_data yppd);

Ypp_data quanttoypp(Quant_data qd);
void inverse_quantize_chroma(Quant_data qd, Ypp_data yppd);
void inverse_dct_calculate(Quant_data qd, Ypp_data yppd);

#endif