#ifndef _CONFIG_H
#define _CONFIG_H

#include "types.h"

#define MAX_CONFIG_ENTRIES 32

typedef struct {
	u8 r, g, b;
} rgb;

typedef struct {
	char 	*title,
			*help_text,
			*root,
			*kernel,
			*kernel_args,
			*find_args;
	u8 reboot, poweroff, browse;
	u8 root_part_no;
} stanza;

int config_load(void);

extern int config_timeout, config_default, config_entries_count, config_vmode, config_nomenu;
extern stanza config_entries[MAX_CONFIG_ENTRIES];

#define rgbcmp(a,b) memcmp(&a, &b, sizeof(a))

extern rgb config_color_normal[2],
	config_color_highlight[2],
	config_color_helptext[2],
	config_color_heading[2];
	
extern rgb color_default[2], color_default_invert[2];

#define get_bgcolor(c) ((unsigned long)(c >> 32))
#define get_fgcolor(c) ((unsigned long)c)

int config_open_fs(u8 part_no);

#endif // _CONFIG_H
