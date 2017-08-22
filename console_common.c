
#include "console_common.h"
#include "console_defs.h"

#ifdef GUMBOOT
#include "string.h"
#include "printf.h"

extern void gfx_draw_rect(gfx_rect *n, char c);
#endif

char pf_buffer[4096];

void gfx_print_at(int x, int y, const char *str) {
	unsigned int i;
	gfx_rect d_char;

	int orig_x = CONSOLE_X_OFFSET + x * CONSOLE_CHAR_WIDTH;
	d_char.x = orig_x;

	for (i = 0; i < strlen(str); i++) {
		d_char.width  = CONSOLE_CHAR_WIDTH;
		d_char.height = CONSOLE_CHAR_HEIGHT;
		d_char.y = CONSOLE_Y_OFFSET + y * CONSOLE_ROW_HEIGHT;

		if (str[i] == '\n') {
			y++;
			d_char.x = orig_x;
			continue;
		}	

		gfx_draw_rect(&d_char, str[i]);
		d_char.x += CONSOLE_CHAR_WIDTH;
	}
}

void gfx_printch_at(int x, int y, char c) {
	gfx_rect d_char;

	d_char.width  = CONSOLE_CHAR_WIDTH;
	d_char.height = CONSOLE_CHAR_HEIGHT;
	d_char.x = CONSOLE_X_OFFSET + x * CONSOLE_CHAR_WIDTH;
	d_char.y = CONSOLE_Y_OFFSET + y * CONSOLE_ROW_HEIGHT;

	gfx_draw_rect(&d_char, c);
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
