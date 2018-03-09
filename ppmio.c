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

/* Function ppmio_compress takes in a file pointer and has void return type.
 * Reads the given file into a Pnm and then into a blocked UArray. Iterates
 * through the UArray and compresses each block and writes the code words
 * to disk using helper functions.
 */

void ppmio_compress(FILE *inputfp)
{
        A2Methods_T plain_methods = uarray2_methods_plain;
        A2Methods_T blocked_methods = uarray2_methods_blocked;

        Pnm_ppm pnm = ppmread(inputfp, plain_methods);

        A2Methods_UArray2 uray2b = make_uray2_blocked(pnm, blocked_methods);

        Rgb_unsigned_pix rup = malloc(sizeof(struct Rgb_unsigned_pix));
        assert(rup != NULL);

        rup->size = 0;
        rup->denominator = pnm->denominator;

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

/* Function ppmread takes in a file pointer and an A2Methods_T and returns
 * a Pnm_ppm. Creates a Pnm from the passed in file pointer.
 */

Pnm_ppm ppmread(FILE* inputfp, A2Methods_T methods)
{
        assert(inputfp != NULL && methods == uarray2_methods_plain);
        return Pnm_ppmread(inputfp, methods);
}

/* Function make_uray2_blocked takes in a Pnm and an A2Methods_T and returns
 * an A2Methods_UArray2. Iterates through the given Pnm UArray and creates
 * a blocked UArray.
 */

A2Methods_UArray2 make_uray2_blocked(Pnm_ppm pnm, A2Methods_T blocked_methods) 
{
        assert(pnm != NULL && blocked_methods == uarray2_methods_blocked);
        A2Methods_UArray2 uray2b;
        int newWidth, newHeight;

        if ((pnm->height % 2) != 0) 
                newHeight = --(pnm->height);
        else
                newHeight = pnm->height;
        if ((pnm->width % 2) != 0) 
                newWidth = --(pnm->width);
        else 
                newWidth = pnm->width;
        
        uray2b = blocked_methods->new_with_blocksize(newWidth, newHeight, 
                                                    sizeof(struct Pnm_rgb), 2);

        copy_data cp = malloc(sizeof(struct copy_data));
        cp->uray2b = uray2b;
        cp->methods = blocked_methods;

        pnm->methods->map_row_major(pnm->pixels, copy_uarray2, cp);
        free(cp);

        return uray2b;
}

/* Function copy_uarray2 takes in 2 ints, an A2Methods_Uarray2 and two void
 * pointers and has void return type. An apply function that iterates through a
 * given UArray and copies the data into another UArray.
 */

void copy_uarray2(int col, int row, A2Methods_UArray2 pixels, 
                  void *x, void *cl)
{
        assert(cl != NULL && x != NULL);

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

/* Function compress_four_pixels takes in 2 ints, an A2Methods_UArray2, and 
 * two void pointers and has void return type. An apply function that iterates
 * through the given UArray and compresses the pixels and adds them to a 
 * sequence.
 */

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

        rup->red[size]   = rgb->red;
        rup->green[size] = rgb->green;
        rup->blue[size]  = rgb->blue;
        rup->size++;

        if (rup->size == 4) {
                call_compression(rup, cd->codeword_queue);
                rup->size = 0;
        }
}

/* Function call_compression takes in an Rgb_unsigned_pix struct and a Seq_T 
 * and has void return type. Calls all of the helper function on the given 
 * structs and adds the final codeword to the sequence.
 */

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

/* Function write_codewords takes in a Seq_T and two ints and has void return
 * type. Writes all of the codewords in the sequence to standart output.
 */

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

                for (int j = 24; j >= 0; j -= 8) {
                        c = Bitpack_getu(*cwp, 8, j);
                        putchar(c);
                }

                free(cwp);
        }
}

/* DECOMPRESSION FUNCTIONS */

/* Function ppmio_decompress takes in a File pointer and has void return 
 * type. Calls the decompression functions and writes the given Pnm to a 
 * file.
 */

void ppmio_decompress(FILE *inputfp)
{
        Pnm_ppm pnm = readcodewords(inputfp);
        Pnm_ppmwrite(stdout, pnm);
        Pnm_ppmfree(&pnm);
}

/* Function readcodewords takes in a File pointer and returns a Pnm. Reads all
 * of the codewords from the file to a sequence and decompresses all of the
 * codewords and adds the new rgb structs to a Pnm.
 */

Pnm_ppm readcodewords(FILE *inputfp)
{
        unsigned height, width;
        int read = fscanf(inputfp, "COMP40 Compressed image format 2\n%u %u", 
                          &width, &height);
        assert(read == 2);
        int c = getc(inputfp);
        assert(c == '\n');

        Seq_T rgb_seq = Seq_new((width * height) / 4);

        unsigned i;
        int j;
        for(i = 0; i < (width * height) / 4; i++) {
                uint64_t codeword = 0;
                int c; 
                for(j = 24; j >= 0; j -= 8) {
                        c = getc(inputfp);
                        assert(c != EOF);
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
        pnm->denominator = 255; 
        pnm->methods = uarray2_methods_blocked;
        pnm->pixels = create_uray2_blocked(width, height);
        pnm->methods->map_block_major(pnm->pixels, put_four_pixels, dp);

        free(dp->temp_rup);
        Seq_free(&rgb_seq);
        free(dp);

        return pnm;
}

/* Function call_decompression takes in a uint32_t and a Seq_T and has void
 * return type. Calls all of the helper functions to decompress the given
 * codeword and adds the result to a sequence.
 */

void call_decompression(uint32_t codeword, Seq_T rgb_seq)
{
        Quant_data qd;
        Ypp_data ypp;
        Rgb_float_pix rfp;
        Rgb_unsigned_pix rup;

        qd  = unpack_codeword(codeword);
        ypp = quanttoypp(qd);
        rfp = ypptorgb(ypp);
        rup = convert_to_uints(rfp);
        Seq_addhi(rgb_seq, rup);
}

/* Function create_uray2_blocked takes in 2 unsigned and returns an
 * A2Methods_UArray2. Creates a new blocked UArray and returns it.
 */

A2Methods_UArray2 create_uray2_blocked(unsigned width, unsigned height)
{
        A2Methods_T methods = uarray2_methods_blocked;

        int iheight = (int) height;
        int iwidth  = (int) width;
        int isize   = (int) sizeof(struct Pnm_rgb);

        return methods->new_with_blocksize(iwidth, iheight, isize, 2);
}

/* Function put_four_pixels takes 2 ints, an A2Methods_UArray2 and two void
 * pointers and has void return type. Iterates through the given UArray2
 * and adds the rgb values from the sequence to the Pnm structs.
 */

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
        }

        Pnm_rgb rgb = (Pnm_rgb) x;
        int size = dp->temp_rup->size;

        rgb->red   = dp->temp_rup->red[size];
        rgb->green = dp->temp_rup->green[size];
        rgb->blue  = dp->temp_rup->blue[size];
        dp->temp_rup->size++;
}