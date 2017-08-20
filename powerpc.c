
#include "powerpc.h"
#include "ipc.h"

void dsp_reset(void)
{
	write16(0x0c00500a, read16(0x0c00500a) & ~0x01f8);
	write16(0x0c00500a, read16(0x0c00500a) | 0x0010);
	write16(0x0c005036, 0);
}

void powerpc_hang(void)
{
	ipc_sys_clear32(HW_RESETS, 0x30);
	usleep(100);
	ipc_sys_set32(HW_RESETS, 0x20);
	usleep(100);
}
