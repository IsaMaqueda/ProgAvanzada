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

#include "pgm_image.h"

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
void GameThreads(pgm_t * pmg_image1, pgm_t * pgm_image2 , int threads);
void  * useThreads(void * arg);
//OpenMp functions
void GameOMP(pgm_t * pgm_image1, pgm_t * pgm_image2);




int main(int argc, char * argv[])
{
    int iterations = atoi(argv[1]);

    char * filename_in = argv[2];
    int num_threads;
    pgm_t * initial_image = malloc(sizeof(pgm_t));

    //reads initial image
    readPGMFile(filename_in, initial_image);
    
    //creates the copy to manipulate

    pgm_t * pgm_image = malloc(sizeof(pgm_t));
    strcpy(pgm_image->magic_number, initial_image->magic_number);
    pgm_image->image.height = initial_image->image.height;
    pgm_image->image.width = initial_image->image.width;
    allocateImage(&pgm_image->image);
    copyPGM(initial_image, pgm_image);

    for(int i = 0; i < iterations; i++)
    {
        //printf("Hello \n");
        //uses threads
            if (argc == 4)
            {
                printf("Estoy usando threads \n ");
                num_threads = atoi(argv[3]);
                GameThreads(initial_image,pgm_image,num_threads);
                //printf("Acabo de usar threads 1 \n");
            }
            //uses OpenMP
            else
            {
                printf("Estoy usando OpenMP \n");
                GameOMP(initial_image,pgm_image);
            }

            printf("Acabo de usar threads  3\n");
            
            //write image 
            char filename[20];
            sprintf(filename, "iteration-%05d.pgm", i);
            writePGMFile(filename , pgm_image);

            //swap pointers
            changePointers(initial_image, pgm_image);

    }
    
}

//swap the images used 
void changePointers(pgm_t * pmg_image1, pgm_t * pmg_image2)
{
    pgm_t temp = *pmg_image1;

    *pmg_image1 = *pmg_image2;
    *pmg_image2 = temp;

    return;
}


//function to create threads to the game of life
void GameThreads(pgm_t * pgm_image1, pgm_t * pgm_image2 , int threads)
{

    //printf("threads1 \n");
    //creates the treads
    pthread_t tids[threads];

    int range; 
    int status;
    //creates the struct to store
    useth_t thread_limits[threads];

    //Initializes the lock
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    //set the number of lines each thread will do
    range  = pgm_image1->image.width / threads; 

    //printf("range %d, \n", range);

    //creates the threads
    for(int i = 0; i < threads ; i++)
    {
        //printf("threads %d \n", i);

        thread_limits[i].lock = &lock;
        thread_limits[i].start = i * range;
        thread_limits[i].stop = i * range + range; 
        thread_limits[i].pgm_image1 = pgm_image1;
        thread_limits[i].pgm_image2 = pgm_image2;

        status = pthread_create(&tids[i], 0, useThreads, &thread_limits[i]);

        if(status < 0)
        {
            perror("ERROR: pthread_create");
            exit(EXIT_FAILURE);
        }


    }
    //jions the threads
    for(int i = 0; i< threads;i++)
    {
        pthread_join(tids[i], NULL);
    }

    /*for (int i = 0; i< thread_limits->pgm_image2->image.width; i++)
    {
        for(int j = 0; j< thread_limits->pgm_image2->image.height; j++)
        {
            printf("The value of %d, %d of pgm_image 2 is: %d \n", i, j, thread_limits->pgm_image2->image.pixels[i][j].value);
        }
    }*/

    pgm_image2 = thread_limits->pgm_image2;
    pgm_image1 = thread_limits->pgm_image1;

    //checks that pgm_image2 is copied correctly
    /*for (int i = 0; i< pgm_image2->image.width; i++)
    {
        for(int j = 0; j< pgm_image2->image.height; j++)
        {
            printf("The value of %d, %d of pgm_image 2 is: %d \n", i, j, pgm_image2->image.pixels[i][j].value);
        }
    }*/
    //printf("Acabo de usar threads -1 \n");
    //printf("Acabo de usar threads 0 \n");

}
//play the game, changes the value of the second image
void  * useThreads(void * arg)
{

    useth_t * thread_limits = (useth_t * ) arg;
    //printf("Limits %d, %d \n ",thread_limits->start,thread_limits->stop);
    for(int i = thread_limits->start; i <thread_limits->stop;i++)
    {
        for(int j = 0; j < thread_limits->pgm_image1->image.height; j++)
        {
            pthread_mutex_lock(thread_limits->lock);
            //analize cell returns the value according to the rules
            thread_limits->pgm_image2->image.pixels[i][j].value = analize_cell(thread_limits->pgm_image1, i, j);
            pthread_mutex_unlock(thread_limits->lock);
        }

    }
    pthread_exit(NULL);
}

//checks the number of the cell
int mod(int n, int limit)
{
    //if the cell outisde of range in the right or the top, it returns the last position posible
    if(n < 0)
    {
        return limit -1;
    }
    //if the cell outisde of range in the left or the bottom, it returns the first position posible
    else if (n >= limit)
    {
        return 0;
    }
    else 
    {
        return n;
    }


}
//analize cell returns the value according to the rules
int analize_cell(pgm_t * pgm_image, int i, int j)
{
    int alive = 0, dead = 0;
    int limitH= pgm_image->image.width;
    int limitV = pgm_image->image.height;


    //check if neighbour is alive or dead
    //mod rounds the neighbour
    
    (pgm_image->image.pixels[mod(i-1,limitH)][mod(j-1,limitV)].value ? alive++ : dead++);

    (pgm_image->image.pixels[mod(i-1,limitH)][mod(j,limitV)].value ? alive++ : dead++);

    (pgm_image->image.pixels[mod(i-1,limitH)][mod(j+1,limitV)].value ? alive++ : dead++);

    (pgm_image->image.pixels[mod(i,limitH)][mod(j-1,limitV)].value ? alive++ : dead++);
    
    (pgm_image->image.pixels[mod(i,limitH)][mod(j+1,limitV)].value ? alive++ : dead++);

    (pgm_image->image.pixels[mod(i+1,limitH)][mod(j-1,limitV)].value ? alive++ : dead++);

    (pgm_image->image.pixels[mod(i+1,limitH)][mod(j,limitV)].value ? alive++ : dead++);

    (pgm_image->image.pixels[mod(i+1,limitH)][mod(j+1,limitV)].value ? alive++ : dead++);

    //Any live cell with fewer than two live neighbours dies, as if caused by underpopulation.
    if (alive < 2)
    {
        return 0;
    }
    //Any live cell with more than three live neighbours dies, as if by overpopulation.
    else if(alive > 3)
    {
        return 0;
    }
    //Any live cell with two or three live neighbours lives on to the next generation
    else if (pgm_image->image.pixels[i][j].value == 1 && (alive==2 || alive == 3))
    {
        return 1;
    }
    //Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction
    else if (pgm_image->image.pixels[i][j].value == 0 && alive == 3)
    {
        return 1;
    }
}
//Plays the game of life using OpenMp 
void GameOMP(pgm_t * pgm_image1, pgm_t * pgm_image2)
{

    int i = 0; int j = 0; 
    //starts the OpenMp parallelization
    #pragma omp parallel default(none) shared(pgm_image1, pgm_image2) private(i,j)
    {
        #pragma omp for
        for(i =0; i < pgm_image1->image.width ; i++)
        {
            for(j = 0; j< pgm_image1->image.height; j++)
            {
                //analize cell returns the value according to the rules
                pgm_image2->image.pixels[i][j].value = analize_cell(pgm_image1, i, j);
            }

        }
    }
    

}


// Generate space to store the image in memory
void allocateImage(image_t * image)
{
    int i;

    printf("\nGetting memory for the pixels\n");
    printf("Size: %d x %d = %d\n", image->width, image->height, image->width * image->height);

    // Allocate memory for the pointers to the rows
    image->pixels = (pixel_t **) malloc(image->height * sizeof (pixel_t *));
    // Validate that the memory was assigned
    if (image->pixels == NULL)
    {
        printf("Error: Could not allocate memory for the matrix rows!\n");
        exit(EXIT_FAILURE);
    }

	// Allocate the memory for the whole image in a single block.
    // This will make the memory contiguous.
    image->pixels[0] = (pixel_t *) malloc(image->width * image->height * sizeof (pixel_t));
    // Validate that the memory was assigned
    if (image->pixels[0] == NULL)
    {
        printf("Error: Could not allocate memory for the matrix!\n");
        exit(EXIT_FAILURE);
    }

    // Assign the pointers to the rows
    for (i=0; i<image->height; i++)
    {   
        image->pixels[i] = image->pixels[0] + image->width * i;
    }

    printf("Done!\n");
}

// Release the dynamic memory used by an image
void freeImage(image_t * image)
{
    printf("\nReleasing the memory for the pixels\n");

    // Free the memory where the data is stored
    free( image->pixels[0] );
    // Free the array for the row pointers
    free(image->pixels);
    // Set safe values for the variables
    image->pixels = NULL;
	image->width = 0;
	image->height = 0;
    printf("Done!\n");
}

// Copy an image to another structure
void copyPGM(const pgm_t * source, pgm_t * destination)
{
	if (source->image.width == destination->image.width && source->image.height == destination->image.height)
    {
        memcpy( *(destination->image.pixels), *(source->image.pixels), source->image.width * source->image.height);
	}
    else
    {
        printf("Error: The matrices to be copied are of different sizes!\n");
    }
}

// Read the contents of a text file and store them in an image structure
void readPGMFile(const char * filename, pgm_t * pgm_image)
{
    FILE * file_ptr = NULL;

    printf("\nReading file: '%s'\n", filename);
    // Open the file
    file_ptr = fopen(filename, "r");
    if (file_ptr == NULL)
    {
        printf("Error: Unable to open file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    // Read the header
    readPGMHeader(pgm_image, file_ptr);

    // Allocate the memory for the array of pixels
    allocateImage( &(pgm_image->image) );

    // Read the data acording to the type
    if ( !strncmp(pgm_image->magic_number, "P2", 3) )
    {
        readPGMTextData(pgm_image, file_ptr);
    }
    else if ( !strncmp(pgm_image->magic_number, "P5", 3) )
    {
        readPGMBinaryData(pgm_image, file_ptr);
    }
    else
    {
        printf("Invalid file format. Unknown type: %s\n", pgm_image->magic_number);
        exit(EXIT_FAILURE);
    }

    // Close the file
    fclose(file_ptr);
    printf("Done!\n");
}

// Read the first lines of the file and store them in the correct fields in the structure
void readPGMHeader(pgm_t * pgm_image, FILE * file_ptr)
{
    char line[LINE_SIZE];

    // Get the type of PGM file
    inputString(line, LINE_SIZE, file_ptr);
    sscanf(line, "%s", pgm_image->magic_number);

    // Read the width and height of the image
    inputString(line, LINE_SIZE, file_ptr);
    // Ignore the line if it has a comment
    if (line[0] == '#')
    {
        inputString(line, LINE_SIZE, file_ptr);
    }
    sscanf(line, "%d %d", &pgm_image->image.width, &pgm_image->image.height);

    // Read the maximum value used in the image format
    inputString(line, LINE_SIZE, file_ptr);
    sscanf(line, "%d", &pgm_image->max_value);
}

// Read data in plain text format (PGM P2)
void readPGMTextData(pgm_t * pgm_image, FILE * file_ptr)
{
    // Read the data for the pixels
    for (int i=0; i<pgm_image->image.height; i++)
    {
        for (int j=0; j<pgm_image->image.width; j++)
        {
            // Read the value for the pixel
            fscanf(file_ptr, "%hhu", &(pgm_image->image.pixels[i][j].value));
        }
    }
}

// Read data in binary format (PGM P5)
void readPGMBinaryData(pgm_t * pgm_image, FILE * file_ptr)
{
    // Count the number of pixels
    size_t pixels = pgm_image->image.height * pgm_image->image.width;

    // Read all the data in one go into the contiguous array
    if ( ! fread(pgm_image->image.pixels[0], sizeof (unsigned char), pixels, file_ptr) )
    {
        perror("Unable to read data from the PGM file\n");
        exit(EXIT_FAILURE);
    }
}

// Write the data in the image structure into a new PGM file
// Receive a pointer to the image, to avoid having to copy the data
void writePGMFile(const char * filename, const pgm_t * pgm_image)
{
    FILE * file_ptr = NULL;

    //printf("\nWriting file: '%s'\n", filename);
    // Open the file
    file_ptr = fopen(filename, "w");
    if (file_ptr == NULL)
    {
        printf("Error: Unable to open file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    // Write the header for the file
    fprintf(file_ptr, "%s\n", pgm_image->magic_number);
    fprintf(file_ptr, "# %s\n", filename);
    fprintf(file_ptr, "%d %d\n", pgm_image->image.width, pgm_image->image.height);
    fprintf(file_ptr, "%d\n", pgm_image->max_value);
    // Write the data acording to the type
    if ( !strncmp(pgm_image->magic_number, "P2", 3) )
    {
        writePGMTextData(pgm_image, file_ptr);
    }
    else if ( !strncmp(pgm_image->magic_number, "P5", 3) )
    {
        writePGMBinaryData(pgm_image, file_ptr);
    }
    else
    {
        printf("Invalid file format. Unknown type: %s\n", pgm_image->magic_number);
        exit(EXIT_FAILURE);
    }
    fclose(file_ptr);
    //printf("Done!\n");
}

// Write the data in the image structure into a new PGM file
// Receive a pointer to the image, to avoid having to copy the data
void writePGMTextData(const pgm_t * pgm_image, FILE * file_ptr)
{
    // Write the pixel data
    for (int i=0; i<pgm_image->image.height; i++)
    {
        for (int j=0; j<pgm_image->image.width; j++)
        {
            fprintf(file_ptr, "%d", pgm_image->image.pixels[i][j].value);
            // Separate pixels in the same row with tabs
            if (j < pgm_image->image.width-1)
                fprintf(file_ptr, "\t");
            else
                fprintf(file_ptr, "\n");
        }
    }
}

// Write the data in the image structure into a new PGM file
// Receive a pointer to the image, to avoid having to copy the data
void writePGMBinaryData(const pgm_t * pgm_image, FILE * file_ptr)
{
    // Write the pixel data
    size_t pixels = pgm_image->image.height * pgm_image->image.width;

    // Write the binary data from the array to the file
    fwrite(pgm_image->image.pixels[0], sizeof (unsigned char), pixels, file_ptr);
}

// Invert the colors in the image
// Since a PGM format will only use integers for the encoding,
//  the negative is obtained by substracting each color component
//  from the maximum value defined in the PGM header
void negativePGM(pgm_t * pgm_image)
{
    printf("\nInverting image\n");
    for (int i=0; i<pgm_image->image.height; i++)
    {
        for (int j=0; j<pgm_image->image.width; j++)
        {
            // Substract the current value from the maximum value
            pgm_image->image.pixels[i][j].value = pgm_image->max_value - pgm_image->image.pixels[i][j].value;
        }
    }
    printf("Done!\n");
}

// Apply a blur algorithm to the image
void blurPGM(pgm_t * pgm_image, int radius)
{
    printf("\nBlurring an image with radius %d\n", radius);
    unsigned int total_value;
    int i;
    int j;
    int rx;
    int ry;
    int x;
    int y;
    int counter;

    // Create a temporary image
    pgm_t image_copy;

    // Initialize the new image
    image_copy.image.height = pgm_image->image.height;
    image_copy.image.width = pgm_image->image.width;
    allocateImage(&image_copy.image);

        // Loops for the pixels in the image
        for (i=0; i<pgm_image->image.height; i++)
        {
            for (j=0; j<pgm_image->image.width; j++)
            {
                // Initialize the final colors
                total_value = 0;
                // Initialize the average counter
                counter = 0;
                // Inner loops for the averaging window
                for (ry=-radius; ry<=radius; ry++)
                {
                    y = i + ry;
                    // Do not use pixels outside the image range
                    if (y < 0)
                    { continue; }
                    if (y > (pgm_image->image.height - 1))
                    { break; }
                    // Loop for the columns, using a circular radius
                    for (rx=-radius+abs(ry); rx<=radius-abs(ry); rx++)
                    {
                        x = j + rx;
                        // Do not use pixels outside the image range
                        if (x < 0)
                        { continue; }
                        if (x > (pgm_image->image.width - 1))
                        { break; }
                        total_value += pgm_image->image.pixels[y][x].value;
                        counter++;
                    }
                }

                // Average the colors in the copy
                image_copy.image.pixels[i][j].value = total_value / counter;
            }
        }
    printf("Done!\n");

    // Copy the new image back to the initial structure
    copyPGM(&image_copy, pgm_image);

    // Release the memory in the temporary image
    freeImage(&image_copy.image);
}

// Convert an image into ASCII art
// Color scales from:
//  http://paulbourke.net/dataformats/asciiart/
// Converts two pixels vertically to a single character
void asciiArtPGM(pgm_t * pgm_image, const char * out_filename)
{
    FILE * ascii_file = NULL;
    int total;
    // Array with the characters to use for 10 levels of grey
    char grey_levels[] = " .:-=+*#%@";
    int max_levels = strlen(grey_levels);
    // Get the range of each level of grey
    int grey_increment = pgm_image->max_value / max_levels;
    int level;
    int grey;

    // Open the text file
    ascii_file = fopen(out_filename, "w");
    if (!ascii_file)
    {
        fprintf(stderr, "ERROR: fopen file '%s'\n", out_filename);
        exit(EXIT_FAILURE);
    }

    printf("\nConverting to ASCII image\n");

    // Move two pixels down each time
    for (int i=0; i<pgm_image->image.height; i+=2)
    {
        for (int j=0; j<pgm_image->image.width; j++)
        {
            // Add the values for the pixel and the one below
            total = pgm_image->image.pixels[i][j].value + pgm_image->image.pixels[i+1][j].value;
            // Average them
            total /= 2;

            // Get the scale of grey in 10 levels
            level = 0;
            grey = pgm_image->max_value;
            while (grey > total)
            {
                grey -= grey_increment;
                level++;
            }
            // Avoid going over the maximum level
            if (level > max_levels - 1)
            {
                //printf("SOME OVERFLOW HERE! [%d, %d]\n", i, j);
                level = max_levels - 1;
            }
            // Print the correct character
            fprintf(ascii_file, "%c", grey_levels[level]);
        }
        fprintf(ascii_file, "\n");
    }
    printf("Writing file: '%s'\n", out_filename);
    printf("Done!\n");

    fclose(ascii_file);
}
