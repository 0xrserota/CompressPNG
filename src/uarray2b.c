/* 
 * Gavin Smith & Ravi Serota
 * Homework 3
 * uarray2b.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "assert.h"
#include "mem.h"
#include "uarray2b.h"
#include "uarray.h"

#define T UArray2b_T

/* 
 * struct T contains the information needed to use the UArray which
 * contains all of the blocks.
 */

struct T {
            int width;
            int height;
            int elemsize;
        int blocksize;
        int blockswide;
            UArray_T uRay;
};

/*
 * Function UArray2b_new takes in 4 ints and returns a struct T. Creates a 
 * new UArray2b, allows the user to specify the blocksize of the blocked
 * UArray2.
 */

extern T UArray2b_new (int width, int height, int size, int blocksize)
{
        int blockswide;
        int numBlocks = (width * height) / (blocksize * blocksize);
        T uray2b = (T)malloc(sizeof(*uray2b));
        assert(uray2b != NULL);

        if (width % blocksize == 0) {
                blockswide = width / blocksize; 

        } else {
                blockswide = (width / blocksize) + 1;
        }

        uray2b->width      = width;
        uray2b->height     = height;
        uray2b->elemsize   = size;
        uray2b->blocksize  = blocksize;
        uray2b->blockswide = blockswide;

        uray2b->uRay = UArray_new(numBlocks, size * blocksize * blocksize);
        for(int i = 0; i < numBlocks; i++) {
                *((UArray_T*) UArray_at(uray2b->uRay, i)) = 
                UArray_new(blocksize * blocksize, size);
        }
        return uray2b;
}

/*
 * Function UArray2b_new_64K_block takes in 3 ints and returns a T struct.
 * Creates a new UArray2b but defaults the blocksize to be as large as it can
 * be and still fit within a 64K block.
 */

extern T UArray2b_new_64K_block(int width, int height, int size)
{
        int blockswide;
        int blockstall;
        int blocksize;
        int numBlocks;
        T uray2b = (T)malloc(sizeof(*uray2b));
        assert(uray2b != NULL);

        if(size > 64000) {
                blocksize = 1;
        } else {
                blocksize = sqrt(64000 / size);
        }

        if (width % blocksize == 0) {
                blockswide = width / blocksize; 

        } else {
                blockswide = (width / blocksize) + 1;
        }      

        if (height % blocksize == 0) {
                blockstall = height / blocksize; 

        } else {
                blockstall = (height / blocksize) + 1;
        } 
        
        numBlocks = blockswide * blockstall; 

        uray2b->width = width;
        uray2b->height = height;
        uray2b->elemsize = size;
        uray2b->blocksize = blocksize;
        uray2b->blockswide = blockswide;
        uray2b->uRay = UArray_new(numBlocks, sizeof(void*));

        for(int i = 0; i < numBlocks; i++) {
                *((UArray_T*) UArray_at(uray2b->uRay, i)) = 
                UArray_new(blocksize * blocksize, size);
        }

        return uray2b;      
}

/*
 * Function UArray2b_free takes in a pointer to a T struct and has void
 * return type. Frees the data associated with the given UArray2b.
 */

extern void UArray2b_free (T *array2b)
{
        for(int i = 0; i < UArray_length((*array2b)->uRay); i++) {
                UArray_free(UArray_at((*array2b)->uRay, i));
        }
        UArray_free(&((*array2b)->uRay));
        free(*array2b);
}

/*
 * Function UArray2b_width takes in a T struct and returns an int. Returns
 * the width of the passed in UArray2b.
 */

extern int UArray2b_width (T array2b)
{
        return array2b->width;
}

/*
 * Function UArray2b_height takes in a T struct and returns an int. Returns
 * the height of the passed in UArray2b.
 */

extern int UArray2b_height (T array2b)
{
        return array2b->height;
}

/*
 * Function UArray2b_size takes in a T struct and returns an int. Returns
 * the element size of the passed in UArray2b.
 */

extern int UArray2b_size (T array2b)
{
        return array2b->elemsize;
}

/*
 * Function UArray2b_blocksize takes in a T struct and returns an int. Returns
 * the blocksize of the passed in UArray2b.
 */

extern int UArray2b_blocksize(T array2b)
{
        return array2b->blocksize;
}

/*
 * Function UArray2b_at takes in a T struct and 2 pointers and returns a 
 * void pointer. Returns the value in the index associated with the given
 * row and column.
 */

extern void *UArray2b_at(T array2b, int col, int row)
{       
        int height    = UArray2b_height(array2b);
        int width     = UArray2b_width(array2b);
        int blocksize = UArray2b_blocksize(array2b);
        int blockInd  = (col / blocksize) + ((row / blocksize) * 
                         array2b->blockswide);

        if(row > height - 1 || col > width - 1)
                assert(0);
       
        int cellInd = (col % blocksize) + ((row % blocksize) * blocksize);

        UArray_T *urayp = UArray_at(array2b->uRay, blockInd);
        return UArray_at(*urayp, cellInd);
}

/*
 * Function UArray2b_map takes in a T struct, a function pointer to an apply
 * function and a void pointer and has void return type. Iterates through
 * the given UArray2b and passes the value of each index to the apply function.
 */

extern void UArray2b_map(T array2b, void apply(int col, int row, T array2b,
void *elem, void *cl), void *cl)
{
        void *val;
        int col;
        int row;
        int width      = UArray2b_width(array2b);
        int height     = UArray2b_height(array2b);
        int blocksize  = UArray2b_blocksize(array2b);
        int blockswide = array2b->blockswide;      

        for(int i = 0; i < UArray_length(array2b->uRay); i++) {
                for(int j = 0; j < (blocksize * blocksize); j++) {
                        row = ((i / (blockswide)) * blocksize) +
                               (j / blocksize);
                        if(i >= (blockswide)) {
                            col = ((i % (blockswide)) * blocksize) +
                                   (j % blocksize);
                        } else {
                            col = (i * blocksize) + (j % blocksize);
                        }                       

                        if (row < height && col < width) {
                            UArray_T *urayp = UArray_at(array2b->uRay, i);
                            val = UArray_at(*urayp, j);
                            apply(col, row, array2b, val, cl);
                        }      
                }
        }
}

#undef T