#ifndef _CONFIG_H
#define _CONFIG_H

#define MAX_CONFIG_ENTRIES 32

typedef struct {
	char 	*title,
			*help_text,
			*root,
			*kernel,
			*find_args;
	int save_default, reboot, poweroff;
	int selected_default; // -1 when referencing self
} stanza;

void config_load(void);

extern int config_timeout, config_entries_count;
extern stanza config_entries[MAX_CONFIG_ENTRIES];

extern unsigned long long config_color_normal,
	config_color_highlight,
	config_color_helptext,
	config_color_heading;

#define get_bgcolor(c) ((unsigned long)(c >> 32))
#define get_fgcolor(c) ((unsigned long)c)

#endif // _CONFIG_H
