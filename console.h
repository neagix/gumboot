/*
	BootMii - a Free Software replacement for the Nintendo/BroadOn bootloader.
	Requires mini.

Copyright (C) 2009		bLAStY <blasty@bootmii.org>
Copyright (C) 2009		John Kelley <wiidev@kelley.ca>

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "types.h"

int gfx_printf(const char *fmt, ...);
int gfx_printf_at(int x, int y, const char *fmt, ...);
void init_fb(int vmode);
void gfx_print(const char *str, size_t len);
void gfx_printch_at(int x, int y, char c);
void gfx_print_at(int x, int y, const char *str);
u32 *get_xfb(void);

extern unsigned char console_font_8x16[];

#define CONSOLE_CHAR_WIDTH 8
#define CONSOLE_CHAR_HEIGHT 16
#define CONSOLE_ROW_HEIGHT CONSOLE_CHAR_HEIGHT

#define RESOLUTION_W 640
#define RESOLUTION_H 480

#define CONSOLE_Y_OFFSET (CONSOLE_CHAR_HEIGHT-2)
#define CONSOLE_X_OFFSET  0

#define CONSOLE_WIDTH RESOLUTION_W
#define CONSOLE_LINES ((RESOLUTION_H-CONSOLE_Y_OFFSET)/CONSOLE_ROW_HEIGHT)
#define CONSOLE_COLUMNS (CONSOLE_WIDTH/CONSOLE_CHAR_WIDTH)

#endif

