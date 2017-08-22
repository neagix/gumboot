
#include "menu.h"
#include "string.h"
#include "powerpc.h"
#include "powerpc_elf.h"
#include "log.h"
#include "time.h"
#include "filesystem.h"
#include "menu_render.h"

void menu_update_timeout(int seconds) {
	u32 **prev = NULL;
	if (selected_font_yuv != font_yuv_helptext) {
		prev = selected_font_yuv;
		selected_font_yuv = font_yuv_helptext;
	}
	gfx_printf_at(1 + strlen(timeout_prompt), BOX_H+3+4, "%*d", 2, seconds);
	if (prev)
		selected_font_yuv = prev;
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

int menu_activate(void) {
	stanza *sel = &config_entries[menu_selection];
	
	if (sel->reboot) {
		powerpc_reset();
		return 0;
	}
	if (sel->poweroff) {
		powerpc_poweroff();
		return 0;
	}
	
	u8 part_no = 0xFF;
	char *root = NULL;
	
	// root setup
	if (sel->find_args) {
		//TODO: find it!
	} else if (sel->browse) {
		//TODO: directory browse, use root if any otherwise first partition
		
		return 0;
	} else if (sel->root) {
		root = sel->root;
		part_no = sel->root_part_no;
	} else {
		log_printf("BUG: invalid menu entry\n");
		return -1;
	}
	
	// at this point root must have been setup
	// and we are going to boot a kernel
	int err = fs_open(part_no);
	if (err) {
		log_printf("could not open partition %d: %d\n", part_no, err);
		return err;
	}

	char *kernel_fn;
	if (strlen(root))
		// root has always trailing slash, kernel has always no leading slash
		kernel_fn = strcat(root, sel->kernel);
	else {
		kernel_fn = sel->kernel;
	}
	
	// sanity check
	err = is_valid_elf(kernel_fn);
	if (err) {
		log_printf("not a valid ELF: %s: %d\n", kernel_fn, err);
		return err;
	}

	// clear screen for ease of the eyes
	selected_font_yuv = font_yuv_normal;
	console_clear();
	
	if (strlen(sel->kernel_args))
		gfx_printf("Booting (sd%d)/%s... [%s]", part_no, kernel_fn, sel->kernel_args);
	else
		gfx_printf("Booting (sd%d)/%s...", part_no, kernel_fn);

	err = powerpc_boot_file(kernel_fn, sel->kernel_args);
	if (err) {
		//TODO: redraw menu
		log_printf("could not boot kernel %s: %d\n", kernel_fn, err);
		return err;
	}
	
	return 0;
}
