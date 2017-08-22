
#include <stdio.h>
#include <stdlib.h>

#include "lodepng.h"

#include "../config.h"
#include "../menu_render.h"
 
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
	if (argc != 3) {
		fprintf(stderr, "Usage: lstrender gumboot.lst preview.png\n");
		return -1;
	}
	
	// load configuration file into memory

	FILE *f = fopen(argv[1], "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  //same as rewind(f);

	char *config_data = malloc(fsize + 1);
	fread(config_data, fsize, 1, f);
	fclose(f);

	config_data[fsize] = 0;	
	
	int err = config_load_from_buffer(config_data, fsize);
	free(config_data);
	if (err) {
		fprintf(stderr, "ERROR: %d\n", err);
		return -1;
	}
	
	for(int i =0;i<config_entries_count;i++) {
		printf("%d: %s\n", i, config_entries[i].title);
	}
	
	menu_draw(config_timeout, 0, 0);
	menu_draw_entries();
	
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

	return 0;
}
