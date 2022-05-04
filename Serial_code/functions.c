#include "serial_header.h"

void allocate_image(image *u, int m, int n){

  // Function for allocating memory for an image
  // The image struct is here used, where the image data 'u' has several components
  // Inputs:
  // image *u: pointer of image struct for storing image data
  // int m: Image height
  // int n: Image width

  u -> n = n;
  u -> m = m;
  u -> image_data = (float**)malloc(m*sizeof(float*));
  u -> image_data[0] = (float*)malloc(m*n*sizeof(float));
  for (int i=0; i < m; i++){
    u -> image_data[i] = &(u->image_data[0][i*n]);
    }
}

void deallocate_image(image *u){

  // Deallocating an image
  // Inputs:
  // image *u: pointer of image struct for storing image data

  free(u->image_data[0]);
  free(u->image_data);
}


void convert_jpeg_to_image(const unsigned char* image_chars, image *u){

  // Looping through all pixels of a jpeg file, and converting into usable image data
  // Inputs:
  // const unsigned char* image chars: Pointer for image chars
  // image *u: pointer of image struct for storing image data

  int m = u->m;
  int n = u->n;
  for (int i=0; i<m; i++)
      for (int j=0; j<n; j++)
          u->image_data[i][j] = (float) image_chars[i*n+j];
}

void convert_image_to_jpeg(const image *u, unsigned char* image_chars){

  // Same as the last function, just the other way around
  // Inputs:
  // unsigned char* image chars: Pointer for image chars
  // image *u: pointer of image struct for storing image data

  int m = u->m;
  int n = u->n;
  for (int i=0; i<m; i++)
      for (int j=0; j<n; j++)
          image_chars[i*n+j] = (unsigned char) u->image_data[i][j];
}

void iso_diffusion_denoising(image *u, image *u_bar, float kappa, int iters){

  // The actual denoising algorithm
  // Inputs:
  // image *u: pointer of image struct for storing image data
  // image *u_bar: pointer of image struct for storing denoised image data
  // float kappa: constant scaling factor. Usually small. 0.2 as default.
  // int iters: Number of iterations of denoising

  int m = u->m;
  int n = u->n;

  for (int i=0;i<iters; i++){
    for (int j=1; j < m-1; j++){
      for (int k=1; k < n-1; k++){
          u_bar -> image_data[j][k] = u -> image_data[j][k] + kappa*(u->image_data[j-1][k]
          + u->image_data[j][k-1] - 4*u->image_data[j][k] + u->image_data[j][k+1]
          + u->image_data[j+1][k]); }}

    // Swapping the image data to the new denoised data
    for (int j=1; j < m-1; j++){
      for (int k=1; k < n-1; k++){
        u -> image_data[j][k] = u_bar -> image_data[j][k]; }}
  }
}
