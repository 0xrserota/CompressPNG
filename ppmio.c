/* MODULE 1 
   ppmio */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "ppmio.h"
#include "assert.h"

#include "rgbfloat.h"
#include "colortrans.h"
#include "quantize.h"
#include "bitpack.h"
#include "codeword.h"

/* COMPRESSION FUNCTIONS */
void ppmio_compress(FILE *inputfp)
{
        A2Methods_T plain_methods = uarray2_methods_plain;
        A2Methods_T blocked_methods = uarray2_methods_blocked;

        Pnm_ppm pnm = ppmread(inputfp, plain_methods);

        A2Methods_UArray2 uray2b = make_uray2_blocked(pnm, blocked_methods);

        Rgb_unsigned_pix rup = malloc(sizeof(struct Rgb_unsigned_pix));
        assert(rup != NULL);

        rup->counter = 0;
        rup->size = 0;
        rup->denominator = pnm->denominator;

        //fprintf(stderr, "denominator: %u\n", pnm->denominator);

        int newWidth  = blocked_methods->width(uray2b);
        int newHeight = blocked_methods->height(uray2b);

        compress_data cd = malloc(sizeof(struct compress_data));
        cd->codeword_queue = Seq_new((newWidth * newHeight) / 4);
        cd->rup = rup;

        blocked_methods->map_block_major(uray2b, compress_four_pixels, cd);
        write_codewords(cd->codeword_queue, newWidth, newHeight);

        free(rup);
        Seq_free(&(cd->codeword_queue));
        free(cd);

        blocked_methods->free(&uray2b);
        Pnm_ppmfree(&pnm);
}

Pnm_ppm ppmread(FILE* inputfp, A2Methods_T methods)
{
        assert(inputfp != NULL && methods == uarray2_methods_plain);
        return Pnm_ppmread(inputfp, methods);
}

A2Methods_UArray2 make_uray2_blocked(Pnm_ppm pnm, A2Methods_T blocked_methods) 
{
        assert(pnm != NULL && blocked_methods == uarray2_methods_blocked);
        A2Methods_UArray2 uray2b;
        int newWidth, newHeight;

        
        
        if ((pnm->height % 2) != 0) {
                newHeight = --(pnm->height);
        } 
        else
                newHeight = pnm->height;

        if ((pnm->width % 2) != 0) {
                newWidth = --(pnm->width);
        }
        else {
                newWidth = pnm->width;
        }
        
        uray2b = blocked_methods->new_with_blocksize(newWidth, newHeight, 
                                                    sizeof(struct Pnm_rgb), 2);

        copy_data cp = malloc(sizeof(struct copy_data));
        cp->uray2b = uray2b;
        cp->methods = blocked_methods;

        pnm->methods->map_row_major(pnm->pixels, copy_uarray2, cp);
        free(cp);

        return uray2b;
}

void copy_uarray2(int col, int row, A2Methods_UArray2 pixels, 
                  void *x, void *cl)
{
        assert(cl != NULL && x != NULL);
        //bool edgeTest = true;

        (void) pixels;
        copy_data cd = (copy_data) cl;

        Pnm_rgb rgb = (Pnm_rgb) x;
        int width = cd->methods->width(cd->uray2b);
        int height = cd->methods->height(cd->uray2b);

        if (col < width && row < height) {
                *((Pnm_rgb) (cd->methods->at(cd->uray2b, col, row)))
                                                           = *rgb;
        }
}

void compress_four_pixels(int col, int row, A2Methods_UArray2 uray2b,
                                     void *x, void *cl)
{
        assert(x != NULL || cl != NULL);

        (void) col;
        (void) row;
        (void) uray2b;

        Pnm_rgb rgb = (Pnm_rgb) x;


        compress_data cd = (compress_data) cl;
        Rgb_unsigned_pix rup = cd->rup;
        int size = rup->size;
/*
        if(rgb->blue > 240) {
            fprintf(stderr, "???\n");
        }
*/
        //fprintf(stderr, "red: %u\n", rgb->red);
        rup->red[size]   = rgb->red;
        rup->green[size] = rgb->green;
        rup->blue[size]  = rgb->blue;
        rup->size++;

        if (rup->size == 4) {
                call_compression(rup, cd->codeword_queue);
                rup->size = 0;
        }
}

void call_compression(Rgb_unsigned_pix rup, Seq_T codeword_queue)
{
        Rgb_float_pix rfp;
        Ypp_data ypp;
        Quant_data qd;
        uint32_t cw;

        rfp = convert_to_floats(rup);
        ypp = rgbtoypp(rfp);
        qd  = ypptoquant(ypp);
        cw  = pack_codeword(qd);
        

        uint32_t *cwp = malloc(sizeof(uint32_t));
        *cwp = cw;
        Seq_addhi(codeword_queue, cwp);
}


void write_codewords(Seq_T codeword_queue, int newWidth, int newHeight) 
{
        unsigned width  = (unsigned) newWidth;
        unsigned height = (unsigned) newHeight;

        printf("COMP40 Compressed image format 2\n%u %u", width, height);
        putchar('\n');

        int i;

        int seq_length = Seq_length(codeword_queue);
        for(i = 0; i < seq_length; i++) {
                uint32_t *cwp = (uint32_t*) Seq_remlo(codeword_queue);           
                char c;

                /* 32 bit == 4 bytes */
                for (int j = 24; j >= 0; j -= 8) {
                        c = Bitpack_getu(*cwp, 8, j);
                        putchar(c);
                }
                //fprintf(stderr, "%u\n", *cwp);

                free(cwp);
        }
}

/* DECOMPRESSION FUNCTIONS */
void ppmio_decompress(FILE *inputfp)
{
        Pnm_ppm pnm = readcodewords(inputfp);
        Pnm_ppmwrite(stdout, pnm);
        Pnm_ppmfree(&pnm);
}

Pnm_ppm readcodewords(FILE *inputfp) {
        unsigned height, width;
        int read = fscanf(inputfp, "COMP40 Compressed image format 2\n%u %u", 
                          &width, &height);
        assert(read == 2);
        int c = getc(inputfp);
        assert(c == '\n');

        //fprintf(stderr, "Width: %u, Height: %u\n", width, height);

        Seq_T rgb_seq = Seq_new((width * height) / 4);

        unsigned i;
        int j;
        for(i = 0; i < (width * height) / 4; i++) {
                uint64_t codeword = 0;
                int c; 
                for(j = 24; j >= 0; j -= 8) {
                        c = getc(inputfp);
                        assert(c != EOF);
                        //fprintf(stderr, "%lu\n", c);
                        codeword = Bitpack_newu(codeword, 8, j, (uint64_t) c);
                }
                call_decompression((uint32_t) codeword, rgb_seq);
        }

        decomp_pix dp = malloc(sizeof(struct decomp_pix));
        dp->rgb_seq  = rgb_seq;
        dp->temp_rup = Seq_remlo(rgb_seq);
        dp->temp_rup->size = 0;

        Pnm_ppm pnm = malloc(sizeof(struct Pnm_ppm));
        pnm->width  = width;
        pnm->height = height;
        pnm->denominator = 255; /* Why? */
        pnm->methods = uarray2_methods_blocked;
        pnm->pixels = create_uray2_blocked(width, height);
        pnm->methods->map_block_major(pnm->pixels, put_four_pixels, dp);
        //pnm->methods->map_block_major(pnm->pixels, print, NULL);

        free(dp->temp_rup);
        Seq_free(&rgb_seq);
        free(dp);

        return pnm;
}

void call_decompression(uint32_t codeword, Seq_T rgb_seq)
{
        Quant_data qd;
        Ypp_data ypp;
        Rgb_float_pix rfp;
        Rgb_unsigned_pix rup;

        //fprintf(stderr, "%x\n", codeword);
        
        qd  = unpack_codeword(codeword);

        //fprintf(stderr, "index_pr: %u, index_pb: %u\n", qd->index_pr, 
                        //qd->index_pb);

        ypp = quanttoypp(qd);
/*
        for(int j = 0; j < 4; j++) {
            fprintf(stderr, "y[%d]: %f\n", j, ypp->y[j]);
        }
*/
        rfp = ypptorgb(ypp);
        rup = convert_to_uints(rfp);


        
/*
        fprintf(stderr, "a: %f, b: %f, c: %f, d: %f\n", qd->a, qd->b, qd->c, qd->d);
*/

        for (int i = 0; i < 4; ++i)
        {
                //fprintf(stderr, "Red: %f, Green: %f, Blue: %f\n", rfp->red[i],
                //rfp->green[i], rfp->blue[i]);
            //fprintf(stderr, "red: %u\n", rup->red[i]);
        }


        Seq_addhi(rgb_seq, rup);
}


A2Methods_UArray2 create_uray2_blocked(unsigned width, unsigned height)
{
        A2Methods_T methods = uarray2_methods_blocked;

        int iheight = (int) height;
        int iwidth  = (int) width;
        int isize   = (int) sizeof(struct Pnm_rgb);

        return methods->new_with_blocksize(iwidth, iheight, isize, 2);
}

void print(int col, int row, A2Methods_UArray2 pixels, void *x,
                     void *cl)
{
    (void) col;
    (void) row;
    (void) pixels;
    (void) cl;

    Pnm_rgb rgb = (Pnm_rgb) x;

    fprintf(stderr, "red: %d green: %d blue: %d\n", rgb->red, rgb->green, rgb->blue);
}

void put_four_pixels(int col, int row, A2Methods_UArray2 pixels, void *x,
                     void *cl)
{
        assert(x != NULL && cl != NULL);

        (void) col;
        (void) row;
        (void) pixels;

        decomp_pix dp = (decomp_pix) cl;

        if((dp->temp_rup->size) == 4) {
                free(dp->temp_rup);
                dp->temp_rup = Seq_remlo(dp->rgb_seq);
                dp->temp_rup->size = 0;
                //fprintf(stderr, "blue: %u\n", dp->temp_rup->blue[3]);
        }

        Pnm_rgb rgb = (Pnm_rgb) x;
        int size = dp->temp_rup->size;

        //fprintf(stderr, "size: %d\n", size);


        rgb->red   = dp->temp_rup->red[size];
        rgb->green = dp->temp_rup->green[size];
        rgb->blue  = dp->temp_rup->blue[size];
        //fprintf(stderr, "blue: %d\n", dp->temp_rup->blue[size - 1]);
        dp->temp_rup->size++;
}