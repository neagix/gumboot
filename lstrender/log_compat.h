
#include <stdio.h>
#include <stdarg.h>

static void log_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

/*#define log_init_bb
#define log_flush_bb
#define log_free_bb
*/
