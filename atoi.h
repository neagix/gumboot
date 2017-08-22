
#ifdef GUMBOOT
#include "types.h"
#else
#include <stddef.h>
#endif

int atoi_base(register char *s, register unsigned int base, size_t *parsed);
