
#include "menu.h"
#include "string.h"

int menu_selection = 0;
int menu_entries_count;
char *menu_entries[MAX_MENU_ENTRIES];
char *menu_entries_help[MAX_MENU_ENTRIES];

const char	top_right_corner = 191,
			bottom_left_corner = 192,
			horiz_line = 196,
			bottom_right_corner = 217,
			top_left_corner = 218,
			vert_line = 179;

void draw_box_at(int x, int y, int w, int h) {
	int base_x = x;
	int max_x = x+w-1, max_y=y+h-1;
	
	if ((h<2) || (w<2))
		return;
		
	// draw top of the box
	gfx_printch_at(x, y, top_left_corner);
	for(x++;x<max_x;x++) {
		gfx_printch_at(x, y, horiz_line);
	}
	gfx_printch_at(x, y, top_right_corner);
	
	// start drawing body
	x = base_x;
	for (y++;y<max_y;y++) {
		gfx_printch_at(x, y, vert_line);
		gfx_printch_at(max_x, y, vert_line);
	}
	
	// draw bottom of the box
	gfx_printch_at(x, y, bottom_left_corner);
	for(x++;x<max_x;x++) {
		gfx_printch_at(x, y, horiz_line);
	}
	gfx_printch_at(x, y, bottom_right_corner);
}

const char	*timeout_prompt = "The highlighted entry will be booted automatically in ",
			*timeout_prompt_term = " seconds.";

#define BOX_H (CONSOLE_LINES-4-HELP_LINES)

void menu_draw(int seconds) {
	const char *menu_title = "Gumboot menu v0.1";

    gfx_print_at((CONSOLE_COLUMNS-strlen(menu_title))/2, 1, menu_title);
    
    draw_box_at(0, 3, CONSOLE_COLUMNS, BOX_H);
    
    // draw help text
    gfx_print_at(2, BOX_H+3, "Use the power (\x18) and reset (\x19) buttons to highlight an entry.\n"
												"Long-press (2s) power or press eject to boot.");

	// draw timeout text
	if (seconds != 0)
		gfx_printf_at(2, BOX_H+3+3, "%s%*d", timeout_prompt, 2, seconds, timeout_prompt_term);
}

void menu_draw_entries(void) {
	// a poorly configured menu
	if (menu_entries_count == 0)
		return;
}

void menu_update_timeout(int seconds) {
	gfx_printf_at(2 + strlen(timeout_prompt), BOX_H+3+3, "%*d", 2, seconds);
}

void menu_clear_timeout(void) {
	gfx_clear(2, BOX_H+3+3, strlen(timeout_prompt) + 2 + strlen(timeout_prompt_term), 1);
}
