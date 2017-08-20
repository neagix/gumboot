#ifndef _CONFIG_H
#define _CONFIG_H

#define MAX_CONFIG_ENTRIES 32

void config_load(void);

extern int config_timeout, config_entries_count;

#endif // _CONFIG_H
