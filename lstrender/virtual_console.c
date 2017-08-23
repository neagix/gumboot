
#include "virtual_console.h"
#include "../console_common.h"

#include <stdio.h>

unsigned char *vfb;
unsigned vfb_stride;

int selected_font;

void select_font(int font) {
	selected_font = font;
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

	for(int y = 0; y < h*CONSOLE_CHAR_HEIGHT; y++) {
		memset32(fb, c.as_u32, w * CONSOLE_CHAR_WIDTH);

		fb += vfb_stride/4;
	}
}

static void memcpy_font(u32 *dst, unsigned char font_row, rgb *fg, rgb *bg) {
	for (u8 x = 0; x < CONSOLE_CHAR_WIDTH; x++) {
		if ((font_row >> (CONSOLE_CHAR_WIDTH-1-x)) & 0x01) {
			dst[x] = fg->as_u32;
		} else {
			dst[x] = bg->as_u32;
		}
	}
}

void gfx_draw_char(int dx, int dy, unsigned char c) {
    u32 *fb = (u32 *)vfb;
    rgb *fg, *bg;

	switch (selected_font) {
		case FONT_HEADING:
			fg = &config_color_heading[0];
			bg = &config_color_heading[1];
			break;
		case FONT_NORMAL:
			fg = &config_color_normal[0];
			bg = &config_color_normal[1];
			break;
		case FONT_HELPTEXT:
			fg = &config_color_helptext[0];
			bg = &config_color_helptext[1];
			break;
		case FONT_HIGHLIGHT:
			fg = &config_color_highlight[0];
			bg = &config_color_highlight[1];
			break;
		case FONT_ERROR:
			fg = &color_error[0];
			bg = &color_error[1];
			break;
		default:
			// PANIC!
			return;
	}

	fb += dy * vfb_stride/4;
	fb += dx;
	
    for(u8 y = 0; y < CONSOLE_CHAR_HEIGHT; y++) {
		unsigned char font_row = console_font_8x16[c*CONSOLE_CHAR_HEIGHT + y];
		memcpy_font(fb, font_row, fg, bg);

		fb += vfb_stride/4;
	}
}
