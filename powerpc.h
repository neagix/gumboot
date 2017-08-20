/*
	BootMii - a Free Software replacement for the Nintendo/BroadOn bootloader.
	Requires mini.

Copyright (C) 2008		Segher Boessenkool <segher@kernel.crashing.org>

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#ifndef __PPC_H__
#define __PPC_H__

#include "time.h"
#include "hollywood.h"

void dsp_reset(void);

void powerpc_hang(void);
void powerpc_reset(void);
void powerpc_poweroff(void);

#include "types.h"
#include "printf.h"

#define OK 0
#define EFAIL 1

#define MEM2_BSS __attribute__ ((section (".bss.mem2")))
#define MEM2_DATA __attribute__ ((section (".data.mem2")))
#define MEM2_RODATA __attribute__ ((section (".rodata.mem2")))
#define ALIGNED(x) __attribute__((aligned(x)))

#define STACK_ALIGN(type, name, cnt, alignment)         \
	u8 _al__##name[((sizeof(type)*(cnt)) + (alignment) + \
	(((sizeof(type)*(cnt))%(alignment)) > 0 ? ((alignment) - \
	((sizeof(type)*(cnt))%(alignment))) : 0))]; \
	type *name = (type*)(((u32)(_al__##name)) + ((alignment) - (( \
	(u32)(_al__##name))&((alignment)-1))))


// Address mapping.

static inline u32 virt_to_phys(const void *p)
{
	return (u32)p & 0x7fffffff;
}

static inline void *phys_to_virt(u32 x)
{
	return (void *)(x | 0x80000000);
}


// Cache synchronisation.

void sync_before_read(void *p, u32 len);
void sync_after_write(const void *p, u32 len);
void sync_before_exec(const void *p, u32 len);


// Special purpose registers.

#define mtspr(n, x) do { asm("mtspr %1,%0" : : "r"(x), "i"(n)); } while (0)
#define mfspr(n) ({ \
	u32 x; asm volatile("mfspr %0,%1" : "=r"(x) : "i"(n)); x; \
})


// Exceptions.

void exception_init(void);


// Debug: blink the tray led.

static inline void blink(void)
{
	write32(0x0d8000c0, read32(0x0d8000c0) ^ 0x20);
}


#endif
