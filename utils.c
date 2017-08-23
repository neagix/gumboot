
#include "malloc.h"
#include "types.h"
#include "fatfs/ff.h"
#include "log.h"

void *load_file(const char *fname, u32 max_allowed, u32 *read) {
	FRESULT res;
	FIL fd;
	FILINFO stat;

	res = f_stat(fname, &stat);
	if (res != FR_OK) {
		log_printf("failed to stat %s: %d\n", fname, res);
		return NULL;
	}

	res = f_open(&fd, fname, FA_READ);
	if (res != FR_OK) {
		log_printf("failed to open %s: %d\n", fname, res);
		return NULL;
	}
	
	DWORD fsize = stat.fsize;
	if (fsize > max_allowed) {
		fsize = max_allowed;
		log_printf("truncating %s to %d bytes\n", fname, fsize);
	}
	void *mem = malloc(fsize);
	if (!mem) {
		log_printf("failed to allocate %d bytes\n", fsize);
		return NULL;
	}
	
	res = f_read(&fd, mem, fsize, read);
	if (res != FR_OK) {
		log_printf("failed to read %s: %d\n", fname, res);
		free(mem);
		f_close(&fd);
		return NULL;
	}
	f_close(&fd);

	return mem;
}
