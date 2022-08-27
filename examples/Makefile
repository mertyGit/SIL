ifeq ($(DEST),gdi) 
  E = .exe
  DISP = winGDIdisplay
  # REMOVE "-mconsole" to don't get debugging/logging console (!)
  CFLAGS = -mconsole -lgdi32 -DSIL_W32 
  CC = x86_64-w64-mingw32-gcc
  ICO = ico.rco
endif
ifeq ($(DEST),winsdl) 
  E = .exe
  DISP = SDLdisplay
  # REMOVE "-mconsole" to don't get debugging/logging console (!)
  CFLAGS= -mconsole -I/usr/local/x86_64-w64-mingw32/include/ -L/usr/local/x86_64-w64-mingw32/lib/ -lSDL2main -lSDL2 -DSDL_MAIN_HANDLED 
  CC = x86_64-w64-mingw32-gcc 
  ICO = ico.rco
endif
ifeq ($(DEST),lnxsdl) 
  CFLAGS=  -lSDL2main -lSDL2 -DSDL_MAIN_HANDLED 
  DISP = SDLdisplay
  CC = gcc
endif
ifeq ($(DEST),macsdl) 
  DISP = SDLdisplay
  CFLAGS=  -L/opt/homebrew/lib -I/opt/homebrew/include/ -lSDL2main -lSDL2 -DSDL_MAIN_HANDLED 
  CC = gcc
endif
ifeq ($(DEST),x11) 
  E = 
  DISP = x11display
  CFLAGS = -lX11 
  CC = cc
  ICO =
endif
ifeq ($(DEST),fb) 
  E = 
  DISP = lnxFBdisplay
  CFLAGS = 
  CC = cc
  ICO =
endif
DEBUG = -g
DEPS = ../src/sil.h ../src/log.h ../src/lodepng.h
OBJ = log.o sil.o framebuffer.o layer.o drawing.o filter.o lodepng.o font.o group.o
CFLAGS +=-I ../src/

$(PROG)$E: $(PROG).o $(OBJ) $(DISP).o 
	$(CC) $(ICO) -o $@ $^ $(CFLAGS)

$(PROG).o: $(PROG).c $(DEPS)
	$(CC) $(DEBUG) -c $< -o $@ $(CFLAGS)

%.o: ../src/%.c $(DEPS)
	$(CC) $(DEBUG) -c $< -o $@ $(CFLAGS)

gdi: clean
winsdl: clean
lnxsdl: clean
x11: clean
fb:  clean

%: 
	$(MAKE) -C ../examples/ PROG=combined  DEST=$@
	$(MAKE) -C ../examples/ PROG=filters   DEST=$@
	$(MAKE) -C ../examples/ PROG=ordercopy DEST=$@
	$(MAKE) -C ../examples/ PROG=draw      DEST=$@
	$(MAKE) -C ../examples/ PROG=text      DEST=$@

clean: 
	rm -rf ../examples/*.exe ../examples/*.o ../src/*.o ../examples/*.o ../examples/combined ../examples/filters ../examples/*dump.png ../examples/printscreen.png
	rm -rf ../examples/draw ../examples/combined ../examples/filters ../examples/ordercopy ../examples/text
