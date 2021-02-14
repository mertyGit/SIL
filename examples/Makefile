DEBUG = -g
CFLAGS = 
CFILES = ../src/log.c ../src/sil.c ../src/framebuffer.c ../src/layer.c ../src/drawing.c ../src/filter.c ../src/lodepng.c ../src/font.c



wingdi: $(CFILES)
	x86_64-w64-mingw32-gcc $(CFLAGS) $(DEBUG) $^ ../src/winGDIdisplay.c combined.c -o combined -mconsole -lgdi32 -DSIL_W32 -I../include/
	x86_64-w64-mingw32-gcc $(CFLAGS) $(DEBUG) $^ ../src/winGDIdisplay.c filters.c -o filters -mconsole -lgdi32 -DSIL_W32 -I../include/

winsdl: $(CFILES)
	x86_64-w64-mingw32-gcc $(CFLAGS) $(DEBUG) $^ ../src/winSDLdisplay.c combined.c -o combined -mconsole -I/usr/local/x86_64-w64-mingw32/include/ -I../include/ -L/usr/local/x86_64-w64-mingw32/lib/ -lSDL2main -lSDL2 -DSDL_MAIN_HANDLED 
	x86_64-w64-mingw32-gcc $(CFLAGS) $(DEBUG) $^ ../src/winSDLdisplay.c filters.c -o filters -mconsole -I/usr/local/x86_64-w64-mingw32/include/ -I../include/ -L/usr/local/x86_64-w64-mingw32/lib/ -lSDL2main -lSDL2 -DSDL_MAIN_HANDLED 

x11: $(CFILES)
	$(CC) $(CFLAGS) $(DEBUG) $^ ../src/x11display.c combined.c -o combined -I../include/ -lX11
	$(CC) $(CFLAGS) $(DEBUG) $^ ../src/x11display.c filters.c -o filters -I../include/ -lX11

clean: 
	rm -rf *.exe combined filters
