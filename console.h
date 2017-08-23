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
#include "console_common.h"

int gfx_printf(const char *fmt, ...);
void init_fb(int vmode);
void free_font(int font);
void init_font(int font);
void gfx_print(const char *str, size_t len);
void gfx_clear(int x, int y, int w, int h, rgb c);
u32 *get_xfb(void);
void clear_fb(rgb fill_rgbc);
void console_clear(void);
void console_move(int x, int y);
void console_set_blinker(int status);

void select_font(int font);

extern int gfx_console_init;

#endif

