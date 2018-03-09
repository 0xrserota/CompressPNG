#include "quantize.h"
#include <arith40.h>
#include <stdio.h>
#include <stdlib.h>

Quant_data ypptoquant(Ypp_data yppd)
{
        Quant_data qd = malloc(sizeof(struct Quant_data));
        dct_calculate(qd, yppd);
        quantize_chroma(qd, yppd);

        free(yppd);

        return qd;

/*
        printf("a: %f, b: %f, c: %f, d: %f\n", qd->a, qd->b, qd->c, qd->d);
        printf("index_pr: %u, index_pb: %u\n", qd->index_pr, qd->index_pb);
*/
}

void dct_calculate(Quant_data qd, Ypp_data yppd)
{
        float *y = yppd->y;

        qd->a = (y[3] + y[2] + y[1] + y[0]) / 4.0; 
        qd->b = (y[3] + y[2] - y[1] - y[0]) / 4.0;
        qd->c = (y[3] - y[2] + y[1] - y[0]) / 4.0;
        qd->d = (y[3] - y[2] - y[1] + y[0]) / 4.0;

        //fprintf(stderr, "a: %f b: %f c: %f d: %f\n",qd->a, qd->b, qd->c, qd->d);
        /*
        for (int i = 0; i < 4; i++) {
                printf("y[%d]: %f\n", i, y[i]);
        }
        */
}

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

        //printf("mean_pr: %f, mean_pb: %f\n", mean_pr, mean_pb);

        qd->index_pr = Arith40_index_of_chroma(mean_pr);
        qd->index_pb = Arith40_index_of_chroma(mean_pb);
}

Ypp_data quanttoypp(Quant_data qd)
{
        Ypp_data yppd = malloc(sizeof(struct Ypp_data));
        inverse_dct_calculate(qd, yppd);
        inverse_quantize_chroma(qd, yppd);

        free(qd);

        return yppd;
}


void inverse_quantize_chroma(Quant_data qd, Ypp_data yppd)
{
        float pr = Arith40_chroma_of_index(qd->index_pr);
        float pb = Arith40_chroma_of_index(qd->index_pb);

        //fprintf(stderr, "pr: %f, pb: %f\n", pr, pb);

        int i;
        for (i = 0; i < 4; i++) {
                yppd->pr[i] = pr;
                yppd->pb[i] = pb; 
        } 
}

void inverse_dct_calculate(Quant_data qd, Ypp_data yppd)
{
        float a, b, c, d;

        a = qd->a;
        b = qd->b;
        c = qd->c;
        d = qd->d;

        //fprintf(stderr, "qd->a: %f, qd->b: %f, qd->c: %f, qd->d: %f\n",
        //qd->a, qd->b, qd->c, qd->d);
        //fprintf(stderr, "a: %f, b: %f, c: %f, d: %f\n", a,b,c,d);

        yppd->y[0] = a - b - c + d;
        yppd->y[1] = a - b + c - d;
        yppd->y[2] = a + b - c - d;
        yppd->y[3] = a + b + c + d;

        /*
        for(int i = 0; i < 4; i ++) {
                fprintf(stderr, "yppd->y[%d]: %f\n", i, yppd->y[i]);
        }
        */
}