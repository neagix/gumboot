/*
	mini - a Free Software replacement for the Nintendo/BroadOn IOS.
	PowerPC ELF file loading

Copyright (C) 2008, 2009	Hector Martin "marcan" <marcan@marcansoft.com>
Copyright (C) 2009			Andre Heider "dhewg" <dhewg@wiibrew.org>

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#ifndef __POWERPC_ELF_H__
#define __POWERPC_ELF_H__

int is_valid_elf(const char *path);
int powerpc_boot_file(const char *path, const char *args);
int try_boot_file(char *kernel_fn, const char *args);

#endif

