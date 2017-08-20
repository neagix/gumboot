
#include "powerpc.h"
#include "ipc.h"
#include "time.h"

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

void powerpc_reset(void)
{
	// enable the broadway IPC interrupt
	write32(HW_PPCIRQMASK, (1<<30));
	ipc_sys_clear32(HW_RESETS, 0x30);
	usleep(100);
	ipc_sys_set32(HW_RESETS, 0x20);
	usleep(100);
	ipc_sys_set32(HW_RESETS, 0x10);
	usleep(100000);
	ipc_sys_set32(HW_EXICTRL, EXICTRL_ENABLE_EXI);
}

void powerpc_poweroff() {
	/* make sure that the poweroff GPIO is configured as output */
	ipc_sys_set32(HW_GPIO1DIR, HW_GPIO_SHUTDOWN);

	/* drive the poweroff GPIO high */
	ipc_sys_set32(HW_GPIO1OUT, HW_GPIO_SHUTDOWN);
}
