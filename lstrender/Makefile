OBJDIR:=.obj

OBJS = $(OBJDIR)/lodepng.o $(OBJDIR)/lstrender.o $(OBJDIR)/shared/config.o $(OBJDIR)/shared/atoi.o $(OBJDIR)/shared/menu_render.o \
	$(OBJDIR)/virtual_console.o $(OBJDIR)/shared/console_common.o $(OBJDIR)/shared/font.o $(OBJDIR)/shared/logo.o

TARGET = lstrender

$(TARGET): $(OBJS)
	@echo "  LINK      $@"
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

$(OBJDIR)/%.o: %.c
	@echo "  COMPILE   $<"
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

$(OBJDIR)/shared/%.o: ../%.c
	@echo "  COMPILE   $<"
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

clean:
	rm -f $(OBJS)
