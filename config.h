#ifndef _CONFIG_H
#define _CONFIG_H

#include "types.h"

#define MAX_SPLASH_SIZE 3*1024*1024
#define MAX_CONFIG_ENTRIES 32
#define MAX_LST_SIZE 16*1024
#define DEFAULT_LST "gumboot/gumboot.lst"

typedef union {
		struct __attribute__((__packed__)) {
			u8 r, g, b, a;
		} as_rgba;
		u32 as_u32;
} rgb;

typedef struct {
	char 	*title,
			*help_text,
			*root,
			*kernel,
			*kernel_args;
	u8 reboot, poweroff, browse;
	// FatFS does not support extended partitions
	// physical drive is always 0 because only one SD is supported
	// 'root' will include the drive/partition number
} stanza;

char *config_load(const char *fname, u32 *read);
int config_load_from_buffer(char *cfg_data, u32 read);

extern int config_timeout, config_default, config_entries_count, config_vmode, config_nomenu;
extern stanza config_entries[MAX_CONFIG_ENTRIES];

#define rgbcmp(a,b) (a[0].as_u32 == b[0].as_u32) && (a[1].as_u32 == b[1].as_u32)

extern rgb config_color_normal[2],
	config_color_highlight[2],
	config_color_helptext[2],
	config_color_heading[2];

extern rgb color_error[2];
extern rgb color_default[2];
extern char *config_splashimage;

#define get_bgcolor(c) ((unsigned long)(c >> 32))
#define get_fgcolor(c) ((unsigned long)c)

int config_open_fs(u8 part_no);

#endif // _CONFIG_H
