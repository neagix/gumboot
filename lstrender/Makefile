OBJS = lodepng.o lstrender.o ../config.o ../atoi.o ../menu_render.o \
	virtual_console.o ../console_common.o ../font.o

TARGET = lstrender

$(TARGET): $(OBJS)
	@echo "  LINK      $@"
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

%.o: %.c
	@echo "  COMPILE   $<"
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

clean:
	rm -f *.o ../*.o
