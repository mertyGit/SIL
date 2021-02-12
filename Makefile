DEBUG = -g
CFLAGS = 
CFILES = log.c sil.c framebuffer.c layer.c drawing.c filter.c lodepng.c font.c



wingdi: $(CFILES)
	x86_64-w64-mingw32-gcc $(CFLAGS) $(DEBUG) $^ winGDIdisplay.c main.c -o test -mconsole -lgdi32 -DSIL_W32 

winsdl: $(CFILES)
	x86_64-w64-mingw32-gcc $(CFLAGS) $(DEBUG) $^ winSDLdisplay.c main.c -o test -mconsole -I/usr/local/x86_64-w64-mingw32/include/  -L/usr/local/x86_64-w64-mingw32/lib/ -lSDL2main -lSDL2 -DSDL_MAIN_HANDLED

x11: $(CFILES)
	$(CC) $(CFLAGS) $(DEBUG) $^ x11display.c main.c -o test -lX11
