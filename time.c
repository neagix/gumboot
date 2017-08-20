/*
	BootMii - a Free Software replacement for the Nintendo/BroadOn bootloader.
	Requires mini.

Copyright (C) 2008		Segher Boessenkool <segher@kernel.crashing.org>

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#include "hollywood.h"

// Timebase frequency is bus frequency / 4.
// following constants are from libogc - Wii only
#define TB_BUS_CLOCK				243000000u
#define TB_CORE_CLOCK				729000000u

// constants that would be used for Gamecube
//#define TB_BUS_CLOCK				162000000u
//#define TB_CORE_CLOCK				486000000u

#define TB_TIMER_CLOCK				(TB_BUS_CLOCK/4000)			//4th of the bus frequency

#define ticks_to_microsecs(ticks)	((((u64)(ticks)*8)/(u64)(TB_TIMER_CLOCK/125)))
#define microsecs_to_ticks(usec)	(((u64)(usec)*(TB_TIMER_CLOCK/125))/8)

u64 mftb(void)
{
  u32 hi, lo, dum;
  
  asm("0: mftbu %0 ; mftb %1 ; mftbu %2 ; cmplw %0,%2 ; bne 0b" 
      : "=r"(hi), "=r"(lo), "=r"(dum)); 
  return ((u64)hi << 32) | lo;
}

static void __delay(u64 ticks)
{
	u64 start = mftb();

	while (mftb() - start < ticks)
		;
}

void usleep(u32 us)
{
	__delay(microsecs_to_ticks(us));
}

u64 mftb_usec(void) {
	return ticks_to_microsecs(mftb());
}
