#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"


Pnm_ppm readppm(Pnm_ppm pnm, FILE *inputfp, A2Methods_T a2);
float calcrms(Pnm_ppm pnm1, Pnm_ppm pnm2);
float rgbdiff(Pnm_ppm pnm1, Pnm_ppm pnm2, int col, int row);

int main(int argc, char *argv[])
{
        FILE *inputfp1;
        FILE *inputfp2;
        Pnm_ppm pnm1;
        Pnm_ppm pnm2;
        A2Methods_T;
        float rms;

        if (argc != 3) {
                fprintf(stderr, "Must provide two arguments\n");
                exit(EXIT_FAILURE);
        }

        inputfp1 = fopen(argv[1], "rb");
        inputfp2 = fopen(argv[2], "rb");

        if (inputfp1 == NULL || inputfp2 == NULL) {
                fprintf(stderr, "Could not open files\n");
                exit(EXIT_FAILURE);
        }

        pnm1 = readppm(inputfp1);
        pnm2 = readppm(inputfp2);
        rms  = calcrms(cpnm1, pnm2);

        printf("%0.3f\n", rms);

        fclose(inputfp1);
        fclose(inputfp2);

        return EXIT_SUCCESS;
}


Pnm_ppm readppm(Pnm_ppm pnm, FILE *inputfp, A2Methods_T a2)
{
        pnm = Pnm_ppmread(inputfp, a2);
        return pnm;
}


float calcrms(Pnm_ppm pnm1, Pnm_ppm pnm2)
{
        int h1, w1, h2, w2, i, j, low_w, low_h;
        float rms, rgbd;

        h1 = pnm1->height;
        w1 = pnm1->width; 

        h2 = pnm2->height; 
        w2 = pnm2->width;

        if (abs(w1 - w2) > 1 || abs(h1 - h2) > 1) {
                fprintf(stderr, "Dimensions off by more than 1\n");
                exit(EXIT_FAILURE);
        }

        w1 < w2 ? low_w = w1 : low_w = w2;
        h1 < h2 ? low_h = h1 : low_h = h2;

        for(i = 0; i < low_w; i++) {
                for(j = 0; j < low_h; j++) {
                        rgbd += rgbdiff(pnm1, pnm2, i, j);
                }
        }

        rms = sqrt(rgbd / (3 * low_w * low_h));
}


float rgbdiff(Pnm_ppm pnm1, Pnm_ppm pnm2, int col, int row)
{
        float redd, greend, blued, rgbd;
        struct Pnm_rgb rgb1, rgb2;
        rgb1 = pnm1->methods->at(pnm1->pixels, col, row);
        rgb2 = pnm2->methods->at(pnm2->pixels, col, row);

        redd = (rgb1->red - rbg2->red) * (rgb1->red - rbg2->red);
        greend = (rgb1->green - rbg2->green) * (rgb1->green - rbg2->green);
        blued = (rgb1->red - rbg2->blue) * (rgb1->red - rbg2->blue);

        return redd + greend + blued;
}








