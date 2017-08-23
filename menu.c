
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

char browse_current_path[4096];

int menu_browse() {
	DIR dirs;
	FILINFO Fno;

	FRESULT res = f_opendir(&dirs, browse_current_path);
	if (res != FR_OK) {
		log_printf("failed to open directory '%s': %d\n", browse_current_path, res);
		return (int)res;
	}

	//NOTE: buffer must already be free on entry

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
	menu_selection = 0;
	menu_clear_entries();
	menu_draw_entries_and_help();
	
	// return non-zero so that input loop continues
	return 1;
}

static void free_browse_menu() {
	// free all browse menu entries
	free(browse_buffer);
	browse_menu_entries_count = 0;
	browse_buffer = NULL;
	browse_buffer_sz = 0;
}

int menu_activate(void) {
	if (browse_buffer) {
		// shall we go back?
		if (menu_selection == 0) {
			free_browse_menu();

			// "0:/" for example
			if (strlen(browse_current_path)<=3) {
				// end of line: go back
				menu_selection = old_menu_selection;
				menu_clear_entries();
				menu_draw_entries_and_help();

				// return non-zero so that input loop continues
				return 1;
			}

			// find before-last slash in the current path
			int i;
			for(i=strlen(browse_current_path)-2;i>2;i++) {
				if (browse_current_path[i] == '/') {
					// cut here
					browse_current_path[i] = 0;

					return menu_browse();
				}
			}
			
			// truncate to root directory
			browse_current_path[3] = 0;
			return menu_browse();
		}

		// is the selection a subdirectory?
		char *label = browse_buffer + browse_menu_entries[menu_selection];
		int l = strlen(label);
		if (label[l-1] == '/') {
			// append subdirectory to the current path
			memcpy(browse_current_path+strlen(browse_current_path), label, l+1);
			free_browse_menu();
			
			return menu_browse();
		}
		
		log_printf("selected a file\n");
		
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
		old_menu_selection = menu_selection;

		// directory browse, uses 'root' to set starting partition and directory
		// otherwise starts from first partition and root directory
		if (sel->root) {
			memcpy(browse_current_path, sel->root, strlen(sel->root)+1);
		} else {
			// use first logical drive by default
			browse_current_path[0] = '0';
			browse_current_path[1] = ':';
			browse_current_path[2] = '/';
			browse_current_path[3] = 0;
		}

		return menu_browse();
	} else if (sel->root) {
		root = sel->root;
	} else {
		log_printf("BUG: invalid menu entry\n");
		return -1;
	}
	
	// at this point root must have been setup
	// and we are going to boot a kernel

	// root has always trailing slash, kernel has always no leading slash
	char *kernel_fn = strcat(root, sel->kernel);
	
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
