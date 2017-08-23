
#ifndef __RASTER_H
#define __RASTER_H

typedef struct {
	unsigned width, height;
	unsigned char *pixels;
} raster;

#define png_to_raster(png, pngsize, rst) lodepng_decode32(&rst.pixels, &rst.width, &rst.height, png, pngsize)

#endif // __RASTER_H
