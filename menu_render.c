
#include "menu_render.h"
#include "console_common.h"
#include "raster.h"

extern int gumboot_logo_width,
	gumboot_logo_height;

extern unsigned char gumboot_logo_pixels[];

raster gumboot_logo;

#define DISP_UP		0x18
#define DISP_DOWN	0x19

int menu_selection = 0;

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
const char *menu_title = "Gumboot menu v0.1";			

void menu_draw_head_and_box(u16 mini_version_major, u16 mini_version_minor) {
	select_font(FONT_HEADING);
    gfx_print_at((CONSOLE_COLUMNS-strlen(menu_title))/2, 1, menu_title);
    
    // print MINI version
    char buffer[100];
    int l = snprintf(buffer, sizeof(buffer)-1, "Mini version: %d.%0d", mini_version_major, mini_version_minor);
    gfx_print_at(CONSOLE_COLUMNS-l-1, 2, buffer);

	// draw the box
	select_font(FONT_NORMAL);
	draw_box_at(0, 3, CONSOLE_COLUMNS, BOX_H);
}

#define TIMEOUT_Y BOX_H+HEAD_LINES+HELP_LINES-1

void menu_draw_default_help() {
    // draw help text
    select_font(FONT_HELPTEXT);
    gfx_print_at(1, BOX_H+HEAD_LINES, "Use the power (\x18) and reset (\x19) buttons to highlight an entry.\n"
												"Long-press reset (1s) or press eject to boot.");
}

void menu_update_timeout(int seconds) {
	select_font(FONT_HELPTEXT);
	gfx_printf_at(1 + strlen(timeout_prompt), TIMEOUT_Y, "%*d", 2, seconds);
}

void menu_clear_timeout(void) {
	gfx_clear(1, TIMEOUT_Y, strlen(timeout_prompt) + 3 + strlen(timeout_prompt_term), 1, config_color_helptext[1]);
}

// called first time, draws the whole message
void menu_draw_timeout(int seconds) {
	menu_clear_timeout();
	// draw timeout text
	if (config_timeout) {
		select_font(FONT_HELPTEXT);
		gfx_printf_at(1, TIMEOUT_Y, "%s%*d%s", timeout_prompt, 2, seconds, timeout_prompt_term);
	}
}

static void menu_draw_help(void) {
	// clear help area
	// do not clear the timeout line
	gfx_clear(0, BOX_H+HEAD_LINES, CONSOLE_COLUMNS, HELP_LINES-1, config_color_helptext[1]);

	if (!config_entries_count) {
		menu_draw_default_help();
		return;
	}

	stanza *sel = &config_entries[menu_selection];
	if (!sel->help_text) {
		menu_draw_default_help();
		return;
	}

	select_font(FONT_HELPTEXT);
	gfx_print_at(1, BOX_H+HEAD_LINES, sel->help_text);
}

void menu_render_logo() {
	console_blit(RESOLUTION_W-gumboot_logo.width-CONSOLE_CHAR_WIDTH, (HEAD_LINES+2) * CONSOLE_CHAR_HEIGHT, gumboot_logo, config_color_normal[1]);
}

void menu_draw_entries_and_help(void) {
	int i;

	for(i=0;i<config_entries_count;i++) {
		rgb c;
		if (i == menu_selection) {
			select_font(FONT_HIGHLIGHT);
			c = config_color_highlight[1];
		} else {
			select_font(FONT_NORMAL);
			c = config_color_normal[1];
		}
		gfx_print_at(1, HEAD_LINES+1+i, config_entries[i].title);
		
		// make a whole bar of highlighted selection / background
		int l = strlen(config_entries[i].title);
		gfx_clear(1 + l, HEAD_LINES+1+i, CONSOLE_COLUMNS-l-2, 1, c);
	}
	
	menu_render_logo();
	
	menu_draw_help();
}

raster *menu_splash;

void menu_init(raster *splash) {
	menu_splash = splash;
	gfx_clear(0, 		  0, CONSOLE_COLUMNS, HELP_LINES, config_color_heading[1]);
	gfx_clear(0, HEAD_LINES, CONSOLE_COLUMNS, CONSOLE_LINES-HELP_LINES-HEAD_LINES, config_color_normal[1]);
	gfx_clear(0, CONSOLE_LINES-HELP_LINES, CONSOLE_COLUMNS, HELP_LINES, config_color_helptext[1]);
	
	// set correct pointers for the logo
	gumboot_logo.width = gumboot_logo_width;
	gumboot_logo.height = gumboot_logo_height;
	gumboot_logo.pixels = gumboot_logo_pixels;
}

int menu_render_splash(raster rst) {
	// blit the pixel data, centered on screen
	console_blit((RESOLUTION_W-rst.width)/2, (RESOLUTION_H-rst.height)/2, rst, config_color_normal[1]);
	return 0;
}
