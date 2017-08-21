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
	// is this a gamecube?
	if (read32(0x0d800190) & 2) {
		gecko_printf("GameCube compatibility mode detected...\n");

		powerpc_hang();
		return 1; /* never reached */
	}
	
	init_font(config_color_normal, font_yuv_normal);
	init_font(config_color_highlight, font_yuv_highlight);
	init_font(config_color_helptext, font_yuv_helptext);
	init_font(config_color_heading, font_yuv_heading);
	
	selected_font_yuv = font_yuv_normal;

    input_init();
	init_fb(vmode, config_color_normal[1]);

	VIDEO_Init(vmode);
	VIDEO_SetFrameBuffer(get_xfb());
	
	// Wii mode
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
	
	config_load();    
    
	menu_draw(config_timeout);
   
    menu_selection = config_default;

	menu_draw_entries();

    // update internal console position
    int i;
    for(i=4;i<config_entries_count+4;i++) {
		gfx_printf("\n");
	}

/*
	DIR dirs;
	FILINFO Fno;
	const char path[512] = "bootmii";

	res = f_opendir(&dirs, path);
	if (res != FR_OK) {
		log_printf("failed to open directory: %d\n", res);
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

	u64 start_time = mftb_usec();
	int last_time_elapsed = 0;

	u16 btn;
	while (1) {
normal_loop:
		;
		do {
			if (config_timeout) {
				// update timeout as needed
				int new_time_elapsed = (int)((mftb_usec()-start_time)/1000000);
				
				if (new_time_elapsed != last_time_elapsed) {
					last_time_elapsed = new_time_elapsed;
					int left = config_timeout-last_time_elapsed;
					if (left <= 0) {
						config_timeout = 0;
						menu_clear_timeout();
						
						menu_activate();
						goto normal_loop;
					} else {
						menu_update_timeout(left);
					}
				}
			}
			
			usleep(INPUT_WAIT_CYCLE_DELAY);
			btn = input_read();
			
			// disable timeout sequence
			if ((btn != 0) && (last_time_elapsed > 1)) {
				config_timeout = 0;
				menu_clear_timeout();
			}
		} while (!btn);
		
		if (btn & PAD_BUTTON_A) {
			menu_activate();
		} else if (btn & PAD_BUTTON_DOWN) {
			menu_down();
		} else if (btn & PAD_BUTTON_UP) {
			menu_up();
		}
	}

	powerpc_hang();

	return 0;
}

