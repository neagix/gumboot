
#ifndef __BROWSE_H
#define __BROWSE_H

#include "menu_render.h"

int menu_browse();
int menu_browse_activate(void);

extern char browse_current_path[4096];
extern char *browse_menu_entries[CONSOLE_LINES-HELP_LINES-HEAD_LINES-2];
extern int browse_menu_entries_count;

#endif // __BROWSE_H
