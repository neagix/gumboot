
#include "types.h"

int gfx_printf_at(int x, int y, const char *fmt, ...);
void gfx_printch_at(int x, int y, char c);
void gfx_print_at(int x, int y, const char *str);

extern char pf_buffer[4096];

typedef struct {
        u32 x, y;
        u32 width, height;
        //u32 *yuv_data;
        u8 has_alpha;
} gfx_rect;