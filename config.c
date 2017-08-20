
#include "config.h"
#include "console.h"
#include "ff.h"
#include "malloc.h"
#include "string.h"
#include "atoi.h"
#include "menu.h"

#define DEFAULT_LST "gumboot.lst"
#define MAX_LST_SIZE 16*1024

#define ERR_MISSING_TOKEN 		0x10
#define ERR_INVALID_NUMBER		0x20
#define ERR_EXTRA_TOKEN 		0x30
#define ERR_MISSING_TITLE		0x40
#define ERR_UNEXPECTED_TOKEN	0x50
#define ERR_NOTHING_TO_DO	 	0x60
#define	ERR_TOO_MANY_ACTIONS	0x70
#define ERR_TOO_MANY_STANZAS	0x80
#define ERR_INVALID_COLOR		0x90
#define ERR_TOO_MANY_COLORS		0xA0

// all configuration options
int config_timeout = 0,
	config_default = 0,
	config_hidden_menu = 0,
	config_entries_count = 0;
char *config_splashimage = NULL;

#define COLOR_STATE_DEFAULT	0xAAAAAA
#define COLOR_STATE_NORMAL	COLOR_STATE_DEFAULT
#define COLOR_STATE_HIGHLIGHT 0xAAAAAA00000000ULL
#define COLOR_STATE_HELPTEXT	COLOR_STATE_DEFAULT
#define COLOR_STATE_HEADING		COLOR_STATE_DEFAULT

// color configuration
unsigned long long config_color_normal = COLOR_STATE_NORMAL,
	config_color_highlight = COLOR_STATE_HIGHLIGHT,
	config_color_helptext = COLOR_STATE_HELPTEXT,
	config_color_heading = COLOR_STATE_HEADING;

stanza config_entries[MAX_CONFIG_ENTRIES];
static stanza *wip_stanza = NULL;

int process_line(char *line);
int complete_stanza();
char *tokenize(char *line);

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
	int line_no = 0;
	while (1) {
		line_no++;
		// grab one line from the input
		while (*start != '\n' && *start != 0x0) {
			start++;
		}
		*start = 0;
		
		int err = process_line(last_line);
		if (err) {
			gfx_printf("error processing line %d: %d\n", line_no, err);
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
	

	if (wip_stanza) {
		int err = complete_stanza();
		if (err)
			gfx_printf("invalid last menu entry: %d\n", err);
	}
	
	if (config_entries_count == 0) {
		gfx_printf("no config entries defined\n");
		return;
	}
	
	if (config_default >= config_entries_count) {
		gfx_printf("invalid default selected\n");
		config_timeout = 0;
		config_default = 0;
		return;
	}
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

int parse_timeout(char *s) {
	size_t parsed;
	int val = atoi(s, 10, &parsed);
	if (parsed != strlen(s))
		return ERR_INVALID_NUMBER;
	
	config_timeout = val;
	
	return 0;
}

int parse_default(char *s) {
	size_t parsed;
	int val = atoi(s, 10, &parsed);
	if (!parsed || (parsed != strlen(s)))
		return ERR_INVALID_NUMBER;
	
	// will be validated later on
	config_default = val;
	
	return 0;
}

int complete_stanza() {
	if (!wip_stanza->title)
		return ERR_MISSING_TITLE;
		
	int actions = 0;
	if (wip_stanza->kernel)
		actions++;
	if (wip_stanza->reboot)
		actions++;
	if (wip_stanza->poweroff)
		actions++;
	if (actions > 1) {
		gfx_printf("completing stanza with %d actions: %p %d %d\n", actions, wip_stanza->kernel, wip_stanza->reboot, wip_stanza->poweroff);
		
		return ERR_TOO_MANY_ACTIONS;
	}
	
	if (!actions && !wip_stanza->save_default)
		return ERR_NOTHING_TO_DO;
	
	// all good, keep track of this stanza
	config_entries_count++;
	wip_stanza = NULL;
	
	return 0;
}

int parse_title(char *s) {
	if (wip_stanza) {
		int err = complete_stanza();
		if (err)
			return err;
	}
	
	if (config_entries_count == MAX_CONFIG_ENTRIES)
		return ERR_TOO_MANY_STANZAS;
	
	// titles define new stanzas as well
	wip_stanza = &config_entries[config_entries_count];
	memset(wip_stanza, sizeof(stanza), 0);
	
	// find first occurrency of '\n'
	char *pos = strchr2(s, '\\', 'n');
	if (pos) {
		// copy first chunk as title
		*pos = 0x0;
		pos+=2;
		
		if (*pos) {
			wip_stanza->help_text = malloc(strlen(pos));
			char *dest = wip_stanza->help_text;
			
			while (1) {
				char *nl = strchr2(pos, '\\', 'n');
				if (!nl) {
					int n = strlen(pos);
					memcpy(dest, pos, n);
					dest+=n;
					break;
				}
				int n = nl-pos;
				memcpy(dest, pos, n);
				dest+=n;
				// add a proper newline
				*dest = '\n';
				dest++;
					
				pos = nl+2;
				if (!*pos)
					break;
			}
			*dest = 0x0;
		}
	}

	wip_stanza->title = strdup(s);
	
	
	return 0;
}

int parse_boot() {
	if (!wip_stanza)
		return ERR_UNEXPECTED_TOKEN;

	return complete_stanza();
}
int parse_reboot() {
	if (!wip_stanza)
		return ERR_UNEXPECTED_TOKEN;
	wip_stanza->reboot = 1;

	return 0;
}
int parse_poweroff() {
	if (!wip_stanza)
		return ERR_UNEXPECTED_TOKEN;
	wip_stanza->poweroff = 1;

	return 0;
}

int parse_savedefault(char *s) {
	if (!wip_stanza)
		return ERR_UNEXPECTED_TOKEN;
	
	if (*s) {
		size_t parsed;
		int val = atoi(s, 10, &parsed);
		if (!parsed || (parsed != strlen(s)))
			return ERR_INVALID_NUMBER;
		wip_stanza->selected_default = val;
	} else {
		wip_stanza->save_default = 1;
		wip_stanza->selected_default = -1; /* self */
	}

	return 0;
}

int parse_kernel(char *s) {
	if (!wip_stanza)
		return ERR_UNEXPECTED_TOKEN;
		
	wip_stanza->kernel = strdup(s);
	return 0;
}

int parse_root(char *s) {
	if (!wip_stanza)
		return ERR_UNEXPECTED_TOKEN;
		
	wip_stanza->root = strdup(s);
	return 0;
}

int parse_splashimage(char *s) {
	config_splashimage = strdup(s);
	return 0;
}

// from grub4dos
unsigned long long color_4_to_32 (unsigned char color4)
{
    switch (color4)
    {
	case 0x00: return 0;
	case 0x01: return 0x0000AA;
	case 0x02: return 0x00AA00;
	case 0x03: return 0x00AAAA;
	case 0x04: return 0xAA0000;
	case 0x05: return 0xAA00AA;
	case 0x06: return 0xAA5500;
	case 0x07: return 0xAAAAAA;
	case 0x08: return 0x555555;
	case 0x09: return 0x5555FF;
	case 0x0A: return 0x55FF55;
	case 0x0B: return 0x55FFFF;
	case 0x0C: return 0xFF5555;
	case 0x0D: return 0xFF55FF;
	case 0x0E: return 0xFFFF55;
	case 0x0F: return 0xFFFFFF;
	default: return 0;
    }
}

unsigned long long color_8_to_64 (unsigned char color8)
{
    return (color_4_to_32 (color8 >> 4) << 32) | color_4_to_32 (color8 & 15);
}

static char *color_list[16] =
{
    "black",
    "blue",
    "green",
    "cyan",
    "red",
    "magenta",
    "brown",
    "light-gray",
    "dark-gray",
    "light-blue",
    "light-green",
    "light-cyan",
    "light-red",
    "light-magenta",
    "yellow",
    "white"
};

/* Convert the color name STR into the magical number.  */
static int atocolor(char *str)
{
      char *ptr;
      int i;
      int color = 0;
      
      /* Find the separator.  */
      for (ptr = str; *ptr && *ptr != '/'; ptr++){
		  ;
	  }

      /* If not found, return -1.  */
      if (! *ptr)
		return -1;

      /* Terminate the string STR.  */
      *ptr = 0;
      /* Search for the color name.  */
      for (i = 0; i < 16; i++) {
		if (strcmp (color_list[i], str) == 0)
		  {
			color |= i;
			break;
		  }
	  }
      if (i == 16)
		return -1;

      str = ptr+1;

      /* Search for the color name.  */      
      for (i = 0; i < 16; i++) {
		if (strcmp (color_list[i], str) == 0)
		  {
			color |= i << 4;
			break;
		  }
	  }

      if (i == 16)
		return -1;

	return color;
}

// parse colors second:
// color NORMAL [HIGHLIGHT [HELPTEXT [HEADING]]]
// see http://diddy.boot-land.net/grub4dos/files/commands.htm#color
int parse_color(char *s) {
	char *next_token = tokenize(s);
	int color = atocolor(s);
	if (color == -1)
		return ERR_INVALID_COLOR;
	config_color_normal = color;

	if (!next_token)
		return 0;
	s = next_token;
	next_token = tokenize(s);

	color = atocolor(s);
	if (color == -1)
		return ERR_INVALID_COLOR;
	config_color_highlight = color;

	if (!next_token)
		return 0;
	s = next_token;
	next_token = tokenize(s);

	color = atocolor(s);
	if (color == -1)
		return ERR_INVALID_COLOR;
	config_color_helptext = color;

	if (!next_token)
		return 0;
	s = next_token;
	next_token = tokenize(s);

	color = atocolor(s);
	if (color == -1)
		return ERR_INVALID_COLOR;
	config_color_heading = color;
	
	if (next_token)
		return ERR_TOO_MANY_COLORS;

	return 0;
}

int parse_hiddenmenu() {
	config_hidden_menu = 1;
	
	return 0;
}

int parse_find(char *s) {
	if (!wip_stanza)
		return ERR_UNEXPECTED_TOKEN;
		
	wip_stanza->find_args = strdup(s);
	return 0;
}

// tokenize will set 0x0 at first whitespace in 's',
// and return the address of the next token (skipping whitespaces)
// or NULL if no other token is available
char *tokenize(char *line) {
	// find next space or end of line
	char *eot = line;
	while (1) {
		if (!*eot) {
			return NULL;
		}
		if (!isspace(*eot))
			eot++;
		else {
			*eot = 0x0;
			eot++;
			break;
		}
	}
	
	// skip some more leading whitespaces for the next token
	if (eot != NULL) {
		while (1) {
			if (!*eot) {
				eot = NULL;
				break;
			}
			if (isspace(*eot))
				eot++;
			else
				/* good token */
				break;
		}
	}

	return eot;	
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
		
	char *eot = tokenize(line);
	int eol_reached = (eot == NULL);
	if (eol_reached)
		eot = "";
	
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
	} else if (0 == strcmp(line, "hiddenmenu")) {
		if (!eol_reached) {
			return ERR_EXTRA_TOKEN;
		}
		
		return parse_hiddenmenu();
	} else {
		gfx_printf("unknown token: %s\n", line);
	}
	
	return 0;
}
