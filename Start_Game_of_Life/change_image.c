/*
    Simple program to use the pgm library

    Gilberto Echeverria
    17/11/2019
*/

#include <stdio.h>
#include <stdlib.h>
#include "pgm_image.h"

int main()
{
    char * in_filename = "sample_1.pgm";
    char * out_filename = "sample_2.pgm";
    pgm_t image;

    readPGMFile(in_filename, &image);
    // Convert the ASCII format into Binary, to generate smaller images
    // Change the P2 format to P5
    image.magic_number[1] = '5';
    negativePGM(&image);
    writePGMFile(out_filename, &image);

    return 0;
}
