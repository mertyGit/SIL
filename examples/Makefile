ifeq ($(DEST),gdi) 
  O = obj
  E = .exe
  DISP = winGDIdisplay
  CFLAGS = -mconsole -lgdi32 -DSIL_W32 -I../include/  
  CC = x86_64-w64-mingw32-gcc
endif
ifeq ($(DEST),sdl) 
  O = obj
  E = .exe
  DISP = winSDLdisplay
  CFLAGS= -mconsole -I/usr/local/x86_64-w64-mingw32/include/ -I../include/ -L/usr/local/x86_64-w64-mingw32/lib/ -lSDL2main -lSDL2 -DSDL_MAIN_HANDLED 
  CC = x86_64-w64-mingw32-gcc 
endif
ifeq ($(DEST),x11) 
  O = o
  E = 
  DISP = x11display
  CFLAGS = -I../include/ -lX11 
  CC = cc
endif
DEBUG = -g
DEPS = ../include/sil.h ../include/log.h ../include/lodepng.h
OBJ = log.$O sil.$O framebuffer.$O layer.$O drawing.$O filter.$O lodepng.$O font.$O

$(PROG)$E: $(PROG).$O $(OBJ) $(DISP).$O 
	$(CC) -o $@ $^ $(CFLAGS)

$(PROG).$O: $(PROG).c $(DEPS)
	$(CC) $(DEBUG) -c $< -o $@ $(CFLAGS)

%.$O: ../src/%.c $(DEPS)
	$(CC) $(DEBUG) -c $< -o $@ $(CFLAGS)


wingdi: 
	$(MAKE) PROG=combined DEST=gdi
	$(MAKE) PROG=filters  DEST=gdi

winsdl:
	$(MAKE) PROG=combined DEST=sdl
	$(MAKE) PROG=filters  DEST=sdl

x11:
	$(MAKE) PROG=combined DEST=x11
	$(MAKE) PROG=filters  DEST=x11

clean: 
	rm -rf *.exe *.obj *.o combined filters
