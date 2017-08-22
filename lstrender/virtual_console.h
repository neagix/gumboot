
#include "../config.h"

void gfx_printch_at(int x, int y, char c);
void gfx_clear(int x, int y, int w, int h, rgb c);
void gfx_print_at(int x, int y, const char *str);
int gfx_printf_at(int x, int y, const char *fmt, ...);

void select_font(int font);

extern unsigned char *vfb;
extern unsigned vfb_stride;
