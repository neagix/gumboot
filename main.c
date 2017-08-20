/*
        BootMii - a Free Software replacement for the Nintendo/BroadOn bootloader.
        Requires mini.

Copyright (C) 2008, 2009        Haxx Enterprises <bushing@gmail.com>
Copyright (C) 2009              Andre Heider "dhewg" <dhewg@wiibrew.org>
Copyright (C) 2008, 2009        Hector Martin "marcan" <marcan@marcansoft.com>
Copyright (C) 2008, 2009        Sven Peter <svenpeter@gmail.com>
Copyright (C) 2009              John Kelley <wiidev@kelley.ca>
Copyright (C) 2017              neagix

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#include "powerpc.h"
#include "string.h"
#include "ipc.h"
#include "gecko.h"
#include "mini_ipc.h"
#include "nandfs.h"
#include "fat.h"
#include "malloc.h"
#include "diskio.h"
#include "printf.h"
#include "video_low.h"
#include "input.h"
#include "console.h"
#include "menu.h"
#include "config.h"

#define MINIMUM_MINI_VERSION 0x00010001

int main(void)
{
	int vmode = -1;
	exception_init();
	dsp_reset();

	// clear interrupt mask
	write32(0x0c003004, 0);

	ipc_initialize();
	ipc_slowping();

	gecko_init();
    input_init();
	init_fb(vmode);

	VIDEO_Init(vmode);
	VIDEO_SetFrameBuffer(get_xfb());
	VISetupEncoder();

	u32 version = ipc_getvers();
	u16 mini_version_major = version >> 16 & 0xFFFF;
	u16 mini_version_minor = version & 0xFFFF;
	gecko_printf("Mini version: %d.%0d\n", mini_version_major, mini_version_minor);

	if (version < MINIMUM_MINI_VERSION) {
		gecko_printf("Sorry, this version of MINI (armboot.bin)\n"
			"is too old, please update to at least %d.%0d.\n", 
			(MINIMUM_MINI_VERSION >> 16), (MINIMUM_MINI_VERSION & 0xFFFF));
		powerpc_hang();
		return 1; /* never reached */
	}
	const char *menu_title = "Gumboot menu v0.1";

    gfx_print_at((CONSOLE_COLUMNS-strlen(menu_title))/2, 1, menu_title);
    
    int box_h = CONSOLE_LINES-4-HELP_LINES;
    draw_box_at(0, 3, CONSOLE_COLUMNS, box_h);
    
    // draw help text
    gfx_print_at(2, box_h+3, "Use the power (\x18) and reset (\x19) buttons to highlight an entry.\n"
												"Long-press (2s) power or press eject to boot.\n\n"
												"The highlighted entry will be booted automatically in 15 seconds.");
    
    // update internal console position
    gfx_printf("\n\n\n\n");
    
	config_load();
	
/*
	DIR dirs;
	FILINFO Fno;
	const char path[512] = "bootmii";

	res = f_opendir(&dirs, path);
	if (res != FR_OK) {
		gfx_printf("failed to open directory: %d\n", res);
	} else {
		while (((res = f_readdir(&dirs, &Fno)) == FR_OK) && Fno.fname[0]) {
			if (Fno.fattrib & AM_DIR) {
				gfx_printf(" directory: %s\n", Fno.fname);
			} else {
				gfx_printf(" file: %s\n", Fno.fname);
			}
		}
	}
	gfx_printf("last res: %d\n", res); */
	
	while (1) {
		u16 btn = input_wait();
		
		if (btn) {
			gfx_printf_at(CONSOLE_COLUMNS/2, CONSOLE_LINES/2, "power: %d long press %d",     (btn & GPIO_POWER) != 0, (btn & GPIO_POWER_LP) != 0);
			gfx_printf_at(CONSOLE_COLUMNS/2, CONSOLE_LINES/2 + 1, "reset: %d long press %d", (btn & GPIO_RESET) != 0, (btn & GPIO_RESET_LP) != 0);
			gfx_printf_at(CONSOLE_COLUMNS/2, CONSOLE_LINES/2 + 2, "eject: %d long press %d", (btn & GPIO_EJECT) != 0, (btn & GPIO_EJECT_LP) != 0);
		}
	}
	
	powerpc_hang();

	return 0;
}

