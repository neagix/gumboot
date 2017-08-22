
#ifndef __CONSOLE_COMMON_H
#define __CONSOLE_COMMON_H

#include "types.h"
#include "console_defs.h"

int gfx_printf_at(int x, int y, const char *fmt, ...);
void gfx_print_at(int x, int y, const char *str);

#define gfx_printch_at(x, y, c)		gfx_draw_char(CONSOLE_X_OFFSET + x * CONSOLE_CHAR_WIDTH, CONSOLE_Y_OFFSET + y * CONSOLE_ROW_HEIGHT, c)

extern char pf_buffer[4096];
extern void gfx_draw_char(int x, int y, char c);

#endif // __CONSOLE_COMMON_H
