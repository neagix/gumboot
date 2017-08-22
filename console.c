/*
	BootMii - a Free Software replacement for the Nintendo/BroadOn bootloader.
	Requires mini.

Copyright (C) 2009		bLAStY <blasty@bootmii.org>
Copyright (C) 2009		John Kelley <wiidev@kelley.ca>

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#include "powerpc.h"
#include "video_low.h"
#include "console.h"
#include "string.h"
#include "printf.h"
#include "malloc.h"
#include "config.h"

#include <stdarg.h>

#include "console_common.h"

int gfx_console_init = 0;

static u32 *xfb = NULL;
static int y_add = 0;
// current absolute cursor position
static int console_pos = 0;

u32 *font_yuv_normal[255],
	*font_yuv_highlight[255],
	*font_yuv_helptext[255],
	*font_yuv_heading[255],
	// used only to print log entries
	*font_yuv_error[255];

u32 **selected_font_yuv = NULL;

u32 *get_xfb(void) {
	return xfb;
}

static void memcpy32(u32 *dst, u32 *src, u32 count) {
	while(count--) {
		*dst = *src;
		sync_after_write((const void *)dst, 4);

		dst++;
		src++;
	}
}

static void memset32(u32 *dst, u32 setval, u32 count) {
	while(count--) {
		*dst = setval;
		sync_after_write((const void *)dst, 4);

		dst++;
	}
}

u32 pal_idx(int i, u8 *pal, u8 *gfx) { 
	u32 pidx = gfx[i];
	pidx *= 3;

	return (pal[pidx+0] << 16) | (pal[pidx+1] << 8) | (pal[pidx+2]);
}

int make_yuv(u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2) {
  int y1, cb1, cr1, y2, cb2, cr2, cb, cr;
 
  y1 = (299 * r1 + 587 * g1 + 114 * b1) / 1000;
  cb1 = (-16874 * r1 - 33126 * g1 + 50000 * b1 + 12800000) / 100000;
  cr1 = (50000 * r1 - 41869 * g1 - 8131 * b1 + 12800000) / 100000;
 
  y2 = (299 * r2 + 587 * g2 + 114 * b2) / 1000;
  cb2 = (-16874 * r2 - 33126 * g2 + 50000 * b2 + 12800000) / 100000;
  cr2 = (50000 * r2 - 41869 * g2 - 8131 * b2 + 12800000) / 100000;
 
  cb = (cb1 + cb2) >> 1;
  cr = (cr1 + cr2) >> 1;
 
  return ((y1 << 24) | (cb << 16) | (y2 << 8) | cr);
}

void fill_rect(int x, int y, int w, int h, u8 r, u8 g, u8 b) {
	u32 *fb = xfb;
	u32 col = make_yuv(r,g,b, r,g,b);

	fb += ((y + y_add) * (RESOLUTION_W >> 1));
	fb += (x >> 1);

	for(y = 0; y < h; y++) {
		memset32(fb, col, w >> 1);
		fb += (RESOLUTION_W >> 1);
	}
}

void gfx_draw_rect(gfx_rect *n, char c) {
        u32 y;
        gfx_rect *d_rect;
        u32 *fb = xfb;
        u32 *yuv_data = selected_font_yuv[ (int)c ];

        d_rect = n;

        fb += ((d_rect->y + y_add) * (RESOLUTION_W >> 1));
        fb += (d_rect->x >> 1);

        for(y = 0; y < d_rect->height; y++) {
                memcpy32(fb, yuv_data + ((d_rect->width >> 1) * y), d_rect->width >> 1);
                fb += (RESOLUTION_W >> 1);
        }
}

void scroll(void) {
	unsigned int y;
	u32 *fb = xfb;

	fb += ((CONSOLE_Y_OFFSET+y_add+CONSOLE_ROW_HEIGHT) * (RESOLUTION_W >> 1));
	fb += (CONSOLE_X_OFFSET >> 1);

	for (y = 0; y < CONSOLE_LINES*CONSOLE_ROW_HEIGHT; y++) {
		memcpy32(fb - (CONSOLE_ROW_HEIGHT * RESOLUTION_W/2), fb, CONSOLE_WIDTH >> 1);
		fb += RESOLUTION_W/2;
	}
	
	fill_rect(CONSOLE_X_OFFSET, CONSOLE_Y_OFFSET+(CONSOLE_LINES-1)*CONSOLE_ROW_HEIGHT,
		CONSOLE_WIDTH, CONSOLE_ROW_HEIGHT, 0, 0, 0);
}

void gfx_clear(int x, int y, int w, int h, rgb c) {
	fill_rect(CONSOLE_X_OFFSET + x*CONSOLE_CHAR_WIDTH, CONSOLE_Y_OFFSET+ y * CONSOLE_ROW_HEIGHT,
		w*CONSOLE_CHAR_WIDTH, h * CONSOLE_ROW_HEIGHT, c.as_rgba.r, c.as_rgba.g, c.as_rgba.b);
}

void gfx_print(const char *str, size_t len) {
	unsigned int i;
	gfx_rect d_char;
	
	d_char.width  = CONSOLE_CHAR_WIDTH;
	d_char.height = CONSOLE_CHAR_HEIGHT;
	
	for (i = 0; i < len; i++) {
		// special case: a newline forces to reposition on next line
		if (str[i] == '\n') {
			console_pos += (CONSOLE_COLUMNS - console_pos % CONSOLE_COLUMNS);
		} else {
			// increase absolute position by 1
			console_pos++;
		}
		// calculate new coordinates
		int x = console_pos % CONSOLE_COLUMNS;
		int y = console_pos / CONSOLE_COLUMNS;

		// did the line number increase?
		if (x == 0) {
			// is the new y coordinate overflowing height?
			if (y == CONSOLE_LINES) {
				scroll();
				y = CONSOLE_LINES - 1;
				// adjust to new scrolled position
				console_pos = y * CONSOLE_COLUMNS + x;
			}
		}
		
		// nothing to draw for newlines
		if (str[i] == '\n') {
			continue;
		}

		d_char.x = CONSOLE_X_OFFSET + x * CONSOLE_CHAR_WIDTH;
		d_char.y = CONSOLE_Y_OFFSET + y * CONSOLE_ROW_HEIGHT;
		
		gfx_draw_rect(&d_char, str[i]);
	}
}

int gfx_printf(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(pf_buffer, sizeof(pf_buffer)-1, fmt, args);
	va_end(args);

	if (i > 0) {
		gfx_print(pf_buffer, i);
	}

	return i;
}

void free_font(u32 *font_yuv[255]) {
	int i;

	for (i = 0; i < 255; i++) {
		free(font_yuv[i]);
	}
}

void font_to_yuv(u32 *font_yuv[255], u8 fill_r, u8 fill_g, u8 fill_b, u8 back_r, u8 back_g, u8 back_b) {
	int i, x, y;
	u8 lr,lg,lb, rr,rg,rb;

	for (i = 0; i < 255; i++) {
		font_yuv[i] = (u32*)malloc(8*CONSOLE_CHAR_HEIGHT*2);

		for (y = 0; y < CONSOLE_CHAR_HEIGHT; y++) {
			for (x = 0; x < CONSOLE_CHAR_WIDTH; x+=2) {
				if (((console_font_8x16[(i*CONSOLE_CHAR_HEIGHT)+y] >> (CONSOLE_CHAR_WIDTH-1-x)) & 0x01) == 1) {
					lr = fill_r; lg = fill_g; lb = fill_b;
				} else {
					lr = back_r; lg = back_g; lb = back_b;
				}

				if (((console_font_8x16[(i*CONSOLE_CHAR_HEIGHT)+y] >> (CONSOLE_CHAR_WIDTH-1-(x+1))) & 0x01) == 1) {
					rr = fill_r; rg = fill_g; rb = fill_b;
				} else {
					rr = back_r; rg = back_g; rb = back_b;
				}

				font_yuv[i][(y<<2)+(x>>1)] = make_yuv(lr, lg, lb, rr, rg, rb);
			}
		}
	}
}

void init_font(rgb c[2], u32 *font_yuv[255]) {
	// re-initialise color
	font_to_yuv(font_yuv, c[0].as_rgba.r, c[0].as_rgba.g, c[0].as_rgba.b, c[1].as_rgba.r, c[1].as_rgba.g, c[1].as_rgba.b);
}

void init_fb(int vmode) {
	// default fonts setup before configuration is loaded
	// and used to display configuration load errors / other log errors
	font_to_yuv(font_yuv_error, 255, 0, 0, 0, 0, 0);
	font_to_yuv(font_yuv_normal, 255, 255, 255, 0, 0, 0);
	selected_font_yuv = font_yuv_normal;
	
	switch(vmode) {
	case VIDEO_640X480_NTSCi_YUV16:
	case VIDEO_640X480_PAL60_YUV16:
	case VIDEO_640X480_NTSCp_YUV16:
		y_add = 0;
		break;

	case VIDEO_640X480_PAL50_YUV16:
		y_add = 48;
		break;
	}

	xfb = memalign(32, RESOLUTION_W * (RESOLUTION_H + (y_add*2)) * 2);
	
	rgb black = {.as_u32 = 0};
	clear_fb(black);
}

void clear_fb(rgb fill_rgb) {
	int i;
	u32 *fb;

	u32 fill_yuv = make_yuv(fill_rgb.as_rgba.r, fill_rgb.as_rgba.g, fill_rgb.as_rgba.b,fill_rgb.as_rgba.r, fill_rgb.as_rgba.g, fill_rgb.as_rgba.b);

	fb  = xfb;
	for (i = 0; i < (RESOLUTION_H + (y_add*2)) * 2 * (RESOLUTION_W >> 1); i++) {
		*fb = fill_yuv;
		sync_after_write(fb, 4);
		fb++;
	}
}

void console_clear(void) {
	console_pos = 0;
	clear_fb(config_color_normal[1]);
}

void select_font(int font) {
	switch (font) {
		case FONT_HEADING:
			selected_font_yuv = font_yuv_heading;
			break;
		case FONT_NORMAL:
			selected_font_yuv = font_yuv_normal;
			break;
		case FONT_HELPTEXT:
			selected_font_yuv = font_yuv_helptext;
			break;
		case FONT_HIGHLIGHT:
			selected_font_yuv = font_yuv_highlight;
			break;
		case FONT_ERROR:
			selected_font_yuv = font_yuv_error;
			break;
	}
}
