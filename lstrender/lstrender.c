
#include <stdio.h>
#include <stdlib.h>

#include "lodepng.h"

#include "../config.h"
#include "../menu_render.h"
#include "../raster.h"
 
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
	// sanity check
	if (sizeof(rgb) != 4) {
		fprintf(stderr, "BUG: rgba is not 4 bytes\n");
		return -1;
	}
	
	if (argc != 3) {
		fprintf(stderr, "Usage: lstrender gumboot.lst preview.png\n");
		return -1;
	}
	
	// load configuration file into memory

	FILE *f = fopen(argv[1], "rb");
	if (!f) {
		fprintf(stderr, "ERROR: could not open '%s'\n", argv[1]);
		return -2;
	}
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
	
	unsigned width = RESOLUTION_W, height = RESOLUTION_H;
	
	vfb_stride = width * 4;
	vfb = malloc(vfb_stride * height);

/*	unsigned x, y;
	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			vfb[4 * width * y + 4 * x + 0] = 255 * !(x & y);
			vfb[4 * width * y + 4 * x + 1] = x ^ y;
			vfb[4 * width * y + 4 * x + 2] = x | y;
			vfb[4 * width * y + 4 * x + 3] = 255;
		}
	}*/
	
	printf("columns = %d, lines = %d\n", CONSOLE_COLUMNS, CONSOLE_LINES);
	
	raster *valid_splash = NULL;

    menu_selection = config_default;
    menu_selection = config_default;
    menu_init(valid_splash);
	menu_draw_head_and_box(1, 3);
	menu_draw_timeout(config_timeout);
	
	// check whether to draw help area or not
	menu_draw_entries_and_help();
	
	encodeOneStep(argv[2], vfb, RESOLUTION_W, RESOLUTION_H);

	free(vfb);	

	return 0;
}
