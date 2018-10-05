#include "codeword.h"
#include <bitpack.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Function pack_codeword takes in a Quant_data struct and returns a uint32_t
 * codeword. Calls helper functions to pack the data.
 */

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

        free(qd);

        return pack_data(a, b, c, d, index_pr, index_pb);        
}

/* Function pack_data takes in 4 floats and 2 unsigned and returns a uint32_t
 * codeword. Uses Bitpack functions too add all of the data into specific
 * places in the created codeword.
 */

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
        codeword = Bitpack_news(codeword, 6, 8, di);
        codeword = Bitpack_news(codeword, 6, 14, ci);
        codeword = Bitpack_news(codeword, 6, 20, bi);
        codeword = Bitpack_newu(codeword, 6, 26, ai);

        return (uint32_t) codeword;
}

/* Function code_a_as_uint takes in a float and returns a uint64_t. Multiplies
 * a by 63 so that it is in the correct range and returns it as a uint64_t.
 */

uint64_t code_a_as_uint(float a)
{
        a *= 63;
        return (uint64_t) round(a);
}

/* Function code_bcd_as_int takes in a float and returns an int64_t. Checks
 * if the float is out of the range and then multiplies by 100 so that they
 * are in the right range and returns it as an int64_t.
 */

int64_t code_bcd_as_int(float n)
{  
        if (n > 0.3) {
                n = 0.3;
        } else if (n < -0.3) {
                n = -0.3;
        } 

        /* Converts float value to range {-15,...,15} */
        n *= 100;

        return (int64_t) round(n);
} 

/* Function decode_a takes in a float and returns a float. Divides the passed
 * in value by 63.
 */       

float decode_a(float a)
{
        a /= 63;
        return a;
}

/* Function decode_bcd takes in a float and returns a float. Divides the given
 * value by 100.
 */

float decode_bcd(float n)
{
        /* Converts range {-30,...,30} to floats */ 
        n /= 100;
        return n;
}

/* Function unpack_codeword takes in a uint32_t and returns a Quant_data 
 * struct. Uses Bitpack functions to get the specific bits from the codeword
 * adds the values to a struct and returns it.
 */

Quant_data unpack_codeword(uint32_t codeword)
{
        Quant_data qd = malloc(sizeof(struct Quant_data));
        uint64_t ai, pri, pbi;
        int64_t bi, ci, di;

        float a, b, c, d;
        unsigned index_pb, index_pr;
         
        ai  = Bitpack_getu(codeword, 6, 26);
        bi  = Bitpack_gets(codeword, 6, 20);
        ci  = Bitpack_gets(codeword, 6, 14);
        di  = Bitpack_gets(codeword, 6, 8);
        pbi = Bitpack_getu(codeword, 4, 4);
        pri = Bitpack_getu(codeword, 4, 0);

        a = decode_a((float) ai);
        b = decode_bcd((float) bi);
        c = decode_bcd((float) ci);
        d = decode_bcd((float) di);
        index_pb = (unsigned) pbi;
        index_pr = (unsigned) pri;

        qd->a = a;
        qd->b = b;
        qd->c = c;
        qd->d = d;
        qd->index_pb = index_pb;
        qd->index_pr = index_pr;

        return qd;
}