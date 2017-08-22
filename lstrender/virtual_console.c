
#include "virtual_console.h"
#include "../console_defs.h"

unsigned char *vfb;
unsigned vfb_stride;

rgb color_error[2] = {{255, 0, 0}, {0, 0, 0}};

int selected_font;

void select_font(int font) {
/*	switch (font) {
		case FONT_HEADING:
			selected_font = config_color_heading;
			break;
		case FONT_NORMAL:
			selected_font = config_color_normal;
			break;
		case FONT_HELPTEXT:
			selected_font = config_color_helptext;
			break;
		case FONT_HIGHLIGHT:
			selected_font = config_color_highlight;
			break;
		case FONT_ERROR:
			selected_font = color_error;
			break;
	}*/
	selected_font = font;
}

void gfx_printch_at(int x, int y, char c) {
}

static void memset32(u32 *dst, u32 setval, u32 count) {
	while(count--) {
		*dst = setval;

		dst++;
	}
}

void gfx_clear(int offset_x, int offset_y, int w, int h, rgb c) {
	u32 *fb = (u32 *)vfb;

	fb += (offset_y * CONSOLE_CHAR_HEIGHT) * (vfb_stride/4);
	fb += offset_x * CONSOLE_CHAR_WIDTH;

	for(int y = 0; y < h; y++) {
		memset32(fb, c.as_u32, w * CONSOLE_CHAR_WIDTH);
		fb += vfb_stride/4;
	}
}

void gfx_print_at(int x, int y, const char *str) {
}

int gfx_printf_at(int x, int y, const char *fmt, ...) {
	return 0;
}
