
#include "atoi.h"

int atoi_base(register char *s, register unsigned int base, size_t *parsed) {
    register int result = 0;
    register unsigned int digit;

    for (*parsed = 0; ; s++) {
		digit = *s - '0';
		if (digit > (base-1)) {
			// invalid digit
			break;
		}
		result = (base*result) + digit;
		(*parsed)++;
    }

    return result;
}
