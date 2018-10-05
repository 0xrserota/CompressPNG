#include "compress40.h"
#include "ppmio.h"

void compress40(FILE *input)
{
        ppmio_compress(input);
}

void decompress40(FILE *input)
{
        ppmio_decompress(input);
}