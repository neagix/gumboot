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
#include "types_compat.h"
#include "config.h"
#include "console_defs.h"

int gfx_printf(const char *fmt, ...);
int gfx_printf_at(int x, int y, const char *fmt, ...);
void init_fb(int vmode);
void free_font(u32 *font_yuv[255]);
void init_font(rgb c[2], u32 *font_yuv[255]);
void gfx_print(const char *str, size_t len);
void gfx_printch_at(int x, int y, char c);
void gfx_print_at(int x, int y, const char *str);
void gfx_clear(int x, int y, int w, int h, rgb c);
u32 *get_xfb(void);
void clear_fb(rgb fill_rgbc);
void console_clear(void);

void select_font(int font);

extern unsigned char console_font_8x16[];

extern u32 **selected_font_yuv;
extern u32 *font_yuv_normal[255],
	*font_yuv_highlight[255],
	*font_yuv_helptext[255],
	*font_yuv_heading[255],
	*font_yuv_error[255];

extern int gfx_console_init;

#endif

