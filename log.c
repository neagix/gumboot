
#include "log.h"
#include "printf.h"

int log_printf(const char *fmt, ...) {
	va_list args;
	char buffer[4096];
	int i;

	va_start(args, fmt);
	i = vsprintf(buffer, fmt, args);
	va_end(args);
	
	gfx_print(buffer, i);

	if (gecko_console_enabled)
		return gecko_sendbuffer(buffer, i);
	return i;
}

