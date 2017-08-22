
#include <stdio.h>
#include <stdlib.h>

#include "lodepng.h"
 
/* The image argument has width * height RGBA pixels or width * height * 4 bytes */
void encodeOneStep(const char* filename, const unsigned char* image, unsigned width, unsigned height)
{
  /*Encode the image*/
  unsigned error = lodepng_encode32_file(filename, image, width, height);

  /*if there's an error, display it*/
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
}
 
int main(int argc, char **argv)
{
	FILE *fp;
	if (argc != 3) {
		fprintf(stderr, "Usage: lstrender gumboot.lst preview.png\n");
		return -1;
	}
	fp = fopen(argv[1], "r");
	if( fp == NULL ) {
		perror("error opening file.\n");
		return -2;
	}
	
	//TODO: read file in memory, load configuration via shared code
	
	unsigned width = 512, height = 512;
	unsigned char* image = malloc(width * height * 4);
	unsigned x, y;
	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			image[4 * width * y + 4 * x + 0] = 255 * !(x & y);
			image[4 * width * y + 4 * x + 1] = x ^ y;
			image[4 * width * y + 4 * x + 2] = x | y;
			image[4 * width * y + 4 * x + 3] = 255;
		}
	}

	encodeOneStep("test.png", image, width, height);

	free(image);	
	
	fclose(fp);
	return 0;
}
