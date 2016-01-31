#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
#include <time.h>
#include <iostream>
#include <mpi.h>
#include <iomanip>
#include <cmath>

using namespace std;

struct BITMAPFILEHEADER{
   uint8_t type[2];
   uint32_t size;
   uint16_t reserved1;
   uint16_t reserved2;
   uint32_t offsetbits;
} __attribute__ ((packed));



struct BITMAPINFOHEADER{
   uint32_t size;
   uint32_t width;
   uint32_t height;
   uint16_t planes;
   uint16_t bitcount;
   uint32_t compression;
   uint32_t sizeimage;
   long xpelspermeter;
   long ypelspermeter;
   uint32_t colorsused;
   uint32_t colorsimportant;
} __attribute__ ((packed));



struct myRGB{
   uint8_t blue; 
   uint8_t green;
   uint8_t red;
};

struct grayStruct{
   uint8_t B;
   uint8_t G;
   uint8_t R;
};

int main(int argc, char* argv[])
{

  clock_t start = clock();
  
  int rank;
  int nproc;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  BITMAPFILEHEADER source_head;
  BITMAPINFOHEADER source_info;
  

  FILE *in;
  FILE *out;

  if(!(in=fopen(argv[1],"rb")))
    {
    	printf("\ncan not open file");
    	exit(-1);
  }
 
  out=fopen("out_mpi.bmp","wb");

  fread(&source_head,sizeof(struct BITMAPFILEHEADER),1,in);
  fread(&source_info,sizeof(struct BITMAPINFOHEADER),1,in);
 
  int total_pixel = source_info.width * source_info.height;
  
  myRGB* pixel;
  pixel = (myRGB*) malloc (sizeof(myRGB)*total_pixel);
  
  
  int* grayValue  = (int*) malloc(total_pixel*sizeof(int));
  int* R  = (int*) malloc(total_pixel*sizeof(int));
  int* G  = (int*) malloc(total_pixel*sizeof(int));
  int* B  = (int*) malloc(total_pixel*sizeof(int));

   if (rank == 0 )  
   {
    
    int g=0;
    fread(pixel,sizeof(myRGB),total_pixel,in);
    for (int x = 0; x < total_pixel; x++)
     {
       R[x] = pixel[x].red;
       G[x] = pixel[x].green;
       B[x] = pixel[x].blue;
     } 
   }
 
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Bcast(R,total_pixel,MPI_INT,0,MPI_COMM_WORLD);
  MPI_Bcast(G,total_pixel,MPI_INT,0,MPI_COMM_WORLD);
  MPI_Bcast(B,total_pixel,MPI_INT,0,MPI_COMM_WORLD);
 
 
  int chunksize=  ceil((double)total_pixel/nproc);
  cout<< "Chunk Size: " << chunksize<< endl;
  MPI_Barrier(MPI_COMM_WORLD);
 
  int k=0;
  int *local_gray = (int*) malloc ( sizeof(int)*chunksize);

  for( int i = rank*chunksize ; i< rank*chunksize+chunksize && i < total_pixel; i++)
  {
   local_gray[k] = (int)(0.33*R[i] + 0.33*G[i] + 0.33*B[i]);
   k++;
  }
 
  MPI_Barrier(MPI_COMM_WORLD); 
  MPI_Gather( local_gray, chunksize, MPI_INT, grayValue, chunksize, MPI_INT, 0, MPI_COMM_WORLD);
 
  if(rank == 0)
  {
   fwrite(&source_head,sizeof(struct BITMAPFILEHEADER),1,out);
   fwrite(&source_info,sizeof(struct BITMAPINFOHEADER),1,out);
   grayStruct* gray = (grayStruct*) malloc(sizeof(grayStruct)*total_pixel);
  
 	for (int x = 0; x < total_pixel; x++)
    {
     gray[x].R=grayValue[x];
     gray[x].G=grayValue[x];
     gray[x].B=grayValue[x];
    }
   fwrite(gray,sizeof(grayStruct),total_pixel,out);
  }

  fclose(in);
  fclose(out);
 
  clock_t end = clock();

  double t_time = (end - start)/(double)CLOCKS_PER_SEC;
  cout << "Rank:" << rank << " Time: " << std::setprecision(9) << t_time << endl;
  
  MPI_Finalize();
  return 0;
}
