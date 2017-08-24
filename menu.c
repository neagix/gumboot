
#include "menu.h"
#include "string.h"
#include "powerpc.h"
#include "powerpc_elf.h"
#include "log.h"
#include "time.h"
#include "fatfs/ff.h"
#include "menu_render.h"
#include "console_common.h"
#include "video_low.h"
#include "malloc.h"
#include "browse.h"

void menu_down(void) {
	int max;
	if (browse_buffer) {
		max = browse_menu_entries_count;
	} else {
		max = config_entries_count;
	}
	menu_selection++;
	if (menu_selection == max)
		menu_selection = 0;
	menu_draw_entries_and_help();
}

void menu_up(void) {
	int max;
	if (browse_buffer) {
		max = browse_menu_entries_count;
	} else {
		max = config_entries_count;
	}
	if (menu_selection == 0)
		menu_selection = max-1;
	else
		menu_selection--;
	menu_draw_entries_and_help();
}

int menu_activate(void) {
	int m_err;
	if (browse_buffer) {
		return menu_browse_activate();
	}
	stanza *sel = &config_entries[menu_selection];
	
	if (sel->reboot) {
		VIDEO_Shutdown();
		powerpc_reset();
		return 0;
	}
	if (sel->poweroff) {
		VIDEO_Shutdown();
		powerpc_poweroff();
		return 0;
	}
	
	// if root is set, initialize the corresponding volume
	FATFS fatfs;
	char target[3];
	if (sel->root) {
		memcpy(target, sel->root, 3);
		target[2] = 0;

		FRESULT res = f_mount(&fatfs, target, 1);
		if (res != FR_OK) {
			log_printf("could not mount %s: %d\n", target, res);
			return res;
		}
	}

	//NOTE: fatfs is not unmounted when browsing, re-mounting will cleanup the previous object
	if (sel->browse) {
		old_menu_selection = menu_selection;

		// directory browse, uses 'root' to set starting partition and directory
		// otherwise starts from first partition and root directory
		if (sel->root) {
			memcpy(browse_current_path, sel->root, strlen(sel->root)+1);
		} else {
			// use first logical drive by default
			browse_current_path[0] = '0';
			browse_current_path[1] = ':';
			browse_current_path[2] = 0;
		}

		return menu_browse();
	}

	int err;
	if (!sel->kernel) {
		log_printf("BUG: invalid menu entry\n");
		err = -1;
		goto unmount_and_exit;
	}
	
	// at this point root must have been setup
	// and we are going to boot a kernel

	// root has never trailing slash, kernel has always leading slash
	char *kernel_fn = strcat(sel->root, sel->kernel);
	err = try_boot_file(kernel_fn, sel->kernel_args);
	free(kernel_fn);

unmount_and_exit:
	m_err = f_mount(NULL, target, 0);
	if (m_err) {
		log_printf("failed to unmount: %d\n", m_err);
	}

	return err;
}
