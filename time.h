
#ifndef _TIME_H
#define _TIME_H

// Time.

#include "types.h"

void usleep(u32 us);
u64 mftb(void);
u64 mftb_usec(void);

#endif // _TIME_H
