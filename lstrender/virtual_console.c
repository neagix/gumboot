
#include "virtual_console.h"
#include "../console_defs.h"

unsigned char *vfb;

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

void gfx_clear(int x, int y, int w, int h, rgb c) {
	
}

void gfx_print_at(int x, int y, const char *str) {
}

int gfx_printf_at(int x, int y, const char *fmt, ...) {
	return 0;
}
