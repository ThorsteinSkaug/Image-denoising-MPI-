**The Project**

This project concerns denoising grey-scale images with a simple ISO denoising algorithm. It can be run either as a serial code, or using MPI parallelisation. If MPI parallelisation is used, the image will be divided into parts, one part for each process, and denoised simultaneously and then put back together as one image. For larger, high-quality (i.e. a lot of pixels) images, this may shorten the execution time quite a bit. For smaller images, however, the parallelisation is not a noteworthy improvement.

The implemented algorithms and general numerical work is very limited, as most of the code is provided and the denoising algorithm is quite simple. However, it does give a good insight in the use of MPI parallelisation.


**Serial code**

To run the code, it is important that the library 'simple-jpeg' is in the parent directory, and the image which you want to denoise is in the working directory.

To compile this project, simply run the command 'make'. To execute, the command 'make run' will do the trick. If you want to change the input file, the kappa value, or the number of denoising iterations, this can be done in the Makefile. The default run uses kappa = 0.2, with 100 iterations. The default input file is 'mona_lisa_noisy.jpg', with 'mona_lisa_smooth.jpg' as the output file.


**Parallel code**

To run the code, it is important that the library 'simple-jpeg' is in the parent directory, and the image which you want to denoise is in the working directory.  You also need to have the MPI module installed and set up on your computer. This can be done using (On Ubuntu):

sudo apt-get install mpi
sudo apt install libopenmpi-dev

To compile this project, simply run the command 'make'. To execute, the command 'make run' will do the trick. If you want to change the input file, the kappa value, or the number of denoising iterations, this can be done in the Makefile. The default run uses kappa = 0.2, with 100 iterations. The default input file is 'mona_lisa_noisy.jpg', with 'mona_lisa_smooth.jpg' as the output file. You can also change the number of processes used in the Makefile. The default value is 4. If only one process is possible on your unit, use the serial code.  
