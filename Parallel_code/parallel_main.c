#include "parallel_header.h"

// Defining the functions from simple-jpeg library
void import_JPEG_file (const char* filename, unsigned char** image_chars,
  int* m, int* n,
  int* conents);

void export_JPEG_file (const char* filename, const unsigned char* image_chars,
  int m, int n,
  int conents, int quality);


int main(int argc, char *argv[]){

  // Defining variables, including variables defined from input,
  // here they are set in the Makefile.
  char input_jpeg_filename[32], output_jpeg_filename[32];
  float kappa = atof(argv[1]);
  int iters = atoi(argv[2]);
  strncpy(input_jpeg_filename, argv[3], 32);
  strncpy(output_jpeg_filename, argv[4], 32);
  int my_m, my_rank, num_procs, n, m, c;
  image u, u_bar;
  unsigned char *image_chars;


  // Initializing MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  //Using the first process to import jpg file for denoising
  if(my_rank == 0) {
    import_JPEG_file(input_jpeg_filename, &image_chars, &m, &n, &c);}

  // Broadcasting dimensions of image to all processes
  MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);


  // Finding the workload to load on each process
  int load = m / num_procs;
  int rest = m % num_procs;

  // Adding one to load if it is impossible to evenly didivde the workload on the processes
  if(rest != 0) load++;
  my_m = load;

  int load_size = my_m*n;
  // Number of pixels each processor is working on:
  if(my_rank == num_procs - 1) my_m = m-my_rank*load;

  // An array for the load sizes for all processes
  int *load_array = malloc(num_procs*sizeof(*load_array));
  // An array containing the image chars
  unsigned char *my_image_chars = malloc(load_size*sizeof(my_image_chars));


  // Collects the send buffer contents for all processes and
  // sorts them in rank order into the receive buffer
  MPI_Gather(&load_size, 1, MPI_INT, &load_array[my_rank], 1, MPI_INT, 0, MPI_COMM_WORLD);


  // Making an array for the start index for each process in the image_char array
  int *init = malloc(num_procs*sizeof(*init));
  for(int i = 0; i < num_procs; i++){
      init[i] = i*load_array[i];
  }

  // Opposite of MPI_gather, distrbuting the image_chars to the different processes.
  MPI_Scatterv(image_chars, load_array, init, MPI_UNSIGNED_CHAR, my_image_chars, load_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);


  // Allocating memory for image
  allocate_image(&u, my_m, n);
  allocate_image(&u_bar, my_m, n);
  // Converting jpg file to image data
  convert_jpeg_to_image(my_image_chars, &u);


  // Image denoising
  iso_diffusion_denoising_parallel(&u, &u_bar, kappa, iters, my_rank, num_procs);

  // Converting all image chars back into jpeg format
  convert_image_to_jpeg(my_image_chars, &u);

  // Gathering all image chars from the different processes as one file
  MPI_Gatherv(my_image_chars, load_size, MPI_UNSIGNED_CHAR, image_chars, load_array, init, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

  // Using the first processor to export the final jpeg file
  if(my_rank==0){
    export_JPEG_file(output_jpeg_filename, image_chars, m, n, c, 75);
    }

  // Deallocating the memory of the image before and after denoising.
  deallocate_image(&u);
  deallocate_image(&u_bar);

  // Finalising the MPI-tree
  MPI_Finalize();

  return 0;
}
