#include "parallel_header.h"


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


void convert_jpeg_to_image(const unsigned char *image_chars, image *u){

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

void convert_image_to_jpeg(unsigned char *image_chars, image *u){

  // Same as the last function, just the other way around
  // Inputs:
  // unsigned char* image chars: Pointer for image chars
  // image *u: pointer of image struct for storing image data

  for(int i = 0; i < u->m; i++){
      for(int j = 0; j < u->n; j++){
          image_chars[i*u->n + j] = u->image_data[i][j];
    }
  }
}


void iso_diffusion_denoising_parallel(image *u, image *u_bar, float kappa, int iters, int my_rank, int num_procs){

  // The actual denoising algorithm using MPI
  // Inputs:
  // image *u: pointer of image struct for storing image data
  // image *u_bar: pointer of image struct for storing denoised image data
  // float kappa: constant scaling factor. Usually small. 0.2 as default.
  // int iters: Number of iterations of denoising
  // int my_rank: Process rank
  // int num_procs: Total number of processes.

  //allocating memory for an array for the top and bottom of the image
  float* top = malloc(u->n*sizeof(*top));
  float* bottom = malloc(u->n*sizeof(*bottom));

  // Looping through the edges of the image, defining u and u-bar to be equal.
  // This is because there is no way to denoise the edges with this method, as
  // it is dependent on two adjecent columns or rows.

  for(int i = 0; i < u->n; i++){
      u_bar->image_data[0][i] = u->image_data[0][i];
      u_bar->image_data[u->m-1][i] = u_bar->image_data[u->m-1][i];
  }
  for(int j = 0; j < u->m; j++){
      u_bar->image_data[j][0] = u->image_data[j][0];
      u_bar->image_data[j][u->n-1] = u->image_data[j][u->n-1];
  }

  // The actual iso diffusion algorithm
  for(int iter = 0; iter < iters; iter++){

    // Here the processes are divided up
          if(my_rank == 0){
              MPI_Send(u->image_data[u->m-1], u->n, MPI_FLOAT, my_rank + 1, 0, MPI_COMM_WORLD);
              MPI_Recv(bottom, u->n, MPI_FLOAT, my_rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          }

          else if (my_rank == num_procs - 1){
              MPI_Recv(top, u->n, MPI_FLOAT, my_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
              MPI_Send(u->image_data[0], u->n, MPI_FLOAT, my_rank - 1, 0, MPI_COMM_WORLD);
          }

          else if ((my_rank % 2) == 1){
              MPI_Recv(top, u->n, MPI_FLOAT, my_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
              MPI_Send(u->image_data[u->m-1], u->n, MPI_FLOAT, my_rank + 1, 0, MPI_COMM_WORLD);
              MPI_Recv(bottom, u->n, MPI_FLOAT, my_rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
              MPI_Send(u->image_data[0], u->n, MPI_FLOAT, my_rank - 1, 0, MPI_COMM_WORLD);
          }

          else {
              MPI_Recv(top, u->n, MPI_FLOAT, my_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
              MPI_Send(u->image_data[u->m-1], u->n, MPI_FLOAT, my_rank + 1, 0, MPI_COMM_WORLD);
              MPI_Recv(bottom, u->n, MPI_FLOAT, my_rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
              MPI_Send(u->image_data[0], u->n, MPI_FLOAT, my_rank - 1, 0, MPI_COMM_WORLD);
          }


      // Looping through the image with the iso diffusion equation, disregarding the frame
      for(int i = 1; i < u->m - 1; i++){
          for(int j = 1; j < u->n - 1; j++){
              u_bar->image_data[i][j] = u->image_data[i][j] + kappa*(u->image_data[i+1][j] + u->image_data[i-1][j]
              - 4*u->image_data[i][j] + u->image_data[i][j+1] + u->image_data[i][j-1]);
          }
      }


      // Looping through the pixels on the edges of each process area
      if(my_rank != 0){
          for(int j = 1; j < u->n - 1; j++){
              u_bar->image_data[0][j] = u->image_data[0][j] + kappa*(top[j] + u->image_data[1][j]
              -4*u->image_data[0][j] + u->image_data[0][j+1] + u->image_data[0][j-1]);
          }
      }

      // Looping through the pixels on the edges of each process area and assigning the u-data
      if(my_rank != num_procs - 1){
          for(int j = 1; j < u->n - 1; j++){
              u_bar->image_data[u->m-1][j] = u->image_data[u->m-1][j] + kappa*(u->image_data[u->m-2][j] + bottom[j]
              -4*u->image_data[u->m-1][j] + u->image_data[u->m-1][j+1] + u->image_data[u->m-1][j-1]);
          }
      }


      // Swapping out u for the new u_bar for all pixels.
      int m = u->m;
      int n = u->n;
      for (int j=0; j < m; j++){
        for (int k=0; k < n; k++){
          u -> image_data[j][k] = u_bar -> image_data[j][k]; }}

  }

}
