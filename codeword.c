#include "codeword.h"
#include <bitpack.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

uint32_t pack_codeword(Quant_data qd) 
{
        unsigned index_pr, index_pb;
        float a, b, c, d;

        index_pr = qd->index_pr;
        index_pb = qd->index_pb;

        a = qd->a;
        b = qd->b;
        c = qd->c;
        d = qd->d;

        //fprintf(stderr, "d: %f\n", d);

        free(qd);

        return pack_data(a, b, c, d, index_pr, index_pb);        
}

uint32_t pack_data(float a, float b, float c, float d, unsigned index_pr, 
                   unsigned index_pb)
{
        uint64_t ai, pri, pbi;
        int64_t bi, ci, di;

        pri = index_pr;
        pbi = index_pb;

        ai  = code_a_as_uint(a);
        bi  = code_bcd_as_int(b);
        ci  = code_bcd_as_int(c);
        di  = code_bcd_as_int(d);

        uint64_t codeword = 0;

        codeword = Bitpack_newu(codeword, 4, 0, pri);
        codeword = Bitpack_newu(codeword, 4, 4, pbi);
        codeword = Bitpack_news(codeword, 5, 8, di);
        codeword = Bitpack_news(codeword, 5, 13, ci);
        codeword = Bitpack_news(codeword, 5, 18, bi);
        codeword = Bitpack_newu(codeword, 9, 23, ai);

        return (uint32_t) codeword;
}

uint64_t code_a_as_uint(float a)
{
        a *= 511;
        return (uint64_t) round(a);
}

int64_t code_bcd_as_int(float n)
{  
        if (n > 0.3) {
                n = 0.3;
        } else if (n < -0.3) {
                n = -0.3;
        } 

        /* Converts float value to range {-15,...,15} */
        n *= 50;

        return (int64_t) round(n);
}        

float decode_a(float a)
{
        a /= 511;
        return a;
}

float decode_bcd(float n)
{
        /* Converts range {-15,...,15} to floats */ 
        n /= 50;
        return n;
}

Quant_data unpack_codeword(uint32_t codeword)
{
        Quant_data qd = malloc(sizeof(struct Quant_data));
        uint64_t ai, pri, pbi;
        int64_t bi, ci, di;

        float a, b, c, d;
        unsigned index_pb, index_pr;
         
        ai  = Bitpack_getu(codeword, 9, 23);
        bi  = Bitpack_gets(codeword, 5, 18);
        ci  = Bitpack_gets(codeword, 5, 13);
        di  = Bitpack_gets(codeword, 5, 8);
        pbi = Bitpack_getu(codeword, 4, 4);
        pri = Bitpack_getu(codeword, 4, 0);

        //fprintf(stderr, "ai: %lu, bi: %li, ci: %li, di: %li, pbi: %lu, pri: %lu\n", ai, bi, ci, di, pbi, pri);

        a = decode_a((float) ai);
        b = decode_bcd((float) bi);
        c = decode_bcd((float) ci);
        d = decode_bcd((float) di);
        index_pb = (unsigned) pbi;
        index_pr = (unsigned) pri;

        //fprintf(stderr, "index_pb: %u, index_pr: %u\n", index_pb, index_pr);
        //fprintf(stderr, "d: %f\n", d);

        qd->a = a;
        qd->b = b;
        qd->c = c;
        qd->d = d;
        qd->index_pb = index_pb;
        qd->index_pr = index_pr;

        return qd;
}