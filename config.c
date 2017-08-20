
#include "config.h"
#include "console.h"
#include "ff.h"
#include "malloc.h"
#include "string.h"

#define DEFAULT_LST "gumboot.lst"
#define MAX_LST_SIZE 16*1024

// data read from .lst file
int config_timeout = 15;

int process_line(char *line);

void config_load(void) {
	FRESULT res;
	FATFS fatfs;
	FIL fd;
	FILINFO stat;
	u32 read;
	
	res = f_mount(0, &fatfs);
	if (res != FR_OK) {
		gfx_printf("failed to mount volume: %d\n", res);
		return;
	}
	
	res = f_stat(DEFAULT_LST, &stat);
	if (res != FR_OK) {
		gfx_printf("failed to stat %s: %d\n", DEFAULT_LST, res);
		return;
	}

	res = f_open(&fd, DEFAULT_LST, FA_READ);
	if (res != FR_OK) {
		gfx_printf("failed to open %s: %d\n", DEFAULT_LST, res);
		return;
	}
	
	int fsize = stat.fsize;
	if (fsize > MAX_LST_SIZE) {
		fsize = MAX_LST_SIZE;
		gfx_printf("truncating %s to %d bytes\n", DEFAULT_LST, fsize);
	}
	char *cfg_data = malloc(fsize);
	
	res = f_read(&fd, cfg_data, fsize, &read);
	if (res != FR_OK) {
		gfx_printf("failed to read %s: %d\n", DEFAULT_LST, res);
		free(cfg_data);
		return;
	}
	// terminate string
	cfg_data[read] = 0;
	
	char *start = cfg_data, *last_line = cfg_data;
	while (1) {
		// grab one line from the input
		while (*start != '\n' && *start != 0x0) {
			start++;
		}
		*start = 0;
		
		if (process_line(last_line)) {
			// in case of error, abort
			free(cfg_data);
			return;
		}
		
		if (start == cfg_data + read) {
			// reached EOF
			break;
		}

		// start looking for next line
		start++;
		last_line = start;
	}
	free(cfg_data);
	
	gfx_printf("configuration loaded successfully\n");
}

#define ERR_MISSING_TOKEN 0x10

int process_line(char *line) {
	//gfx_printf("first line: %s\n", last_line);
	
	// skip whitespace
	while ((*line == ' ') || (*line == '\t'))
		line++;
	if (!*line)
		return 0;
	
	// is this a comment?
	if (*line == '#')
		return 0;
		
	// find next space or end of line
	char *eot = line;
	int eol_reached = 0;
	while (1) {
		if (!*eot) {
			eol_reached = 1;
			break;
		}
		if ((*eot != ' ') && (*eot != '\t'))
			eot++;
		else {
			*eot = 0x0;
			eot++;
			break;
		}
	}
	
	// skip some more leading whitespaces
	if (!eol_reached) {
		while (1) {
			if (!*eot) {
				break;
			}
			if ((*eot == ' ') || (*eot == '\t'))
				eot++;
			else
				break;
		}
	}
	
	if (0 == strcmp(line, "timeout")) {
		if (eol_reached) {
			return ERR_MISSING_TOKEN;
		}
		
		// parse next token
		gfx_printf("token for %s is: '%s'\n", line, eot);
	} else if (0 == strcmp(line, "default")) {
		if (eol_reached) {
			return ERR_MISSING_TOKEN;
		}
		
		// parse next token
		gfx_printf("token for %s is: '%s'\n", line, eot);
	} else if (0 == strcmp(line, "title")) {
		if (eol_reached) {
			return ERR_MISSING_TOKEN;
		}
		
		// parse next token
		gfx_printf("token for %s is: '%s'\n", line, eot);
	} else {
		gfx_printf("unknown token: %s\n", line);
	}
	
//	if (eol_reached)
//		return 0;
	
	return 0;
}
