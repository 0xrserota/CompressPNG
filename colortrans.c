#include "colortrans.h"
#include "quantize.h"
#include <stdlib.h>
#include <stdio.h>

/* Function rbgtoypp takes in an Rgb_float_pix struct and returns a Ypp_data 
 * struct. Converts the float rgb values to brightness and Pb and Pr using
 * the supplied formulas and adds them to a struct which is returned.
 */

Ypp_data rgbtoypp(Rgb_float_pix rfp) 
{
        Ypp_data yppd = malloc(sizeof(struct Ypp_data));

        float r,g,b;

        int i;
        for (i = 0; i < 4; i++) {

                r = rfp->red[i];
                g = rfp->green[i];
                b = rfp->blue[i];

                yppd->y[i]  = 0.299 * r + 0.587 * g + 0.114 * b; 
                yppd->pb[i] = -0.168736 * r - 0.331264 * g + 0.5 * b;
                yppd->pr[i] = 0.5 * r - 0.418688 * g - 0.081312 * b;
        }

        free(rfp);

        return yppd;
}

/* Function ypptorgb takes in a Ypp_data struct and returns a Rgb_float_pix 
 * struct. Converts the video component data to rgb float data using the 
 * given formulas and adds the data to a struct.
 */

Rgb_float_pix ypptorgb(Ypp_data yppd)
{
        Rgb_float_pix rfp = malloc(sizeof(struct Rgb_float_pix));

        float y,pb,pr;

        int i;
        for (i = 0; i < 4; i++) {

                y = yppd->y[i];
                pb = yppd->pb[i];
                pr = yppd->pr[i];

                rfp->red[i]   = 1.0 * y + 0.0 * pb + 1.402 * pr;
                rfp->green[i] = 1.0 * y - 0.344136 * pb - 0.714136 * pr;
                rfp->blue[i]  = 1.0 * y + 1.772 * pb + 0.0 * pr;
        }

        free(yppd);

        return rfp;
}

