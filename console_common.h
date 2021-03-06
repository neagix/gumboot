
#ifndef __CONSOLE_COMMON_H
#define __CONSOLE_COMMON_H

#include "types.h"
#include "config.h"
#include "raster.h"

#define CONSOLE_CHAR_WIDTH 8
#define CONSOLE_CHAR_HEIGHT 16

#define CONSOLE_ROW_HEIGHT CONSOLE_CHAR_HEIGHT

#define RESOLUTION_W 640
#define RESOLUTION_H 480

#define CONSOLE_WIDTH RESOLUTION_W
#define CONSOLE_X_OFFSET  0
#ifdef GUMBOOT
#define CONSOLE_Y_OFFSET (CONSOLE_CHAR_HEIGHT-2)
#define CONSOLE_LINES ((RESOLUTION_H-CONSOLE_Y_OFFSET)/CONSOLE_ROW_HEIGHT - 1)
#else
#define CONSOLE_Y_OFFSET 0
#define CONSOLE_LINES (RESOLUTION_H/CONSOLE_ROW_HEIGHT)
#endif

#define CONSOLE_COLUMNS (CONSOLE_WIDTH/CONSOLE_CHAR_WIDTH)

#define FONT_NORMAL		0x0
#define FONT_HELPTEXT	0x1
#define FONT_HIGHLIGHT	0x2
#define FONT_HEADING	0x3
#define FONT_ERROR		0x4

// utility function for software alpha blending
rgb apply_alpha(rgb pixel, rgb bg);

int gfx_printf_at(int x, int y, const char *fmt, ...);
void gfx_print_at(int x, int y, const char *str);

#define gfx_printch_at(x, y, c)		gfx_draw_char(CONSOLE_X_OFFSET + (x) * CONSOLE_CHAR_WIDTH, CONSOLE_Y_OFFSET + (y) * CONSOLE_ROW_HEIGHT, c)

extern char pf_buffer[4096];

// defined either by XFB or VFB implementations
extern void gfx_draw_char(int x, int y, unsigned char c);
extern int console_render_splash(raster rst);
extern void console_blit(int dx, int dy, raster rst, rgb solid_bg, u32 *yuv_row);

// defined in font.c
extern unsigned char console_font_8x16[256*CONSOLE_CHAR_HEIGHT];

// some special characters in our console font
#define DISP_UP		0x18
#define DISP_DOWN	0x19
#define DISP_RIGHT	(DISP_DOWN+1)
#define DISP_LEFT	(DISP_DOWN+2)

#endif // __CONSOLE_COMMON_H
