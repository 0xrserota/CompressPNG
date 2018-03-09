#include "ppmio.h"

#ifndef RGBFLOAT_H
#define RGBFLOAT_H

typedef struct Rgb_float_pix {
        float red[4];    /* 4 element array of red */
        float green[4];  /* 4 element array of green */
        float blue[4];   /* 4 element array of blue */
} *Rgb_float_pix;


Rgb_float_pix convert_to_floats(Rgb_unsigned_pix rup);

Rgb_unsigned_pix convert_to_uints(Rgb_float_pix rfd);

#endif