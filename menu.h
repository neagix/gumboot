
#ifndef _MENU_H
#define _MENU_H

#include "console.h"

#define HELP_LINES 5

#define DISP_UP		0x18
#define DISP_DOWN	0x19

void draw_box_at(int x, int y, int w, int h);
void menu_draw(int seconds);
void menu_update_timeout(int seconds);

#endif // _MENU_H
