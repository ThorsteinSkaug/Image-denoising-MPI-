#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

// Making a typedef struct to define components of an image-like data type
typedef struct {
    float** image_data;
    int m;
    int n;
} image;


// Defining all the functions which will be used
void pointer_swap(void** a, void** b);
void allocate_image(image *u, int m, int n);
void deallocate_image(image *u);
void convert_jpeg_to_image(const unsigned char *image_chars, image *u);
void convert_image_to_jpeg(unsigned char *image_chars, image *u);
void iso_diffusion_denoising_parallel(image *u, image *u_bar, float kappa, int iters, int my_rank, int num_procs);
