
#include "config.h"
#include "ff.h"
#include "malloc.h"
#include "string.h"
#include "atoi.h"
#include "menu.h"
#include "log.h"
#include "video_low.h"

#define DEFAULT_LST "gumboot/gumboot.lst"
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
#define ERR_DOUBLE_DEFINITION	0xB0
#define ERR_NO_ROOT				0xC0
#define ERR_INVALID_ROOT		0xD0
#define ERR_INVALID_VIDEO_MODE	0xE0

// all configuration options
int config_timeout = 0,
	config_default = 0,
	config_nomenu = 0,
	config_entries_count = 0;
char *config_splashimage = NULL;

//TODO: add support for this setting
int config_vmode = -1;

rgb color_default[2] = {{0xAA, 0xAA, 0xAA}, {0,0,0}};
rgb color_default_invert[2] = {{0,0,0}, {0xAA, 0xAA, 0xAA}};

// color configuration
rgb config_color_normal[2] = {{0xAA, 0xAA, 0xAA}, {0,0,0}},
	config_color_highlight[2] = {{0,0,0}, {0xAA, 0xAA, 0xAA}},
	config_color_helptext[2] = {{0xAA, 0xAA, 0xAA}, {0,0,0}},
	config_color_heading[2] = {{0xAA, 0xAA, 0xAA}, {0,0,0}};

stanza config_entries[MAX_CONFIG_ENTRIES];
static stanza *wip_stanza = NULL;

int process_line(char *line);
int complete_stanza();
char *tokenize(char *line);

static u8 open_part_no = 0xFF;
static FATFS fatfs;

int config_open_fs(u8 part_no) {
	if (part_no == open_part_no)
		return FR_OK;

	FRESULT res = f_mount(part_no, &fatfs);
	if (res != FR_OK) {
		return (int)res;
	}
	open_part_no = part_no;
	return FR_OK;
}

int config_load(void) {
	FRESULT res;
	FIL fd;
	FILINFO stat;
	u32 read;

	int err = config_open_fs(0);
	if (err) {
		log_printf("failed to mount volume: %d\n", err);
		return err;
	}

	res = f_stat(DEFAULT_LST, &stat);
	if (res != FR_OK) {
		log_printf("failed to stat %s: %d\n", DEFAULT_LST, res);
		return res;
	}

	res = f_open(&fd, DEFAULT_LST, FA_READ);
	if (res != FR_OK) {
		log_printf("failed to open %s: %d\n", DEFAULT_LST, res);
		return res;
	}
	
	int fsize = stat.fsize;
	if (fsize > MAX_LST_SIZE) {
		fsize = MAX_LST_SIZE;
		log_printf("truncating %s to %d bytes\n", DEFAULT_LST, fsize);
	}
	char *cfg_data = malloc(fsize);
	
	res = f_read(&fd, cfg_data, fsize, &read);
	if (res != FR_OK) {
		log_printf("failed to read %s: %d\n", DEFAULT_LST, res);
		free(cfg_data);
		f_close(&fd);
		return res;
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
			log_printf("error processing line %d: %d\n", line_no, err);
			// in case of error, abort
			free(cfg_data);
			f_close(&fd);
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
	free(cfg_data);
	f_close(&fd);

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
		{0,0,0}, 
		{0,0,0xAA}, 
		{0,0xAA,0}, 
		{0,0xAA,0xAA}, 
		{0xAA,0,0}, 
		{0xAA,0,0xAA}, 
		{0xAA,0x55,0}, 
		{0xAA,0xAA,0xAA}, 
		{0x55,0x55,0x55}, 
		{0x55,0x55,0xFF}, 
		{0x55,0xFF,0x55}, 
		{0x55,0xFF,0xFF}, 
		{0xFF,0x55,0x55}, 
		{0xFF,0x55,0xFF}, 
		{0xFF,0xFF,0x55}, 
		{0xFF,0xFF,0xFF}
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

/* Convert the color name STR into the magical number.  */
static int atocolor(char *str, rgb *result)
{
      char *ptr;
      int i;
      
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
			result[0] = color_to_rgb[i];
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
			result[1] = color_to_rgb[i];
			break;
		  }
	  }

      if (i == 16)
		return -1;

	return 0;
}

// parse colors second:
// color NORMAL [HIGHLIGHT [HELPTEXT [HEADING]]]
// see http://diddy.boot-land.net/grub4dos/files/commands.htm#color
int parse_color(char *s) {
	int parsed;
	char *next_token = tokenize(s);
	
	parsed = atocolor(s, (rgb *)&config_color_normal);
	if (parsed == -1)
		return ERR_INVALID_COLOR;

	if (!next_token)
		return 0;
	s = next_token;
	next_token = tokenize(s);

	parsed = atocolor(s, (rgb *)&config_color_highlight);
	if (parsed == -1)
		return ERR_INVALID_COLOR;

	if (!next_token)
		return 0;
	s = next_token;
	next_token = tokenize(s);

	parsed = atocolor(s, (rgb *)&config_color_helptext);
	if (parsed == -1)
		return ERR_INVALID_COLOR;

	if (!next_token)
		return 0;
	s = next_token;
	next_token = tokenize(s);

	parsed = atocolor(s, (rgb *)&config_color_heading);
	if (parsed == -1)
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
	}
	
	return 0;
}
