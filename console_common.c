
#include "console_common.h"

#ifdef GUMBOOT
#include "string.h"
#include "printf.h"

#else
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#endif

char pf_buffer[4096];

void gfx_print_at(int x, int y, const char *str) {
	unsigned int i;

	int orig_x = CONSOLE_X_OFFSET + x * CONSOLE_CHAR_WIDTH;
	int dx = orig_x;

	for (i = 0; i < strlen(str); i++) {
		int dy = CONSOLE_Y_OFFSET + y * CONSOLE_ROW_HEIGHT;

		if (str[i] == '\n') {
			y++;
			dx = orig_x;
			continue;
		}	

		gfx_draw_char(dx, dy, str[i]);
		dx += CONSOLE_CHAR_WIDTH;
	}
}

int gfx_printf_at(int x, int y, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(pf_buffer, sizeof(pf_buffer)-1, fmt, args);
	va_end(args);

	if (i > 0) {
		gfx_print_at(x, y, pf_buffer);
	}

	return i;
}

// apply (pre-multiplied) alpha blending
// https://www.ocf.berkeley.edu/~horie/alphablend.html
rgb apply_alpha(rgb pixel, rgb bg) {
	rgb result;
	result.as_rgba.a = 0xFF;
	u8 invalpha = 0xFF - pixel.as_rgba.a;

	result.as_rgba.r = (pixel.as_rgba.r   * pixel.as_rgba.a + bg.as_rgba.r   * invalpha)/256;
	result.as_rgba.g = (pixel.as_rgba.g   * pixel.as_rgba.a + bg.as_rgba.g   * invalpha)/256;
	result.as_rgba.b = (pixel.as_rgba.b   * pixel.as_rgba.a + bg.as_rgba.b   * invalpha)/256;
	
	return result;
}
