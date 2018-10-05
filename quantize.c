#include "quantize.h"
#include <arith40.h>
#include <stdio.h>
#include <stdlib.h>

/* Function ypptoquant takes in a Ypp_data struct and returns a Quant_data 
 * struct. Calls the helper function which quantizes the video component data
 * and adds the new data to a struct and returns it.
 */

Quant_data ypptoquant(Ypp_data yppd)
{
        Quant_data qd = malloc(sizeof(struct Quant_data));
        dct_calculate(qd, yppd);
        quantize_chroma(qd, yppd);

        free(yppd);

        return qd;
}

/* Function dct_calculate takes in a Quant_data struct and a Ypp_data struct 
 * and has void return type. Calculates the values of a, b, c, and d using
 * the given functions and adds the data to the new struct.
 */

void dct_calculate(Quant_data qd, Ypp_data yppd)
{
        float *y = yppd->y;

        qd->a = (y[3] + y[2] + y[1] + y[0]) / 4.0; 
        qd->b = (y[3] + y[2] - y[1] - y[0]) / 4.0;
        qd->c = (y[3] - y[2] + y[1] - y[0]) / 4.0;
        qd->d = (y[3] - y[2] - y[1] + y[0]) / 4.0;
}

/* Function quantize_chroma takes in a Quant_data struct and a Ypp_data struct
 * and has void return type. Finds the mean of the Pb and Pr values and uses
 * the given functions to get the index of the chroma values.
 */

void quantize_chroma(Quant_data qd, Ypp_data yppd)
{
        float sum_pr = 0, sum_pb = 0, mean_pr = 0, mean_pb = 0;
        int i;
        for (i = 0; i < 4; i++) {
                sum_pr += yppd->pr[i];
                sum_pb += yppd->pb[i];
        }

        mean_pr = sum_pr / 4;
        mean_pb = sum_pb / 4;

        qd->index_pr = Arith40_index_of_chroma(mean_pr);
        qd->index_pb = Arith40_index_of_chroma(mean_pb);
}

/* Function quanttoypp takes in a Quant_data struct and returns a Ypp_data 
 * struct. Calls helper functions to convert the quantized data to component
 * video data and returns a struct with the data.
 */

Ypp_data quanttoypp(Quant_data qd)
{
        Ypp_data yppd = malloc(sizeof(struct Ypp_data));
        inverse_dct_calculate(qd, yppd);
        inverse_quantize_chroma(qd, yppd);

        free(qd);

        return yppd;
}

/* Function inverse_quantize_chroma takes in a Quant_data struct and a Ypp_data
 * struct and has void return type. Uses the given functions to get the Pb 
 * and Pr values for the block of pixels and adds them to the struct.
 */

void inverse_quantize_chroma(Quant_data qd, Ypp_data yppd)
{
        float pr = Arith40_chroma_of_index(qd->index_pr);
        float pb = Arith40_chroma_of_index(qd->index_pb);

        int i;
        for (i = 0; i < 4; i++) {
                yppd->pr[i] = pr;
                yppd->pb[i] = pb; 
        } 
}

/* Function iverse_dct_calculate takes in a Quant_data struct and a Ypp_data 
 * struct and has void return type. Uses the given functions to convert from
 * a, b, c, and d values to brightness values.
 */

void inverse_dct_calculate(Quant_data qd, Ypp_data yppd)
{
        float a, b, c, d;

        a = qd->a;
        b = qd->b;
        c = qd->c;
        d = qd->d;

        yppd->y[0] = a - b - c + d;
        yppd->y[1] = a - b + c - d;
        yppd->y[2] = a + b - c - d;
        yppd->y[3] = a + b + c + d;
}