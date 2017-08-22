
#include "../config.h"
#include "../console_common.h"

void gfx_clear(int x, int y, int w, int h, rgb c);
void gfx_draw_char(int dx, int dy, char c);

void select_font(int font);

extern unsigned char *vfb;
extern unsigned vfb_stride;
