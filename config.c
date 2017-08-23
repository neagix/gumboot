
#include "config.h"
#include "fatfs/ff.h"
#include "video_low.h"
#include "atoi.h"

#ifdef GUMBOOT
#include "malloc.h"
#include "string.h"
//#include "menu.h"
#include "log.h"

#define MAX_LST_SIZE 16*1024

#else
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "lstrender/log_compat.h"

#endif

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
#define ERR_DOUBLE_DEFINITION	0xB0
#define ERR_NO_ROOT				0xC0
#define ERR_INVALID_ROOT		0xD0
#define ERR_INVALID_VIDEO_MODE	0xE0
#define ERR_UNKNOWN_TOKEN		0xF0

// all configuration options
int config_timeout = 0,
	config_default = 0,
	config_nomenu = 0,
	config_entries_count = 0;
char *config_splashimage = NULL;

int config_vmode = -1;

rgb color_default[2] = {
	{.as_rgba = {0xAA, 0xAA, 0xAA, 0xFF}},
	{.as_rgba = {0, 0, 0, 0xFF}}
	};
rgb color_default_invert[2] = {{.as_rgba = {0, 0, 0, 0xFF}}, {.as_rgba = {0xAA, 0xAA, 0xAA, 0xFF}}};

// color configuration
rgb config_color_normal[2] = {
		{.as_rgba = {0xAA, 0xAA, 0xAA, 0xFF}},
		{.as_rgba = {0, 0, 0, 0xFF}}
	},
	config_color_highlight[2] = {
		{.as_rgba = {0xAA, 0xAA, 0xAA, 0xFF}},
		{.as_rgba = {0, 0, 0, 0xFF}}
	},
	config_color_helptext[2] = {
		{.as_rgba = {0xAA, 0xAA, 0xAA, 0xFF}},
		{.as_rgba = {0, 0, 0, 0xFF}}
	},
	config_color_heading[2] = {
		{.as_rgba = {0xAA, 0xAA, 0xAA, 0xFF}},
		{.as_rgba = {0, 0, 0, 0xFF}}
	};

stanza config_entries[MAX_CONFIG_ENTRIES];
static stanza *wip_stanza = NULL;

int process_line(char *line);
int complete_stanza();
char *tokenize(char *line);

#ifdef	GUMBOOT
char *config_load(const char *fname, u32 *read) {
	FRESULT res;
	FIL fd;
	FILINFO stat;

	res = f_stat(DEFAULT_LST, &stat);
	if (res != FR_OK) {
		log_printf("failed to stat %s: %d\n", DEFAULT_LST, res);
		return NULL;
	}

	res = f_open(&fd, DEFAULT_LST, FA_READ);
	if (res != FR_OK) {
		log_printf("failed to open %s: %d\n", DEFAULT_LST, res);
		return NULL;
	}
	
	int fsize = stat.fsize;
	if (fsize > MAX_LST_SIZE) {
		fsize = MAX_LST_SIZE;
		log_printf("truncating %s to %d bytes\n", fname, fsize);
	}
	char *cfg_data = malloc(fsize);
	
	res = f_read(&fd, cfg_data, fsize, read);
	if (res != FR_OK) {
		log_printf("failed to read %s: %d\n", fname, res);
		free(cfg_data);
		f_close(&fd);
		return NULL;
	}
	// terminate string
	cfg_data[*read] = 0;
	f_close(&fd);
	
	return cfg_data;
}
#endif

const char *config_strerr(int err) {
	switch (err) {
		case ERR_MISSING_TOKEN 		:
			return "missing token";
		case ERR_INVALID_NUMBER		:
			return "invalid number";
		case ERR_EXTRA_TOKEN 		:
			return "extra token";
		case ERR_MISSING_TITLE		:
			return "missing title";
		case ERR_UNEXPECTED_TOKEN	:
			return "unexpected token";
		case ERR_NOTHING_TO_DO	 	:
			return "no action specified";
		case	ERR_TOO_MANY_ACTIONS:
			return "too many actions";
		case ERR_TOO_MANY_STANZAS	:
			return "too many stanzas";
		case ERR_INVALID_COLOR		:
			return "invalid color";
		case ERR_TOO_MANY_COLORS	:
			return "too many colors";
		case ERR_DOUBLE_DEFINITION	:
			return "duplicate definition";
		case ERR_NO_ROOT			:
			return "no root specified";
		case ERR_INVALID_ROOT		:
			return "invalid root";
		case ERR_INVALID_VIDEO_MODE	:
			return "invalid video mode";
		case ERR_UNKNOWN_TOKEN		:
			return "unknown token";
	}
	return "???";
}

int config_load_from_buffer(char *cfg_data, u32 read) {
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
			log_printf("error processing line %d: %s\n", line_no, config_strerr(err));
			// in case of error, abort
			return err;
		}
		
		if (start == cfg_data + read) {
			// reached EOF
			break;
		}

		// start looking for next line
		start++;
		last_line = start;
	}

	if (wip_stanza) {
		int err = complete_stanza();
		if (err) {
			log_printf("invalid last menu entry: %d\n", err);
			return err;
		}
	}
	
	if (config_entries_count == 0) {
		log_printf("no config entries defined\n");
		return -1;
	}
	
	if (config_default >= config_entries_count) {
		log_printf("invalid default selected\n");
		config_timeout = 0;
		config_default = 0;
		return -2;
	}
	
	if (config_nomenu && config_timeout) {
		log_printf("cannot use nomenu with a timeout\n");
		config_nomenu = 0;
		return -2;
	}
	
	return 0;
}

int is_whitespace(char c) {
	return ((c == ' ') || (c == '\t'));
}

void trim_right(char *s, char *beginning) {
	s--;
	while (1) {
		if (s == beginning)
			return;
		if (*s == 0x0)
			return;
		if (is_whitespace(*s)) {
			*s = 0x0;
			s--;
		} else
			return;
	}
}

char * strchr2(const char *s, char c1, char c2)
{
	size_t i;
	
	for (i = 0; s[i]; i++) {
		if (s[i] == c1) {
			if (!s[i+1])
				break;
			if (s[i+1] == c2)
				return (char *)s + i;
		}
	}

	return NULL;
}

int parse_timeout(char *s) {
	size_t parsed;
	int val = atoi_base(s, 10, &parsed);
	if (parsed != strlen(s))
		return ERR_INVALID_NUMBER;
	
	config_timeout = val;
	
	return 0;
}

int parse_default(char *s) {
	size_t parsed;
	int val = atoi_base(s, 10, &parsed);
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
	if (wip_stanza->browse)
		actions++;
	if (actions > 1) {
		return ERR_TOO_MANY_ACTIONS;
	}
	
	if (!actions)
		return ERR_NOTHING_TO_DO;

	if (wip_stanza->kernel && (!wip_stanza->root && !wip_stanza->find_args))
		return ERR_NO_ROOT;

	if (wip_stanza->root && wip_stanza->find_args)
		return ERR_DOUBLE_DEFINITION;
	
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

	if (wip_stanza->reboot)
		return ERR_DOUBLE_DEFINITION;

	wip_stanza->reboot = 1;

	return 0;
}
int parse_poweroff() {
	if (!wip_stanza)
		return ERR_UNEXPECTED_TOKEN;

	if (wip_stanza->poweroff)
		return ERR_DOUBLE_DEFINITION;

	wip_stanza->poweroff = 1;

	return 0;
}

int parse_kernel(char *s) {
	if (!wip_stanza)
		return ERR_UNEXPECTED_TOKEN;

	if (wip_stanza->kernel)
		return ERR_DOUBLE_DEFINITION;
	
	if (*s == '/')
		s++;
	if (!*s)
		return ERR_MISSING_TOKEN;
	
	// find arguments (if any)
	char *pos = strchr(s, ' ');
	if (pos) {
		*pos = 0x0;
		pos++;
		wip_stanza->kernel_args = strdup(pos);
	} else {
		wip_stanza->kernel_args = "";
	}
	
	wip_stanza->kernel = strdup(s);
	return 0;
}

int parse_root(char *s) {
	if (!wip_stanza)
		return ERR_UNEXPECTED_TOKEN;

	if (wip_stanza->root)
		return ERR_DOUBLE_DEFINITION;

	if (strncmp(s, "(sd", 3)) {
		return ERR_INVALID_ROOT;
	}
	// get partition number
	s+=3;
	if (strlen(s) < 2)
		return ERR_INVALID_ROOT;
	// check string after the number
	if (s[1] != ')')
		return ERR_INVALID_ROOT;
	u8 n = (u8)(*s-48);
	if (n > 9)
		return ERR_INVALID_ROOT;

	wip_stanza->root_part_no = n;
	
	// skip number and parenthesis
	s++; s++;
	
	if (!*s || (s[0] == '/' && !s[1])) {
		// only partition provided
		wip_stanza->root = "";
		return 0;
	}

	// remove leading slash
	if (*s != '/')
		return ERR_INVALID_ROOT;
	s++;

	// make sure there is a trailing slash
	char *last = s + strlen(s) - 1;
	if (*last != '/') {
		wip_stanza->root = strcat(s, "/");
	} else
		wip_stanza->root = strdup(s);

	return 0;
}

int parse_splashimage(char *s) {
	if (config_splashimage)
		return ERR_DOUBLE_DEFINITION;
	
	config_splashimage = strdup(s);
	return 0;
}

rgb color_to_rgb[16]={
		{.as_rgba = {0, 0, 0, 0xFF}}, 
		{.as_rgba = {0, 0, 0xAA, 0xFF}},
		{.as_rgba = {0, 0xAA, 0, 0xFF}},
		{.as_rgba = {0, 0xAA, 0xAA, 0xFF}},
		{.as_rgba = {0xAA, 0, 0, 0xFF}},
		{.as_rgba = {0xAA, 0, 0xAA, 0xFF}},
		{.as_rgba = {0xAA, 0x55, 0, 0xFF}},
		{.as_rgba = {0xAA, 0xAA, 0xAA, 0xFF}},
		{.as_rgba = {0x55, 0x55, 0x55, 0xFF}},
		{.as_rgba = {0x55, 0x55, 0xFF, 0xFF}},
		{.as_rgba = {0x55, 0xFF, 0x55, 0xFF}},
		{.as_rgba = {0x55, 0xFF, 0xFF, 0xFF}},
		{.as_rgba = {0xFF, 0x55, 0x55, 0xFF}},
		{.as_rgba = {0xFF, 0x55, 0xFF, 0xFF}},
		{.as_rgba = {0xFF, 0xFF, 0x55, 0xFF}},
		{.as_rgba = {0xFF, 0xFF, 0xFF, 0xFF}}
};

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

static int atocolor_half(char *str, rgb *result) {
      if (!strncmp(str, "rgb(", 4)) {
		  char *ptr;
		str+=4;
		  
		// read red component
		ptr = strchr(str, ',');
		if (!ptr)
			return -1;
		*ptr=0;
		ptr++;
		
		size_t parsed;
		int val = atoi_base(str, 10, &parsed);
		if (parsed != strlen(str) || (val > 255))
			return ERR_INVALID_NUMBER;
		result->as_rgba.r = val;

		str = ptr;
		ptr = strchr(ptr, ',');
		if (!ptr)
			return -1;
		*ptr=0;
		ptr++;

		val = atoi_base(str, 10, &parsed);
		if (parsed != strlen(str) || (val > 255))
			return ERR_INVALID_NUMBER;
		result->as_rgba.g = val;

		str = ptr;
		ptr = strchr(ptr, ')');
		if (!ptr)
			return -1;
		*ptr=0;

		val = atoi_base(str, 10, &parsed);
		if (parsed != strlen(str) || (val > 255))
			return ERR_INVALID_NUMBER;
		result->as_rgba.b = val;
		
		return 0;
	  }
	  int i;
      /* Search for the color name.  */
      for (i = 0; i < 16; i++) {
		if (strcmp (color_list[i], str) == 0)
		  {
			*result = color_to_rgb[i];
			return 0;
		  }
	  }
	
	return -1;
}

/* Convert the color name STR into the magical number.  */
static int atocolor(char *str, rgb *result)
{
      char *ptr;

    /* Find the separator.  */
    for (ptr = str; *ptr && *ptr != '/'; ptr++){
		  ;
	}

	/* If not found, return -1.  */
	if (! *ptr)
		return -1;
	/* Terminate the string STR.  */
	*ptr = 0;
	ptr++;
      
	// detect first half
	int err = atocolor_half(str, &result[0]);
	if (err)
		return err;

	// detect second half
	return atocolor_half(ptr, &result[1]);
}

// parse colors second:
// color NORMAL [HIGHLIGHT [HELPTEXT [HEADING]]]
// see http://diddy.boot-land.net/grub4dos/files/commands.htm#color
int parse_color(char *s) {
	char *next_token = tokenize(s);
	
	int err = atocolor(s, (rgb *)&config_color_normal);
	if (err)
		return ERR_INVALID_COLOR;

	if (!next_token)
		return 0;
	s = next_token;
	next_token = tokenize(s);

	err = atocolor(s, (rgb *)&config_color_highlight);
	if (err)
		return ERR_INVALID_COLOR;

	if (!next_token)
		return 0;
	s = next_token;
	next_token = tokenize(s);

	err = atocolor(s, (rgb *)&config_color_helptext);
	if (err)
		return ERR_INVALID_COLOR;

	if (!next_token)
		return 0;
	s = next_token;
	next_token = tokenize(s);

	err = atocolor(s, (rgb *)&config_color_heading);
	if (err)
		return ERR_INVALID_COLOR;
	
	if (next_token)
		return ERR_TOO_MANY_COLORS;

	return 0;
}

int parse_nomenu() {
	if (config_nomenu)
		return ERR_DOUBLE_DEFINITION;

	config_nomenu = 1;
	
	return 0;
}

int parse_browse() {
	if (!wip_stanza)
		return ERR_UNEXPECTED_TOKEN;

	if (wip_stanza->browse)
		return ERR_DOUBLE_DEFINITION;

	wip_stanza->browse = 1;
	return 0;
}

int parse_video(char *s) {
	if (wip_stanza)
		return ERR_UNEXPECTED_TOKEN;

	if (config_vmode != -1)
		return ERR_DOUBLE_DEFINITION;

	if (!strcmp(s, "PROGRESSIVE"))
		config_vmode = VIDEO_640X480_NTSCp_YUV16;
	else if (!strcmp(s, "NTSC"))
		config_vmode = VIDEO_640X480_NTSCi_YUV16;
	else if (!strcmp(s, "PAL50"))
		config_vmode = VIDEO_640X480_PAL50_YUV16;
	else if (!strcmp(s, "PAL60"))
		config_vmode = VIDEO_640X480_PAL60_YUV16;
	else
		return ERR_INVALID_VIDEO_MODE;

	return 0;
}

int parse_find(char *s) {
	if (!wip_stanza)
		return ERR_UNEXPECTED_TOKEN;

	if (wip_stanza->find_args)
		return ERR_DOUBLE_DEFINITION;

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
		if (!is_whitespace(*eot))
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
			if (is_whitespace(*eot))
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
	while (is_whitespace(*line))
		line++;
	if (!*line)
		return 0;
	
	// is this a comment?
	if (*line == '#')
		return 0;
		
	char *token = tokenize(line);
	
	if (0 == strcmp(line, "timeout")) {
		if (token == NULL) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_timeout(token);
	} else if (0 == strcmp(line, "default")) {
		if (token == NULL) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_default(token);
	} else if (0 == strcmp(line, "title")) {
		if (token == NULL) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_title(token);
	} else if (0 == strcmp(line, "splashimage")) {
		if (token == NULL) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_splashimage(token);
	} else if (0 == strcmp(line, "color")) {
		if (token == NULL) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_color(token);
	} else if (0 == strcmp(line, "video")) {
		if (token == NULL) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_video(token);
	} else if (0 == strcmp(line, "kernel")) {
		if (token == NULL) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_kernel(token);
	} else if (0 == strcmp(line, "root")) {
		if (token == NULL) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_root(token);
	} else if (0 == strcmp(line, "find")) {
		if (token == NULL) {
			return ERR_MISSING_TOKEN;
		}
		
		return parse_find(token);
	} else if (0 == strcmp(line, "reboot")) {
		if (token) {
			return ERR_EXTRA_TOKEN;
		}
		
		return parse_reboot();
	} else if (0 == strcmp(line, "boot")) {
		if (token) {
			return ERR_EXTRA_TOKEN;
		}
		
		return parse_boot();
	} else if ((0 == strcmp(line, "poweroff")) || (0 == strcmp(line, "halt"))) {
		if (token) {
			return ERR_EXTRA_TOKEN;
		}
		
		return parse_poweroff();
	} else if ((0 == strcmp(line, "hiddenmenu")) || (0 == strcmp(line, "nomenu"))) {
		if (token) {
			return ERR_EXTRA_TOKEN;
		}
		
		return parse_nomenu();
	} else if (0 == strcmp(line, "browse")) {
		if (token) {
			return ERR_EXTRA_TOKEN;
		}
		
		return parse_browse();
	} else {
		log_printf("unknown token: %s\n", line);
		return ERR_UNKNOWN_TOKEN;
	}
	
	return 0;
}
