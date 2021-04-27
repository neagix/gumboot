include broadway.mk

OBJDIR := .obj

DEFINES = -DLACKS_SYS_TYPES_H -DLACKS_ERRNO_H -DLACKS_STDLIB_H -DLACKS_STRING_H -DLACKS_STRINGS_H -DLACKS_UNISTD_H \
	-DGUMBOOT \
	-DLODEPNG_NO_COMPILE_DISK -DLODEPNG_NO_COMPILE_ANCILLARY_CHUNKS -DLODEPNG_NO_COMPILE_CPP \
	-DNO_SPLASHIMAGE

LDSCRIPT = mini.ld
LIBS = -lgcc

TARGET = gumboot.elf

OBJS = $(OBJDIR)/realmode.o $(OBJDIR)/crt0.o $(OBJDIR)/main.o $(OBJDIR)/string.o \
	$(OBJDIR)/sync.o $(OBJDIR)/time.o $(OBJDIR)/printf.o $(OBJDIR)/input.o \
	$(OBJDIR)/exception.o $(OBJDIR)/exception_2200.o $(OBJDIR)/malloc.o $(OBJDIR)/gecko.o $(OBJDIR)/video_low.o \
	$(OBJDIR)/ipc.o $(OBJDIR)/mini_ipc.o $(OBJDIR)/diskio.o $(OBJDIR)/font.o $(OBJDIR)/console.o \
	$(OBJDIR)/menu.o $(OBJDIR)/powerpc.o $(OBJDIR)/config.o $(OBJDIR)/atoi.o $(OBJDIR)/powerpc_elf.o $(OBJDIR)/log.o \
	$(OBJDIR)/menu_render.o $(OBJDIR)/console_common.o $(OBJDIR)/fatfs/ff.o $(OBJDIR)/fatfs/ffunicode.o \
	$(OBJDIR)/utils.o $(OBJDIR)/lodepng.o $(OBJDIR)/logo.o $(OBJDIR)/browse.o

include common.mk

mklogo/mklogo:
	if patch --forward --batch -p1 --dry-run < lodepng-gumboot.patch >/dev/null; then exec patch -p1 < lodepng-gumboot.patch; fi
	make -C mklogo

logo.c: logo.png mklogo/mklogo
	@echo "  GENERATE   $@"
	mklogo/mklogo logo.png >logo.c 2>logo.h

upload: $(TARGET)
	@$(WIIDEV)/bin/bootmii -p $<

.PHONY: upload mklogo/mklogo
