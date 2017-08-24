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
#include "utils.h"
#include "lodepng/lodepng.h"
#include "raster.h"

void flush_logs_and_enable_gfx(void);

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
	
	// mount the first partition, where we expect to find gumboot/gumboot.lst
	int has_fs = 0;
	FATFS fatfs;
	FRESULT res = f_mount(&fatfs, "0:", 1);
	if (res == FR_OK) {
		has_fs = 1;

		u32 read;
		char *cfg_data = (char*)load_file(DEFAULT_LST, MAX_LST_SIZE, &read);
		if (cfg_data) {
			// terminate string
			cfg_data[read] = 0;

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
	// however 'gfx_console_init' is set to 1 later on

	if (version < MINIMUM_MINI_VERSION) {
		init_font(FONT_ERROR);
		init_font(FONT_NORMAL);
		flush_logs_and_enable_gfx();

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

	raster splash, *valid_splash = NULL;
	if (has_fs) {
		if (config_splashimage) {
			u32 read;
			void *png = load_file(config_splashimage, MAX_SPLASH_SIZE, &read);
			if (png) {
				// convert to raster
				int err = png_to_raster(png, read, splash);
				free(png);
				if (err) {
					log_printf("error %u: %s\n", err, lodepng_error_text(err));
				} else {
					valid_splash = &splash;
				}
			}
		}
		int err = f_mount(NULL, "0:", 0);
		if (err) {
			log_printf("failed to unmount: %d\n", err);
		}
	}	

    menu_selection = config_default;
    menu_init(valid_splash);
    
    if (config_nomenu) {
		if (!menu_activate())
			goto quit;
	}
    
	menu_draw_head_and_box(mini_version_major, mini_version_minor);
	menu_draw_timeout(config_timeout);
	menu_draw_entries_and_help();

	flush_logs_and_enable_gfx();

	u64 start_time = mftb_usec();
	int last_time_elapsed = 0;

	u16 btn;
	while (1) {
		do {
			if (config_timeout) {
				// update timeout as needed
				int new_time_elapsed = (int)((mftb_usec()-start_time)/1000000);
				
				if (new_time_elapsed != last_time_elapsed) {
					last_time_elapsed = new_time_elapsed;
					int left = config_timeout-last_time_elapsed;
					if (left <= 0) {
						// pretend there was an activation keypress
						btn = PAD_BUTTON_A;
						goto parse_input;
					} else {
						menu_update_timeout(left);
					}
				}
			}
			
			usleep(INPUT_WAIT_CYCLE_DELAY);
			btn = input_read();

parse_input:
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

void flush_logs_and_enable_gfx(void) {
	// do not buffer log entries anymore
	gfx_console_init = 1;

    // put all the backbuffer log lines below the menu entries
    console_move(0, HEAD_LINES + 1 + config_entries_count + 1);
	// flush log lines from the backbuffer - if any
	log_flush_bb();
	log_free_bb();
}
