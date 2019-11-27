/*
   Definition of an image data structure for PGM format images
   Uses typedef, struct
   Uses functions to read and write a file in PGM format, described here:
    http://netpbm.sourceforge.net/doc/pgm.html
    http://rosettacode.org/wiki/Bitmap/Write_a_PGM_file#C

    Isabel Maqueda Rolon 
    A01652906
    27/11/2019
*/

#ifndef PGM_IMAGE_H
#define PGM_IMAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "string_functions.h"
#include <pthread.h>

// Constant for the size of strings to be read from a PGM file header
#define LINE_SIZE 255

//// TYPE DECLARATIONS

// Structure for a pixel color information, using a single grayscale value in the range 0-255
typedef struct pixel_struct
{
    unsigned char value;
} pixel_t;

// Structure to store full image data of any size
typedef struct image_struct
{
    int width;
    int height;
    pixel_t ** pixels;
} image_t;

// Structure for an image in PGM format
typedef struct pgm_struct
{
    char magic_number[3];           // String for the code indicating the type of PGM file
    int max_value;                  // Maximum value for pixel data in each component
    image_t image;
} pgm_t;

//structure to store and use threads
typedef struct use_threads
{
    pgm_t * pgm_image1;
    pgm_t * pgm_image2;
    pthread_mutex_t * lock;
    unsigned int start;
    unsigned int stop;

}useth_t;

//// FUNCTION DECLARATIONS
void allocateImage(image_t * image);
void freeImage(image_t * image);
void copyPGM(const pgm_t * source, pgm_t * destination);
void readPGMFile(const char * filename, pgm_t * pgm_image);
void readPGMHeader(pgm_t * pgm_image, FILE * file_ptr);
void readPGMTextData(pgm_t * pgm_image, FILE * file_ptr);
void readPGMBinaryData(pgm_t * pgm_image, FILE * file_ptr);
void writePGMFile(const char * filename, const pgm_t * pgm_image);
void writePGMTextData(const pgm_t * pgm_image, FILE * file_ptr);
void writePGMBinaryData(const pgm_t * pgm_image, FILE * file_ptr);
void negativePGM(pgm_t * pgm_image);
//void greyscalePGM(pgm_t * pgm_image);
void blurPGM(pgm_t * pgm_image, int radius);
void asciiArtPGM(pgm_t * pgm_image, const char * out_filename);

//
void changePointers(pgm_t * pmg_image1, pgm_t * pmg_image2);
int analize_cell(pgm_t * pgm_image, int i, int j);
int mod(int i, int limit);

//Paralel Functions
void GameThreads(pgm_t * pmg_image1,pgm_t * pgm_image2 , int threads);

void  * useThreads(void * arg);
//OpenMp functions
void GameOMP(pgm_t * pgm_image1, pgm_t * pgm_image2);

#endif
