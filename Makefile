include broadway.mk

DEFINES = -DLACKS_SYS_TYPES_H -DLACKS_ERRNO_H -DLACKS_STDLIB_H -DLACKS_STRING_H -DLACKS_STRINGS_H -DLACKS_UNISTD_H -DGUMBOOT
LDSCRIPT = mini.ld
LIBS = -lgcc

TARGET = gumboot.elf

OBJS = realmode.o crt0.o main.o string.o sync.o time.o printf.o input.o \
	exception.o exception_2200.o malloc.o gecko.o video_low.o \
	ipc.o mini_ipc.o nandfs.o ff.o diskio.o font.o console.o \
	menu.o powerpc.o config.o atoi.o powerpc_elf.o log.o \
	filesystem.o menu_render.o

include common.mk

upload: $(TARGET)
	@$(WIIDEV)/bin/bootmii -p $<

.PHONY: upload
