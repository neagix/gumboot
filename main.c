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
#include "fatfs/ff.h"
#include "malloc.h"
#include "fatfs/diskio.h"
#include "printf.h"
#include "video_low.h"
#include "input.h"
#include "console.h"
#include "menu.h"
#include "menu_render.h"
#include "config.h"
#include "log.h"

#define MINIMUM_MINI_VERSION 0x00010001

int main(void)
{
	exception_init();
	dsp_reset();

	// clear interrupt mask
	write32(0x0c003004, 0);

	ipc_initialize();
	ipc_slowping();
	
	gecko_init();

	// initialise backbuffer for our precious log messages
	if (log_init_bb()) {
		gecko_printf("could not allocate log backbuffer\n");
		powerpc_hang();
		return -1;
	}
	
	FATFS fatfs;
	FRESULT res = f_mount(&fatfs, "0:", 1);
	if (res == FR_OK) {
		u32 read;
		char *cfg_data = config_load(DEFAULT_LST, &read);
		if (cfg_data) {
			if (config_load_from_buffer(cfg_data, read)) {
				// in case of error disable some features
				config_timeout = 0;
				config_nomenu = 0;
			}
			// error will be checked later on
			free(cfg_data);
		}
	} else {
		log_printf("could not mount first partition: %d\n", res);
	}
	
    input_init();
	init_fb(config_vmode);

	rgb black = {.as_rgba = {0, 0, 0, 0xFF}};
	clear_fb(black);

	VIDEO_Init(config_vmode);
	VIDEO_SetFrameBuffer(get_xfb());

	// is this a gamecube?
	if (read32(0x0d800190) & 2) {
		log_printf("GameCube compatibility mode detected...\n");
	} else {	
		// Wii mode
		VISetupEncoder();
	}

	u32 version = ipc_getvers();
	u16 mini_version_major = version >> 16 & 0xFFFF;
	u16 mini_version_minor = version & 0xFFFF;

	// from now, console could be used
	// however we assume it is not going to be used
	// until the graphical setup has been completed,
	// because log is still ripe with backbuffer entries
	gfx_console_init = 1;

	if (version < MINIMUM_MINI_VERSION) {
		init_font(FONT_ERROR);
		init_font(FONT_NORMAL);
		// flush log lines from the backbuffer - if any
		log_flush_bb();
		log_free_bb();

		log_printf("Mini version: %d.%0d\n", mini_version_major, mini_version_minor);
		log_printf("Sorry, this version of MINI (armboot.bin)\n"
			"is too old, please update to at least %d.%0d.\n", 
			(MINIMUM_MINI_VERSION >> 16), (MINIMUM_MINI_VERSION & 0xFFFF));

		powerpc_hang();
		return 1; /* never reached */
	}

	init_font(FONT_ERROR);
	init_font(FONT_NORMAL);
	init_font(FONT_HIGHLIGHT);
	init_font(FONT_HELPTEXT);
	init_font(FONT_HEADING);
			
    menu_selection = config_default;
    menu_init();
    
    if (config_nomenu) {
		if (!menu_activate())
			goto quit;
	}
    
	menu_draw(config_timeout, mini_version_major, mini_version_minor);
	menu_draw_entries();

    // put all the backbuffer log lines below the menu entries
    console_move(0, HEAD_LINES + 1 + config_entries_count);
	// flush log lines from the backbuffer - if any
	log_flush_bb();
	log_free_bb();

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
						
						if (!menu_activate())
							goto quit;
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
			if (!menu_activate())
				goto quit;
		} else if (btn & PAD_BUTTON_DOWN) {
			menu_down();
		} else if (btn & PAD_BUTTON_UP) {
			menu_up();
		}
	}
quit:
	powerpc_hang();

	return 0;
}

