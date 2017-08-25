
#ifndef _MENU_RENDER_H
#define _MENU_RENDER_H

#include "types.h"
#include "config.h"
#include "logo.h"

#ifdef GUMBOOT
#include "console.h"
#include "string.h"
#include "printf.h"
#else
#include "console_common.h"
#include <string.h>
#include <stdio.h>

#include "lstrender/virtual_console.h"
#endif

#define GUMBOOT_VERSION "0.7"

#define HELP_LINES 5
#define HEAD_LINES 3
#define BOX_H (CONSOLE_LINES-HEAD_LINES-HELP_LINES)
#define LOGO_COLUMNS (GUMBOOT_LOGO_WIDTH/CONSOLE_CHAR_WIDTH + (GUMBOOT_LOGO_WIDTH % CONSOLE_CHAR_WIDTH != 0))

extern int menu_selection, old_menu_selection;

void draw_box_at(int x, int y, int w, int h);
void menu_draw_head_and_box(u16 mini_version_major, u16 mini_version_minor);
void menu_draw_entries_and_help(void);
void menu_clear_entries(void);
void menu_init(raster *splash);

void menu_update_timeout(int seconds);
void menu_clear_timeout(void);
void menu_draw_timeout(int seconds);

extern const char	*timeout_prompt, *timeout_prompt_term;

#endif // _MENU_RENDER_H
