/*
	mini - a Free Software replacement for the Nintendo/BroadOn IOS.
	PowerPC ELF file loading

Copyright (C) 2008, 2009	Hector Martin "marcan" <marcan@marcansoft.com>
Copyright (C) 2009			Andre Heider "dhewg" <dhewg@wiibrew.org>

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#include "types.h"
#include "powerpc.h"
#include "hollywood.h"
#include "gecko.h"
#include "ff.h"
#include "powerpc_elf.h"
#include "elf.h"
#include "malloc.h"
#include "string.h"
#include "mini_ipc.h"
#include "log.h"

#define PHDR_MAX 10

static Elf32_Ehdr elfhdr;
static Elf32_Phdr phdrs[PHDR_MAX];

int is_valid_elf(const char *path)
{
	u32 read;
	FIL fd;
	FRESULT fres;

	fres = f_open(&fd, path, FA_READ);
	if (fres != FR_OK)
		return fres;

	fres = f_read(&fd, &elfhdr, sizeof(elfhdr), &read);
	if (fres != FR_OK) {
		f_close(&fd);
		return fres;
	}

	if (read != sizeof(elfhdr)) {
		f_close(&fd);
		return -100;
	}

	if (memcmp("\x7F" "ELF\x01\x02\x01\x00\x00",elfhdr.e_ident,9)) {
		f_close(&fd);
		gecko_printf("Invalid ELF header! 0x%02x 0x%02x 0x%02x 0x%02x\n",elfhdr.e_ident[0], elfhdr.e_ident[1], elfhdr.e_ident[2], elfhdr.e_ident[3]);
		return -101;
	}

	// entry point not checked here

	if (elfhdr.e_phoff == 0 || elfhdr.e_phnum == 0) {
		f_close(&fd);
		gecko_printf("ELF has no program headers!\n");
		return -103;
	}

	if (elfhdr.e_phnum > PHDR_MAX) {
		f_close(&fd);
		gecko_printf("ELF has too many (%d) program headers!\n", elfhdr.e_phnum);
		return -104;
	}

	fres = f_lseek(&fd, elfhdr.e_phoff);
	if (fres != FR_OK) {
		f_close(&fd);
		return -fres;
	}

	fres = f_read(&fd, phdrs, sizeof(phdrs[0])*elfhdr.e_phnum, &read);
	if (fres != FR_OK) {
		f_close(&fd);
		return -fres;
	}

	if (read != sizeof(phdrs[0])*elfhdr.e_phnum) {
		f_close(&fd);
		return -105;
	}

	u16 count = elfhdr.e_phnum;
	Elf32_Phdr *phdr = phdrs;

	int found = 0;
	while (count--) {
		if (phdr->p_type == PT_LOAD) {
			found = 1;
			break;
		}
		phdr++;
	}
	if (!found) {
		f_close(&fd);
		return -200;
	}
	
	f_close(&fd);
	return 0;
}

static char *memstr(char *mem, unsigned int fsize, const char *s) {
	unsigned int i;
	int l = strlen(s);
	int match = 0;
	for(i=0;i<fsize;i++) {
		if (mem[i] == s[match]) {
			match++;
			if (match == l)
				break;
		} else {
			match = 0;
		}
	}
	
	// not found?
	if (match != l)
		return NULL;

	return mem+i;
}

const char 	*bootargs_end_marker = "mark.end=1",
			*bootargs_start_marker = "mark.start=1";

static int edit_bootargs(char *mem, unsigned int fsize, const char *args) {
	int l = strlen(args);
	if (!l)
		return 0;

	// find a match for start marker
	char *start = memstr(mem, fsize, bootargs_start_marker);
	if (!start)
		return -1;

	// find a match for end marker
	char *end = memstr(start, fsize-(start-mem), bootargs_end_marker);
	if (!end)
		return -2;
	end += strlen(bootargs_end_marker);
	int span = (end-start);
	if (l > span)
		return -3;
	memcpy(start, args, l);
	
	// blank rest of bootargs
	int i;
	for(i=l;i<span;i++) {
		start[i] = ' ';
	}

	return 0;
}

// will boot via IPC call to MINI
int powerpc_boot_file(const char *path, const char *args) {
	FRESULT res;
	FILINFO stat;
	FIL fd;

	res = f_stat(path, &stat);
	if (res != FR_OK) {
		return res;
	}

	unsigned int fsize = stat.fsize;
	char *mem = malloc(fsize);
	if (!mem) {
		return -300;
	}

	res = f_open(&fd, path, FA_READ);
	if (res != FR_OK) {
		return res;
	}
	
	unsigned int read;
	res = f_read(&fd, mem, fsize, &read);
	if (res != FR_OK) {
		free(mem);
		f_close(&fd);
		return res;
	}
	if (read != fsize) {
		return -200;
	}
	
	int err = edit_bootargs(mem, fsize, args);
	if (err) {
		log_printf("could not edit bootargs: %d\n", err);
		return err;
	}
	
	return ipc_powerpc_boot(mem, fsize);
}
