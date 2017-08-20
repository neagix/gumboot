
#ifndef _MENU_H
#define _MENU_H

#include "console.h"

#define HELP_LINES 5
#define MAX_MENU_ENTRIES 32

#define DISP_UP		0x18
#define DISP_DOWN	0x19

void draw_box_at(int x, int y, int w, int h);
void menu_draw(int seconds);
void menu_update_timeout(int seconds);
void menu_clear_timeout(void);
void menu_draw_entries(void);

extern int menu_selection;
extern int menu_entries_count;
extern char *menu_entries[MAX_MENU_ENTRIES];
extern char *menu_entries_help[MAX_MENU_ENTRIES];

#endif // _MENU_H
