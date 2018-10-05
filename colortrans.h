#include "rgbfloat.h"

#ifndef COLORTRANS_H
#define COLORTRANS_H

/* Struct Ypp_data contains 3 arrays of floats that are the brightness's of
 * the block of pixels and the Pb and Pr values. Used to convert from rbg 
 * values to component video values.
 */

typedef struct Ypp_data {
        float y[4];  /* Four element array of Y */
        float pb[4]; /* Four element array of Pb */
        float pr[4]; /* Four element array of Pr */
} *Ypp_data;


Ypp_data rgbtoypp(Rgb_float_pix rfp);

Rgb_float_pix ypptorgb(Ypp_data yppd);

#endif