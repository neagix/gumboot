
#ifndef GECKO_H
#define GECKO_H

#include "types.h"

extern int gecko_console_enabled;

void gecko_init(void);
int gecko_printf(const char *fmt, ...);
int gecko_sendbuffer(const void *buffer, u32 size);

#endif
