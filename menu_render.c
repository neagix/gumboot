
#include "menu_render.h"

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

static int help_drawn = 0;

void menu_draw(int seconds, u16 mini_version_major, u16 mini_version_minor) {
    // draw help text
    selected_font_yuv = font_yuv_helptext;
    gfx_print_at(1, BOX_H+3, "Use the power (\x18) and reset (\x19) buttons to highlight an entry.\n"
												"Long-press reset (1s) or press eject to boot.");

	// draw timeout text
	if (seconds != 0)
		gfx_printf_at(1, BOX_H+3+4, "%s%*d%s", timeout_prompt, 2, seconds, timeout_prompt_term);

	selected_font_yuv = font_yuv_heading;
    gfx_print_at((CONSOLE_COLUMNS-strlen(menu_title))/2, 1, menu_title);
    
    // print MINI version
    char buffer[100];
    int l = snprintf(buffer, sizeof(buffer)-1, "Mini version: %d.%0d", mini_version_major, mini_version_minor);
    gfx_print_at(CONSOLE_COLUMNS-l-1, 2, buffer);
   
    selected_font_yuv = font_yuv_normal;
    draw_box_at(0, 3, CONSOLE_COLUMNS, BOX_H);
}

void menu_draw_entries(void) {
	int i;
	u32 **prev = selected_font_yuv;

	for(i=0;i<config_entries_count;i++) {
		rgb c;
		if (i == menu_selection && (selected_font_yuv != font_yuv_highlight)) {
			if (help_drawn || config_entries[i].help_text) {
				// clear help area
				selected_font_yuv = font_yuv_helptext;
				// do not clear the timeout line
				gfx_clear(0, BOX_H+3, CONSOLE_COLUMNS, HELP_LINES-1, config_color_helptext[1]);
			}
			
			if (config_entries[i].help_text) {
				help_drawn = 1;
				
				selected_font_yuv = font_yuv_helptext;
				gfx_print_at(1, BOX_H+3, config_entries[i].help_text);
			}

			selected_font_yuv = font_yuv_highlight;
			c = config_color_highlight[1];
		} else {
			selected_font_yuv = font_yuv_normal;
			c = config_color_normal[1];
		}
		gfx_print_at(1, 4+i, config_entries[i].title);
		
		// make a whole bar of highlighted selection / background
		int l = strlen(config_entries[i].title);
		gfx_clear(1 + l, 4+i, CONSOLE_COLUMNS-l-2, 1, c);
	}

	selected_font_yuv = prev;
}
