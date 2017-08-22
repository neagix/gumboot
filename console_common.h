
#ifndef __CONSOLE_COMMON_H
#define __CONSOLE_COMMON_H

#include "types.h"

int gfx_printf_at(int x, int y, const char *fmt, ...);
void gfx_printch_at(int x, int y, char c);
void gfx_print_at(int x, int y, const char *str);

extern char pf_buffer[4096];
extern void gfx_draw_char(int x, int y, char c);

#endif // __CONSOLE_COMMON_H
