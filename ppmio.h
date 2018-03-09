#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "seq.h"

#ifndef PPMIO_H
#define PPMIO_H

/* Struct Rgb_unsigned_pix contains 3 unsigned arrays of rgb values, the size
 * of the arrays when they are being emptied, and the denominator of the 
 * image. Used for passing the information of 4 pixels.
 */

typedef struct Rgb_unsigned_pix {
        unsigned red[4];   /* 4 element array of red */
        unsigned green[4]; /* 4 element array of green */
        unsigned blue[4];  /* 4 element array of blue */
        int size;
        unsigned denominator;
} *Rgb_unsigned_pix;

/* Struct compress_data contains a sequence and an Rgb_unsigned_pix struct.
 * Used to write all of the codewords to standard output after being 
 * compressed.
 */

typedef struct compress_data {
        Seq_T codeword_queue;
        Rgb_unsigned_pix rup;
} *compress_data;

/* Struct copy_data contains an A2Methods_UArray2 and an A2Methods_T. Used to
 * copy the data from one UArray2 to another.
 */

typedef struct copy_data {
        A2Methods_UArray2 uray2b;
        A2Methods_T methods;   
} *copy_data;

/* Struct decomp_pix contains a Seq_T and an Rgb_unsigned_pix struct. Contains
 * the same information as the compress_data struct but has a different name 
 * to differentiate between compression and decompression.
 */

typedef struct decomp_pix {
                Seq_T rgb_seq;
                Rgb_unsigned_pix temp_rup;
} *decomp_pix;


void ppmio_compress(FILE *inputfp);

Pnm_ppm ppmread(FILE *inputfp, A2Methods_T methods);

A2Methods_UArray2 make_uray2_blocked(Pnm_ppm pnm, A2Methods_T blocked_methods);

void copy_uarray2(int col, int row, A2Methods_UArray2 pixels, void *x, 
                  void *cl);

void compress_four_pixels(int col, int row, A2Methods_UArray2,
                          void *x, void *cl);

void call_compression(Rgb_unsigned_pix rup, Seq_T codeword_queue);

void write_codewords(Seq_T codeword_queue, int newWidth, int newHeight);

void call_decompression(uint32_t codeword, Seq_T rgb_seq);

void ppmio_decompress(FILE *inputfp);

Pnm_ppm readcodewords(FILE *inputfp); 

A2Methods_UArray2 create_uray2_blocked(unsigned width, unsigned height);

void put_four_pixels(int col, int row, A2Methods_UArray2 pixels, void *x,
                     void *cl);

#endif