
#include "browse.h"
#include "fatfs/ff.h"
#include "malloc.h"
#include "log.h"
#include "powerpc_elf.h"

char *browse_menu_entries[CONSOLE_LINES-HELP_LINES-HEAD_LINES-2];
int browse_menu_entries_count = 0;

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

	if (!is_directory)
		browse_menu_entries[browse_menu_entries_count++] = strdup(name);
	else
		browse_menu_entries[browse_menu_entries_count++] = strcat(name, "/");
}

static void menu_browse_leave(void) {
	// for debugging purposes
	unsigned int i;
	for(i=0;i<sizeof(browse_current_path);i+=4) {
		browse_current_path[0]=DISP_RIGHT;
		browse_current_path[1]=DISP_RIGHT;
		browse_current_path[2]=0x0;
		browse_current_path[3]=DISP_RIGHT;
	}

	menu_selection = old_menu_selection;
	menu_clear_entries();
	menu_draw_entries_and_help();
}

int menu_browse() {
	DIR dirs;
	FILINFO Fno;
	//NOTE: menu entries must have already been free'd on entry

	FRESULT res = f_opendir(&dirs, browse_current_path);
	if (res != FR_OK) {
		log_printf("failed to open directory '%s': %d\n", browse_current_path, res);
		sleep(4);

		menu_browse_leave();
		return (int)res;
	}

	// add first entry to go back one level
	browse_append("..", 0);

	while (((res = f_readdir(&dirs, &Fno)) == FR_OK) && Fno.fname[0]) {
		if (Fno.fattrib & AM_DIR) {
			browse_append(Fno.fname, 1);
		} else {
			browse_append(Fno.fname, 0);
		}
	}
	if (res != FR_OK) {
		log_printf("failed to read directory '%s': %d\n", browse_current_path, res);
		sleep(3);
	}

	// add an extra entry to go back - it uses DISP_LEFT character code point
	browse_append("\x1b Back to menu", 0);

	// draw the new menu
	menu_selection = 0;
	menu_clear_entries();
	menu_draw_entries_and_help();
	
	// return non-zero so that input loop continues
	return 1;
}

static void free_browse_menu() {
	int i;
	// free all browse menu entries
	for(i=0;i<browse_menu_entries_count;i++) {
		free(browse_menu_entries[i]);
	}
	browse_menu_entries_count = 0;
}

int menu_browse_activate(void) {
	// shall we go back? handles first and last menu entries
	if ((menu_selection == 0) || (menu_selection == browse_menu_entries_count-1)) {
		// "0:" for example, or the "go back to menu" entry
		if ((strlen(browse_current_path)<=2) || (menu_selection == browse_menu_entries_count-1)) {
			free_browse_menu();
			// end of line: go back
			menu_browse_leave();

			// return non-zero so that input loop continues
			return 1;
		}
		free_browse_menu();
		
		// find before-last slash in the current path
		// length is above 3 thanks to criteria above
		char *pos = browse_current_path+3, *last_pos = NULL;
		while (1) {
			pos = strchr(pos, '/');
			if (pos)
				last_pos = pos;
			else
				break;
		}
		
		if (last_pos) {
			// cut here
			*last_pos = 0;
		} else
			// truncate to root directory
			browse_current_path[2] = 0;

		return menu_browse();
	}

	// a file was selected, try too boot it
	int cur_len = strlen(browse_current_path);
	char *label = browse_menu_entries[menu_selection];
	int l = strlen(label);

	// is the selection a subdirectory?
	if (label[l-1] == '/') {
		// append subdirectory to the current path
		browse_current_path[cur_len] = '/';
		cur_len++;

		l--;
		memcpy(browse_current_path+cur_len, label, l);
		browse_current_path[cur_len + l] = 0;

		free_browse_menu();
		return menu_browse();
	}

	// previous length + 1 slash + filename + delimiter
	char *elf_fn = malloc(cur_len+l+2);
	memcpy(elf_fn, browse_current_path, cur_len);
	elf_fn[cur_len]='/';
	cur_len++;
	memcpy(elf_fn+cur_len, label, l+1);

	int err = try_boot_file(elf_fn, "");
	free(elf_fn);

	return err;
}
