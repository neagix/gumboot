
#include "menu.h"
#include "string.h"
#include "powerpc.h"
#include "powerpc_elf.h"
#include "log.h"
#include "time.h"

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

#define BOX_H (CONSOLE_LINES-4-HELP_LINES)

void menu_draw(int seconds) {
    // draw help text
    selected_font_yuv = font_yuv_helptext;
    gfx_print_at(2, BOX_H+3, "Use the power (\x18) and reset (\x19) buttons to highlight an entry.\n"
												"Long-press (2s) power or press eject to boot.");

	// draw timeout text
	if (seconds != 0)
		gfx_printf_at(2, BOX_H+3+3, "%s%*d%s", timeout_prompt, 2, seconds, timeout_prompt_term);

	selected_font_yuv = font_yuv_heading;
    gfx_print_at((CONSOLE_COLUMNS-strlen(menu_title))/2, 1, menu_title);
   
    selected_font_yuv = font_yuv_normal;
    draw_box_at(0, 3, CONSOLE_COLUMNS, BOX_H);
}

void menu_draw_entries(void) {
	int i;
	u32 **prev = NULL;
	for(i=0;i<config_entries_count;i++) {
		rgb c;
		if (i == menu_selection && (selected_font_yuv != font_yuv_highlight)) {
			prev = selected_font_yuv;
			selected_font_yuv = font_yuv_highlight;
			c = config_color_highlight[1];
		} else {
			c = config_color_normal[1];
		}
		gfx_print_at(1, 4+i, config_entries[i].title);

		// make a whole bar of highlighted selection
		int l = strlen(config_entries[i].title);
		gfx_clear(1 + l, 4+i, CONSOLE_COLUMNS-l-2, 1, c);

		if (prev) {
			selected_font_yuv = prev;
			prev = NULL;
		}
	}
}

void menu_update_timeout(int seconds) {
	u32 **prev = NULL;
	if (selected_font_yuv != font_yuv_helptext) {
		prev = selected_font_yuv;
		selected_font_yuv = font_yuv_helptext;
	}
	gfx_printf_at(2 + strlen(timeout_prompt), BOX_H+3+3, "%*d", 2, seconds);
	if (prev) {
		selected_font_yuv = prev;
	}
}

void menu_clear_timeout(void) {
	gfx_clear(2, BOX_H+3+3, strlen(timeout_prompt) + 2 + strlen(timeout_prompt_term), 1, config_color_helptext[1]);
}

void menu_down(void) {
	menu_selection++;
	if (menu_selection == config_entries_count)
		menu_selection = 0;
	menu_draw_entries();
}

void menu_up(void) {
	if (menu_selection == 0)
		menu_selection = config_entries_count-1;
	else
		menu_selection--;
	menu_draw_entries();
}

void menu_activate(void) {
	stanza *sel = &config_entries[menu_selection];
	
	if (sel->reboot) {
		powerpc_reset();
		return;
	}
	if (sel->poweroff) {
		powerpc_poweroff();
		return;
	}
	
	u8 part_no = 0xFF;
	char *root = NULL;
	
	// root setup
	if (sel->find_args) {
		//TODO: find it!
	} else if (sel->browse) {
		//TODO: directory browse, use root if any otherwise first partition
		
		return;
	} else if (sel->root) {
		root = sel->root;
		part_no = sel->root_part_no;
	} else {
		// just a save default action
		if (!sel->save_default) {
			log_printf("BUG: invalid menu entry\n");
			return;
		}
		//TODO: save the default
		
		return;
	}
	
	// at this point root must have been setup
	// and we are going to boot a kernel
	int err = config_open_fs(part_no);
	if (err) {
		log_printf("could not open partition %d: %d\n", part_no, err);
		return;
	}

	char *kernel_fn;
	if (strlen(root))
		// root has always trailing slash, kernel has always no leading slash
		kernel_fn = strcat(root, sel->kernel);
	else {
		kernel_fn = sel->kernel;
	}
	
	log_printf("booting in 30s: '%s'\n", kernel_fn);
	sleep(30);
	
	// sanity check
	err = is_valid_elf(kernel_fn);
	if (err) {
		log_printf("not a valid ELF: %s: %d\n", kernel_fn, err);
		return;
	}
	
	err = powerpc_boot_file(kernel_fn);
	if (err) {
		log_printf("could not boot kernel %s: %d\n", kernel_fn, err);
		return;
	}
	
}
