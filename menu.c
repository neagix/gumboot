
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

char *browse_buffer = NULL;
int browse_menu_entries[CONSOLE_LINES-HELP_LINES-HEAD_LINES-2];
int browse_buffer_sz = 0, browse_menu_entries_count = 0;

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

void browse_append(const char *name, int is_directory) {
	// stop ingurgitating menu entries
	if (browse_menu_entries_count == sizeof(browse_menu_entries))
		return;

	// get directory name length
	int len = strlen(name);
	// reallocate to make room for trailing slash and delimiter
	int end_offset = browse_buffer_sz;
	browse_buffer_sz+=len+1;
	if (is_directory)
		browse_buffer_sz++;
	browse_buffer = realloc(browse_buffer, browse_buffer_sz);
	// copy the name
	memcpy(browse_buffer+end_offset, name, len);
	if (is_directory) {
		browse_buffer[end_offset+len] = '/';
		browse_buffer[end_offset+len+1] = 0;
	} else {
		browse_buffer[end_offset+len] = 0;
	}
	// store what was the previous end as the start of this new entry
	browse_menu_entries[browse_menu_entries_count++] = end_offset;
}

int menu_browse(stanza *sel) {
	DIR dirs;
	FILINFO Fno;
	FATFS fatfs;
	char target[3], *root;
	u8 part_no;

	if (sel->root) {
		part_no = sel->root_pt;
		root = sel->root;
	} else {
		part_no = 0;
		root = "";
	}
	target[0] = part_no + '0';
	target[1] = ':';
	target[2] = 0x0;
	FRESULT res = f_mount(&fatfs, target, 1);

	if (res != FR_OK) {
		log_printf("could not open partition %d: %d\n", part_no, res);
		return (int)res;
	}

	res = f_opendir(&dirs, root);
	if (res != FR_OK) {
		log_printf("failed to open directory '%s': %d\n", root, res);
		return (int)res;
	}

	// buffer must already be free here

	// add first entry to go back
	browse_buffer = malloc(3);
	memcpy(browse_buffer, "..", 3);
	browse_menu_entries[0] = 0;
	browse_menu_entries_count++;
	browse_buffer_sz+=3;

	while (((res = f_readdir(&dirs, &Fno)) == FR_OK) && Fno.fname[0]) {
		if (Fno.fattrib & AM_DIR) {
			browse_append(Fno.fname, 1);
		} else {
			browse_append(Fno.fname, 0);
		}
	}

	// draw the new menu
	old_menu_selection = menu_selection;
	menu_selection = 0;
	menu_draw_entries_and_help();
	
	// return non-zero so that input loop continues
	return 1;
}

int menu_activate(void) {
	if (browse_buffer) {
		if (menu_selection == 0) {
			// quick path: go back
			menu_selection = old_menu_selection;
			free(browse_buffer);
			browse_menu_entries_count = 0;
			browse_buffer = NULL;
			browse_buffer_sz = 0;
			
			menu_draw_entries_and_help();
			
			// return non-zero so that input loop continues
			return 1;
		}
		
		log_printf("don't know what to do\n");
		
		return 1;
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
		// error message printed by function
		return err;
	}
	
	//TODO: shutdown video to see if kernel 2.6 recovers
	//VIDEO_Shutdown();
	
	err = powerpc_boot_file(part_no, kernel_fn, sel->kernel_args);
	if (err) {
		log_printf("MINI boot failed: %d\n", err);
		return err;
	}
	
	return 0;
}
