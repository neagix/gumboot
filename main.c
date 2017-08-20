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

#include "bootmii_ppc.h"
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

#define MINIMUM_MINI_VERSION 0x00010001

static void dsp_reset(void)
{
	write16(0x0c00500a, read16(0x0c00500a) & ~0x01f8);
	write16(0x0c00500a, read16(0x0c00500a) | 0x0010);
	write16(0x0c005036, 0);
}

void powerpc_hang(void)
{
	ipc_sys_clear32(HW_RESETS, 0x30);
	udelay(100);
	ipc_sys_set32(HW_RESETS, 0x20);
	udelay(100);
}

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

	gfx_printf("\xc9\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xbb\n");
    gfx_printf("\xba Gumboot menu v0.1 \xba\n");
    gfx_printf("\xc8\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xbc\n");
    
    int power_pressed = 0, reset_pressed = 0, eject_pressed = 0;

	while (1) {
		u16 btn = input_wait();
		
		if (btn & GPIO_POWER) {
			power_pressed = !power_pressed;
			if (!power_pressed) {
				gfx_printf("power event: %d\n", btn);
			}
		}

		if (btn & GPIO_RESET) {
			reset_pressed = !reset_pressed;
			if (!reset_pressed) {
				gfx_printf("reset event: %d\n", btn);
			}
		}
	
		if (btn & GPIO_EJECT) {
			eject_pressed = !eject_pressed;
			if (!eject_pressed) {
				gfx_printf("eject event: %d\n", btn);
			}
		}
		
	}
	
	powerpc_hang();

	return 0;
}

