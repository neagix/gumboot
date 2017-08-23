
#include "log.h"
#include "printf.h"
#include "console.h"
#include "malloc.h"
#include "string.h"

#define MAX_BACKBUFFER_LEN ((CONSOLE_COLUMNS * CONSOLE_LINES * 2) / 3)

char *back_buffer;
int back_buffer_ptr = 0;

int log_init_bb() {
	back_buffer = malloc(MAX_BACKBUFFER_LEN);
	if (!back_buffer)
		return -1;
	return 0;
}

void log_free_bb() {
	free(back_buffer);
}

int log_print(const char *buffer, int i) {
	if (gfx_console_init) {
		select_font(FONT_ERROR);
		gfx_print(buffer, i);
	} else {
		// accrete the back buffer
		if (i > (MAX_BACKBUFFER_LEN-back_buffer_ptr)) {
			// WE ARE DOOMED! NO MORE BACK BUFFER SPACE! OH NO..WHY..
			return -1;
		}
		
		memcpy(back_buffer+back_buffer_ptr, buffer, i);
		back_buffer_ptr+=i;
	}

	if (gecko_console_enabled)
		return gecko_sendbuffer(buffer, i);

	return i;
}

int log_printf(const char *fmt, ...) {
	va_list args;
	char buffer[4096];
	int i;
	
	va_start(args, fmt);
	i = vsnprintf(buffer, sizeof(buffer)-1, fmt, args);
	va_end(args);
	
	return log_print(buffer, i);
}

void log_flush_bb(void) {
	if (!back_buffer_ptr)
		return;
	
	log_print(back_buffer, back_buffer_ptr);
}

