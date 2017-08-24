
#include "browse.h"
#include "fatfs/ff.h"
#include "malloc.h"
#include "log.h"
#include "powerpc_elf.h"

char *browse_buffer = NULL;
int browse_menu_entries[CONSOLE_LINES-HELP_LINES-HEAD_LINES-2];
int browse_buffer_sz = 0, browse_menu_entries_count = 0;

char browse_current_path[4096];

// used by FatFS
PARTITION VolToPart[FF_VOLUMES] = {
    {0, 1},     /* "0:" ==> Physical drive 0, 1st partition */
    {0, 2},     /* "1:" ==> Physical drive 0, 2nd partition */
    {0, 3},     /* "2:" ==> Physical drive 0, 3rd partition */
    {0, 4}      /* "3:" ==> Physical drive 0, 4th partition */
};

static void browse_append(const char *name, int is_directory) {
	// stop ingurgitating menu entries
	if (browse_menu_entries_count == sizeof(browse_menu_entries))
		return;

	// get directory name length
	int len = strlen(name);
	// reallocate to make room for trailing slash and delimiter
	int end_offset = browse_buffer_sz;
	// add one for the 0x0 delimiter
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
	browse_buffer_sz = 3;
	browse_buffer = malloc(browse_buffer_sz);
	memcpy(browse_buffer, "..", browse_buffer_sz);
	browse_menu_entries[0] = 0;
	browse_menu_entries_count = 1;

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

int menu_browse_activate(void) {
	// shall we go back?
	if (menu_selection == 0) {
		free_browse_menu();

		// "0:/" for example
		if (strlen(browse_current_path)<=2) {
			// end of line: go back
			menu_selection = old_menu_selection;
			menu_clear_entries();
			menu_draw_entries_and_help();

			// return non-zero so that input loop continues
			return 1;
		}

		// find before-last slash in the current path
		int i;
		for(i=strlen(browse_current_path);i>2;i--) {
			if (browse_current_path[i] == '/') {
				// cut here
				browse_current_path[i] = 0;

				return menu_browse();
			}
		}
		
		// truncate to root directory
		browse_current_path[2] = 0;
		return menu_browse();
	}

	// is the selection a subdirectory?
	char *label = browse_buffer + browse_menu_entries[menu_selection];
	int l = strlen(label);
	if (label[l-1] == '/') {
		// append subdirectory to the current path without trailing slash
		int cur_len = strlen(browse_current_path);
		browse_current_path[cur_len] = '/';
		memcpy(browse_current_path+cur_len+1, label, l-1);
		browse_current_path[cur_len+1+l] = 0;
		free_browse_menu();
		
		return menu_browse();
	}
	
	// a file was selected, try too boot it
	l = strlen(browse_current_path);
	browse_current_path[l] = '/';
	browse_current_path[l+1] = 0;
	char *elf_fn = strcat(browse_current_path, label);
	int err = try_boot_file(elf_fn, "");
	free(elf_fn);
	return err;
}
