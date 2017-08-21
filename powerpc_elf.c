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

// will boot via IPC call to MINI
int powerpc_boot_file(const char *path) {
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
	
	log_printf("IPC LOAD 0x%p %d -> ", mem, fsize);
	int err = ipc_powerpc_boot(mem, fsize);
	log_printf("%d\n", err);
	return err;
}
