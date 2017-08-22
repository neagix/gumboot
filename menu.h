
#ifndef _MENU_H
#define _MENU_H

#include "console.h"
#include "config.h"
#include "types.h"

void menu_update_timeout(int seconds);
void menu_clear_timeout(void);

void menu_up(void);
void menu_down(void);
int menu_activate(void);

#endif // _MENU_H
