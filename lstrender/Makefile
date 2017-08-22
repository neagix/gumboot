OBJS = lodepng.o lstrender.o

TARGET = lstrender

$(TARGET): $(OBJS)
	@echo "  LINK      $@"
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

%.o: %.c
	@echo "  COMPILE   $<"
	$(CC) $(CFLAGS) $(DEFINES) -Wp,-MMD,-MQ,-MP -c $< -o $@

clean:
	rm -f *.o ../lodepng/*.o
