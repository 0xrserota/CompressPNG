#include <stdint.h>
#include <stdio.h>
#include "bitpack.h"
#include "assert.h"

/* Function Bitpack_fitsu takes in a uint64_t and an unsigned and returns a 
 * bool. Checks to see if the given uint64_t can fit in the given width of
 * bits.
 */

bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        assert(width <= 64);
        /* Computes 2^width */
        uint64_t m = 1 << width;

        if (n < m) {
                return true;
        } else {
                return false;
        }
}

/* Function Bitpack_fitss takes in an int64_t and an unsigned and returns a 
 * bool. Checks to see if the given int64_t can fit within the given width of
 * bits.
 */

bool Bitpack_fitss(int64_t n, unsigned width)
{
        assert(width <= 64);
        /* Computes 2^(width - 1) */
        int64_t m = 1 << (width - 1);

        if (n <= m - 1 && n >= (m * -1)) {
                return true;
        } else {
                return false;
        }
}

/* Function Bitpack_getu takes in a uint64_t and 2 unsigned and returns a
 * uint64_t. Gets the bits from the given uint64_t in the width starting 
 * at the given least significant bit.
 */

uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width + lsb <= 64);
        if(width == 0) {
                return 0;
        }
        return word << (64 - (width + lsb)) >> (64 - width);
}

/* Function Bitpack_gets takes in a int64_t and 2 unsigned and returns an
 * int64_t. Gets the bits from the given int64_t in the width starting 
 * at the given least significant bit.
 */

int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width + lsb <= 64);
        if(width == 0) {
                return 0;
        }

        int64_t bits = word << (64 - (width + lsb)) >> (64 - width);
        int64_t bittest = bits >> (width - 1);
        int64_t newbit = -1;
        if(bittest == 0) {
                return bits;
        } else {
                newbit = newbit << width;
                return (bits | newbit);
        }
}

/* Function Bitpack_newu takes in a 2 uint64_t's and 2 unsigned and returns a 
 * uint64_t. Puts the given uint64_t in the given codeword in the given 
 * width at the given least significant bit.
 */

uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, uint64_t
                      value)
{
        assert(width + lsb <= 64);
        if(width == 0) {
                return 0;
        } else if(!(Bitpack_fitsu(value, width))) {
                RAISE(Bitpack_Overflow);
        }

        uint64_t bits1 = Bitpack_getu(word, lsb, 0);
        word = word >> (width + lsb) << (width + lsb);
        value = value << lsb;
        return word | bits1 | value;
}

/* Function Bitpack_news takes in an int64_t and a uint64_t and 2 unsigned and
 * returns a uint64_t. Puts the given int64_t in the given codeword in the
 * given width at the given least significant bit.
 */

uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, 
                      int64_t value)
{
        assert(width + lsb <= 64);
        if(width == 0) {
                return 0;
        } else if(!(Bitpack_fitss(value, width))) {
                RAISE(Bitpack_Overflow);
        }

        value = value << lsb;
        uint64_t bits1 = Bitpack_getu(word, lsb, 0);
        word = word >> (width + lsb) << (width + lsb);
        return word | bits1 | value;
}

#include "except.h"
Except_T Bitpack_Overflow = { "Overflow packing bits" };
