
#ifndef __HOLLYWOOD_H
#define __HOLLYWOOD_H

#include "types.h"

// Hollywood stuff

#define		HW_REG_BASE		0xd800000
#define		HW_RESETS		(HW_REG_BASE + 0x194)

// PPC side of GPIO1 (Starlet can access this too)
// Output state
#define		HW_GPIO1BOUT		(HW_REG_BASE + 0x0c0)
// Direction (1=output)
#define		HW_GPIO1BDIR		(HW_REG_BASE + 0x0c4)
// Input state
#define		HW_GPIO1BIN			(HW_REG_BASE + 0x0c8)

// hardware time/alarm
#define		HW_TIMER		(HW_REG_BASE + 0x010)
#define		HW_ALARM		(HW_REG_BASE + 0x014)

#define		HW_PPCIRQMASK		(HW_REG_BASE + 0x034)
#define		HW_EXICTRL		(HW_REG_BASE + 0x070)
#define		EXICTRL_ENABLE_EXI	1


// Starlet side of GPIO1
// Output state
#define		HW_GPIO1OUT		(HW_REG_BASE + 0x0e0)
// Direction (1=output)
#define		HW_GPIO1DIR		(HW_REG_BASE + 0x0e4)

#define HW_GPIO_SHUTDOWN	(1<<1)

// Basic I/O.

static inline u32 read32(u32 addr)
{
	u32 x;

	asm volatile("lwz %0,0(%1) ; sync" : "=r"(x) : "b"(0xc0000000 | addr));

	return x;
}

static inline void write32(u32 addr, u32 x)
{
	asm("stw %0,0(%1) ; eieio" : : "r"(x), "b"(0xc0000000 | addr));
}

static inline void mask32(u32 addr, u32 clear, u32 set)
{
	write32(addr, (read32(addr)&(~clear)) | set);
}

static inline u16 read16(u32 addr)
{
	u16 x;

	asm volatile("lhz %0,0(%1) ; sync" : "=r"(x) : "b"(0xc0000000 | addr));

	return x;
}

static inline void write16(u32 addr, u16 x)
{
	asm("sth %0,0(%1) ; eieio" : : "r"(x), "b"(0xc0000000 | addr));
}

#endif // __HOLLYWOOD_H
