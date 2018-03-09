#include "rgbfloat.h"
#include "colortrans.h"
#include <stdlib.h>

/* Function convert_to_floats takes in an Rgb_unsigned_pix struct and returns
 * a Rgb_float_pix struct. Converts the rgb values to floats and divides them
 * by the denominator.
 */

Rgb_float_pix convert_to_floats(Rgb_unsigned_pix rup) 
{
        Rgb_float_pix rfp = malloc(sizeof(struct Rgb_float_pix));

        unsigned denominator = rup->denominator;

        int i;
        for (i = 0; i < 4; i++) {


                rfp->red[i]   = ((float) rup->red[i])   / denominator;
                rfp->green[i] = ((float) rup->green[i]) / denominator;
                rfp->blue[i]  = ((float) rup->blue[i])  / denominator;
        }

        return rfp;
}

/* Function convert_to_uints takes in a Rgb_float_pix struct and returns a 
 * Rgb_unsigned_pix struct. Converts the rgb values to unsigned and multiples
 * by the denominator.
 */

Rgb_unsigned_pix convert_to_uints(Rgb_float_pix rfp)
{
        Rgb_unsigned_pix rup = malloc(sizeof(struct Rgb_unsigned_pix));

        unsigned denominator = rup->denominator = 255;

        int i;
        for (i = 0; i < 4; i++) {

                /* Checks to see if float values are negative */
                if(rfp->red[i] < 0)
                        rfp->red[i] = 0;
                if(rfp->green[i] < 0)
                        rfp->green[i] = 0;
                if(rfp->blue[i] < 0)
                        rfp->blue[i] = 0;

                rup->red[i]   = (unsigned) (rfp->red[i]   * denominator);
                rup->green[i] = (unsigned) (rfp->green[i] * denominator);
                rup->blue[i]  = (unsigned) (rfp->blue[i] * denominator);

                /* Checks to see if new rgb values are over the denominator */
                if(rup->red[i] > denominator)
                        rup->red[i] = denominator;
                if(rup->green[i] > denominator)
                        rup->green[i] = denominator;
                if(rup->blue[i] > denominator)
                        rup->blue[i] = denominator;
        }
        rup->size = 4;

        free(rfp);

        return rup;
}