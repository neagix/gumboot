/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "fatfs/diskio.h"		/* FatFs lower layer API */
#include "mini_ipc.h"
#include "string.h"

static u8 *sector_buffer[512] __attribute__((aligned(32)));

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	// only SD supported
	if (pdrv != 0)
		return STA_NODISK;

	int state = sd_get_state();

	switch (state) {
	case SDMMC_NO_CARD:
		return STA_NODISK;
	case SDMMC_NEW_CARD:
		return STA_NOINIT;
	}
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (BYTE drv)
{
	// only SD supported
	if (drv != 0)
		return STA_NODISK;

	int state = sd_get_state();

	switch (state) {
	case SDMMC_NO_CARD:
		return STA_NODISK;

	case SDMMC_NEW_CARD:
		if (sd_mount())
			return STA_NOINIT;
		else
			return 0;

	default:
		return 0;
	}
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	u32 i;
	DRESULT res;

	// only SD supported
	if (pdrv != 0)
		return STA_NODISK;

	if (count > 1 && ((u32) buff % 64) == 0) {
		if (sd_read(sector, count, buff) != 0)
			return RES_ERROR;
		return RES_OK;
	}

	res = RES_OK;
	for (i = 0; i < count; i++) {
		if (sd_read(sector + i, 1, sector_buffer) != 0) {
			res = RES_ERROR;
			break;
		}

		memcpy(buff + i * 512, sector_buffer, 512);
	}

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if _READONLY == 0
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	// only SD supported
	if (pdrv != 0)
		return STA_NODISK;

	if (count > 1 && ((u32) buff % 64) == 0) {
		if (sd_write(sector, count, buff) != 0)
			return RES_ERROR;
		return RES_OK;
	}

	u32 i;
	for (i = 0; i < count; i++) {
		memcpy(sector_buffer, buff + i * 512, 512);
		if (sd_write(sector + i, 1, sector_buffer) != 0) {
			return RES_ERROR;
		}
	}

	return RES_OK;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	// only SD supported
	if (pdrv != 0)
		return STA_NODISK;

	u32 *buff_u32 = (u32 *) buff;

	switch (cmd) {
	case CTRL_SYNC:
		return RES_OK;
	case GET_SECTOR_COUNT:
		*buff_u32 = sd_getsize();
		return RES_OK;
	case GET_SECTOR_SIZE:
		*buff_u32 = 512;
		return RES_OK;
		break;
	case GET_BLOCK_SIZE:
		*buff_u32 = 512;
		return RES_OK;
	}

	return RES_PARERR;
}

