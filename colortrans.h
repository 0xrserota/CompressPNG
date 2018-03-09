#include "rgbfloat.h"

#ifndef COLORTRANS_H
#define COLORTRANS_H

typedef struct Ypp_data {
        float y[4];  /* Four element array of Y */
        float pb[4]; /* Four element array of Pb */
        float pr[4]; /* Four element array of Pr */
} *Ypp_data;


Ypp_data rgbtoypp(Rgb_float_pix rfp);

Rgb_float_pix ypptorgb(Ypp_data yppd);

#endif