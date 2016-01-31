#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <time.h>
#include <iomanip>

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

int main(int argc, char* argv[])
{

 clock_t start = clock();
 BITMAPFILEHEADER source_head;
 BITMAPINFOHEADER source_info;
 myRGB* pixel;

 FILE *in;
 FILE *out;

 if(!(in=fopen(argv[1],"rb")))
 {
	printf("\ncan not open file");
	exit(-1);
 }


 out=fopen("out_serial.bmp","wb");

 fread(&source_head,sizeof(struct BITMAPFILEHEADER),1,in);
 fread(&source_info,sizeof(struct BITMAPINFOHEADER),1,in);

 fwrite(&source_head,sizeof(struct BITMAPFILEHEADER),1,out);
 fwrite(&source_info,sizeof(struct BITMAPINFOHEADER),1,out);

 int total_pixel=source_info.width*source_info.height;

 pixel = (myRGB*) malloc (sizeof(myRGB)*total_pixel);

 fread(pixel,sizeof(struct myRGB),total_pixel,in);
 
 int gray = 0;
 int w = source_info.width;
 for(int i=0;i<total_pixel;++i)
 {
  int gray = 0.33*pixel[i].red+0.33*pixel[i].green+ 0.33*pixel[i].blue;
 
  pixel[i].red = gray ;
  pixel[i].green = gray;
  pixel[i].blue = gray;
 }

 fwrite(pixel,sizeof(struct myRGB),total_pixel,out);

 fclose(in);
 fclose(out);
 clock_t end = clock();
 cout << "Execution time: " <<  std::setprecision(9) << (double)(end-start)/CLOCKS_PER_SEC << " s\n";
 return 0;
}
