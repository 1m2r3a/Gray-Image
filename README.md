Convert a color BMP file to gray using OpenMP, MPI and CUDA



****
Serial

g++ blur.cpp -o blur

./blur test.bmp



****
OpenMP
icc -openmp grayMP.cpp -o grayMP

OR

gcc -fopenmp grayMP.cpp -o grayMP

./grayMP test.bmp 2 

                  ^Number of threads




****
MPI
mpic++ -o grayMPI grayMPI.cpp

mpirun -n 4 grayMPi test.bmp

          ^number of threads


****
CUDA

nvcc grayCUDA.cu -o grayCUDA

./grayCUDA test.bmp 512 64

                     ^   ^ thread size

                     | block size