
#include "config.h"
#include "console.h"
#include "ff.h"
#include "malloc.h"
#include "string.h"
#include "atoi.h"

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

int isspace(char c) {
	return ((c == ' ') || (c == '\t'));
}

void trim_right(char *s, char *beginning) {
	s--;
	while (1) {
		if (s == beginning)
			return;
		if (*s == 0x0)
			return;
		if (isspace(*s)) {
			*s = 0x0;
			s--;
		} else
			return;
	}
}

#define ERR_MISSING_TOKEN 	0x10
#define ERR_INVALID_NUMBER	0x20
#define ERR_EXTRA_TOKEN 	0x30

int parse_timeout(char *s) {
	size_t parsed;
	int val = atoi(s, 10, &parsed);
	if (parsed != strlen(s))
		return ERR_INVALID_NUMBER;
	
	gfx_printf("timeout will be %d\n", val);
	
	return 0;
}

int parse_default(char *s) {
	size_t parsed;
	int val = atoi(s, 10, &parsed);
	if (!parsed || (parsed != strlen(s)))
		return ERR_INVALID_NUMBER;
	
	gfx_printf("default will be %d\n", val);
	
	return 0;
}

int parse_title(char *s) {
	gfx_printf("title will be %s\n", s);
	
	return 0;
}

int parse_boot() {
	return 0;
}
int parse_reboot() {
	return 0;
}
int parse_poweroff() {
	return 0;
}

int parse_savedefault(char *s) {
	return 0;
}

int parse_kernel(char *s) {
	return 0;
}

int parse_root(char *s) {
	return 0;
}

int parse_splashimage(char *s) {
	return 0;
}

int parse_color(char *s) {
	return 0;
}

int parse_find(char *s) {
	return 0;
}

int process_line(char *line) {
	// skip whitespace
	while (isspace(*line))
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
		if (!isspace(*eot))
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
			if (isspace(*eot))
				eot++;
			else
				break;
		}
	}
	
	if (0 == strcmp(line, "timeout")) {
		if (eol_reached) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_timeout(eot);
	} else if (0 == strcmp(line, "default")) {
		if (eol_reached) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_default(eot);
	} else if (0 == strcmp(line, "title")) {
		if (eol_reached) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_title(eot);
	} else if (0 == strcmp(line, "savedefault")) {
		// extra argument is optional
		return parse_savedefault(eot);
	} else if (0 == strcmp(line, "splashimage")) {
		if (eol_reached) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_splashimage(eot);
	} else if (0 == strcmp(line, "color")) {
		if (eol_reached) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_color(eot);
	} else if (0 == strcmp(line, "kernel")) {
		if (eol_reached) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_kernel(eot);
	} else if (0 == strcmp(line, "root")) {
		if (eol_reached) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_root(eot);
	} else if (0 == strcmp(line, "find")) {
		if (eol_reached) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_find(eot);
	} else if (0 == strcmp(line, "reboot")) {
		if (!eol_reached) {
			return ERR_EXTRA_TOKEN;
		}
		
		return parse_reboot();
	} else if (0 == strcmp(line, "boot")) {
		if (!eol_reached) {
			return ERR_EXTRA_TOKEN;
		}
		
		return parse_boot();
	} else if ((0 == strcmp(line, "poweroff")) || (0 == strcmp(line, "halt"))) {
		if (!eol_reached) {
			return ERR_EXTRA_TOKEN;
		}
		
		return parse_poweroff();
	} else {
		gfx_printf("unknown token: %s\n", line);
	}
	
//	if (eol_reached)
//		return 0;
	
	return 0;
}
