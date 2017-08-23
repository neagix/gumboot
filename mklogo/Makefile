DEFINES = -DLODEPNG_NO_COMPILE_ANCILLARY_CHUNKS -DLODEPNG_NO_COMPILE_CPP

OBJS = lodepng.o main.o

TARGET = mklogo

$(TARGET): $(OBJS)
	@echo "  LINK      $@"
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

%.o: %.c
	@echo "  COMPILE   $<"
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

clean:
	rm -f *.o
