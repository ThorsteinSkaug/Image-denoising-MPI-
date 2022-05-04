#include "serial_header.h"

// Defining the functions from simple-jpeg library
void import_JPEG_file (const char* filename, unsigned char** image_chars,
  int* image_height, int* image_width,
  int* num_components);

void export_JPEG_file (const char* filename, const unsigned char* image_chars,
  int image_height, int image_width,
  int num_components, int quality);


int main(int argc, char *argv[])
{

// Defining variables, including variables defined from input
// This input can be altered in the Makefile
 char input_jpeg_filename[32], output_jpeg_filename[32];
 float kappa = atof(argv[1]);
 int iters = atoi(argv[2]);
 strncpy(input_jpeg_filename, argv[3], 32);
 strncpy(output_jpeg_filename, argv[4], 32);
 int m, n, c;
 image u, u_bar;
 unsigned char *image_chars;

 // These functions kinda speak for themselves on what is happening.
 import_JPEG_file(input_jpeg_filename, &image_chars, &m, &n, &c);
 allocate_image (&u, m, n);
 allocate_image (&u_bar, m, n);
 convert_jpeg_to_image (image_chars, &u);
 iso_diffusion_denoising (&u, &u_bar, kappa, iters);
 convert_image_to_jpeg (&u_bar, image_chars);
 export_JPEG_file(output_jpeg_filename, image_chars, m, n, c, 75);
 deallocate_image (&u);
 deallocate_image (&u_bar);
 return 0;
}
