
#include "menu.h"
#include "string.h"
#include "powerpc.h"
#include "powerpc_elf.h"
#include "log.h"
#include "time.h"
#include "fatfs/ff.h"
#include "menu_render.h"
#include "console_common.h"

void menu_update_timeout(int seconds) {
	select_font(FONT_HELPTEXT);
	gfx_printf_at(1 + strlen(timeout_prompt), BOX_H+3+4, "%*d", 2, seconds);
}

void menu_clear_timeout(void) {
	gfx_clear(2, BOX_H+HEAD_LINES+2, strlen(timeout_prompt) + 2 + strlen(timeout_prompt_term), 1, config_color_helptext[1]);
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

int menu_browse(stanza *sel) {
	DIR dirs;
	FILINFO Fno;
	FATFS fatfs;
	char target[3];
	target[0] = sel->root_pt + '0';
	target[1] = ':';
	target[2] = 0x0;
	FRESULT res = f_mount(&fatfs, target, 1);

	if (res != FR_OK) {
		log_printf("could not open partition %d: %d\n", sel->root_pt, res);
		return (int)res;
	}

	res = f_opendir(&dirs, sel->root);
	if (res != FR_OK) {
		log_printf("failed to open directory %s: %d\n", sel->root, res);
		return (int)res;
	}
	while (((res = f_readdir(&dirs, &Fno)) == FR_OK) && Fno.fname[0]) {
		if (Fno.fattrib & AM_DIR) {
			gfx_printf(" directory: %s\n", Fno.fname);
		} else {
			gfx_printf(" file: %s\n", Fno.fname);
		}
	}
	
	return 0;
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
		// directory browse, uses 'root' to set starting partition and directory
		// otherwise starts from first partition and root directory
		return menu_browse(sel);
	} else if (sel->root) {
		root = sel->root;
		part_no = sel->root_pt;
	} else {
		log_printf("BUG: invalid menu entry\n");
		return -1;
	}
	
	// at this point root must have been setup
	// and we are going to boot a kernel
	FATFS fatfs;
	char target[3];
	target[0] = part_no + '0';
	target[1] = ':';
	target[2] = 0x0;
	FRESULT res = f_mount(&fatfs, target, 1);

	if (res != FR_OK) {
		log_printf("could not open partition %d: %d\n", part_no, res);
		return (int)res;
	}

	char *kernel_fn;
	if (strlen(root))
		// root has always trailing slash, kernel has always no leading slash
		kernel_fn = strcat(root, sel->kernel);
	else {
		kernel_fn = sel->kernel;
	}
	
	// sanity check
	int err = is_valid_elf(kernel_fn);
	if (err) {
		log_printf("not a valid ELF: %s: %d\n", kernel_fn, err);
		return err;
	}

	// clear screen for ease of the eyes
	select_font(FONT_NORMAL);
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
