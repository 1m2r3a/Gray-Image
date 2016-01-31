#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <time.h>
#include <iomanip>
#include <iostream>

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

__global__ void kernel(myRGB* input, grayStruct* gray,int w,int h, int chunksize)
{  
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
  int totalsize = w*h;
	if( idx < totalsize)
   for( int i = idx*chunksize ; i< idx*chunksize+chunksize && i < totalsize; i++)
     {
      int graypixel = (int)(0.33* input[i].red + 0.33* input[i].green + 0.33* input[i].blue);
      gray[i].R = graypixel;gray[i].G = graypixel;gray[i].B = graypixel;
     }

}
int main(int argc, char* argv[])
{
  clock_t start = clock();

  BITMAPFILEHEADER source_head;
  BITMAPINFOHEADER source_info;

  FILE *in;
  FILE *out;

  if(!(in=fopen(argv[1],"rb")))
    {
    	printf("\ncan not open file");
    	exit(-1);
  }

  out=fopen("out_cuda.bmp","wb");

  fread(&source_head,sizeof(struct BITMAPFILEHEADER),1,in);
  fread(&source_info,sizeof(struct BITMAPINFOHEADER),1,in);
 
  int total_pixel = source_info.width * source_info.height;
  
  myRGB* h_pixel;
  h_pixel = (myRGB*) malloc (sizeof(myRGB)*total_pixel);
  fread(h_pixel,sizeof(myRGB),total_pixel,in);
  
  myRGB* d_pixel;
  cudaMalloc((void **)&d_pixel, total_pixel*sizeof(myRGB));
  cudaMemcpy(d_pixel, h_pixel,total_pixel*sizeof(myRGB), cudaMemcpyHostToDevice);

  grayStruct* d_gray;
  cudaMalloc( (void**) &d_gray, total_pixel*sizeof(grayStruct));
  cudaMemset(d_gray,255,total_pixel*sizeof(grayStruct));


  const int BLOCK_SIZE = atoi(argv[2]);
  const int THREAD_SIZE = atoi(argv[3]);
  int chunksize=  ceil( (double)total_pixel/(BLOCK_SIZE*THREAD_SIZE));
  
  cout<< "Chunk Size: " << chunksize<< endl;
  kernel <<<BLOCK_SIZE ,THREAD_SIZE >>> (d_pixel, d_gray, source_info.width, source_info.height, chunksize);


  fwrite(&source_head,sizeof(struct BITMAPFILEHEADER),1,out);
  fwrite(&source_info,sizeof(struct BITMAPINFOHEADER),1,out);
 
  cudaDeviceSynchronize();
  grayStruct* h_gray = (grayStruct*) malloc (total_pixel*sizeof(grayStruct));
 cudaMemcpy(h_gray, d_gray, total_pixel*sizeof(grayStruct), cudaMemcpyDeviceToHost);

  fwrite(h_gray,sizeof(grayStruct),total_pixel,out);

  fclose(in);
  fclose(out);
  
  clock_t end = clock();
  double t_time = (end - start)/(double)CLOCKS_PER_SEC;
  cout << "Time: " << std::setprecision(9) << t_time << endl;
  
  return 0;
}
