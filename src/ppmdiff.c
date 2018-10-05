#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"


Pnm_ppm readppm(FILE *inputfp, A2Methods_T a2);
float calcrms(Pnm_ppm pnm1, Pnm_ppm pnm2);
float rgbdiff(Pnm_ppm pnm1, Pnm_ppm pnm2, int col, int row);

int main(int argc, char *argv[])
{
        FILE *inputfp1;
        FILE *inputfp2;
        Pnm_ppm pnm1;
        Pnm_ppm pnm2;
        A2Methods_T a2 = uarray2_methods_plain;
        float rms = 0;

        if (argc != 3) {
                fprintf(stderr, "Must provide two arguments\n");
                exit(EXIT_FAILURE);
        }

        inputfp1 = fopen(argv[1], "r");
        inputfp2 = fopen(argv[2], "r");

        if (inputfp1 == NULL || inputfp2 == NULL) {
                fprintf(stderr, "Could not open files\n");
                exit(EXIT_FAILURE);
        }

        pnm1 = readppm(inputfp1, a2);
        pnm2 = readppm(inputfp2, a2);
        rms  = calcrms(pnm1, pnm2);

        printf("%0.4f\n", rms);

        fclose(inputfp1);
        fclose(inputfp2);

        Pnm_ppmfree(&pnm1);
        Pnm_ppmfree(&pnm2);

        return EXIT_SUCCESS;
}


Pnm_ppm readppm(FILE *inputfp, A2Methods_T a2)
{
        return Pnm_ppmread(inputfp, a2);
}


float calcrms(Pnm_ppm pnm1, Pnm_ppm pnm2)
{
        int h1, w1, h2, w2, i, j, low_w, low_h;
        float rgbd = 0;

        h1 = pnm1->height;
        w1 = pnm1->width; 

        h2 = pnm2->height; 
        w2 = pnm2->width;

        if (abs(w1 - w2) > 1 || abs(h1 - h2) > 1) {
                fprintf(stderr, "Dimensions off by more than 1\n");
                exit(EXIT_FAILURE);
        }

        w1 < w2 ? (low_w = w1) : (low_w = w2);
        h1 < h2 ? (low_h = h1) : (low_h = h2);

        for(i = 0; i < low_w; i++) {
                for(j = 0; j < low_h; j++) {
                        rgbd += rgbdiff(pnm1, pnm2, i, j);
                }
        }

        return sqrt(rgbd / (3 * low_w * low_h));
}


float rgbdiff(Pnm_ppm pnm1, Pnm_ppm pnm2, int col, int row)
{
        float redd = 0, greend = 0, blued = 0;
        Pnm_rgb rgb1, rgb2;
        rgb1 = (Pnm_rgb) pnm1->methods->at(pnm1->pixels, col, row);
        rgb2 = (Pnm_rgb) pnm2->methods->at(pnm2->pixels, col, row);



        redd   =  ((((float) rgb1->red) / 255) - (((float) rgb2->red) / 255)) *
                  ((((float) rgb1->red) / 255) - (((float) rgb2->red) / 255));

        greend =  ((((float) rgb1->green) / 255) - (((float) rgb2->green) / 255)) 
                * ((((float) rgb1->green) / 255) - (((float) rgb2->green) / 255));

        blued  =  ((((float) rgb1->blue) / 255) - (((float) rgb2->blue) / 255)) *
                  ((((float) rgb1->blue) / 255) - (((float) rgb2->blue) / 255));

        //fprintf(stderr, "Red 1: %u, Red 2: %u\n", rgb1->red, rgb2->red);
        //fprintf(stderr, "Green 1: %u, Green 2: %u\n", rgb1->green, rgb2->green);
        //fprintf(stderr, "Blue 1: %u, Blue 2: %u\n", rgb1->blue, rgb2->blue);

        return redd + greend + blued;
}