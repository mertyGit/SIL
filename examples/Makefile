# Make one choice of target list below

# Windows + SDL 2.0.20 or later
#DEST=winsdl

# Linux + SDL 2.0.20 or later
DEST = lnxsdl

# Mac OS X + SDL 2.0.20 or later
#DEST = macsdl

# Windows native (GDI)
#DEST = gdi   

# Linux + X11 (xlib)
#DEST = x11  

# linux /dev/fb0 + /dev/input/event2 , like raspberry + touchscreen
#DEST = fb  

ifeq ($(DEST),gdi) 
  TARGET = SIL_TARGET_GDI
  # REMOVE "-mconsole" to get rid of debugging/logging console (!)
  CFLAGS = -mconsole -lgdi32 -DSIL_W32 
  CC     = x86_64-w64-mingw32-gcc
  E      = .exe
  ICO    = ico.rco
endif
ifeq ($(DEST),winsdl) 
  TARGET = SIL_TARGET_WINSDL
  # REMOVE "-mconsole" to get rid of debugging/logging console (!)
  CFLAGS = -mconsole -I/usr/local/x86_64-w64-mingw32/include/ -L/usr/local/x86_64-w64-mingw32/lib/ -lSDL2main -lSDL2 -DSDL_MAIN_HANDLED 
  CC     = x86_64-w64-mingw32-gcc 
  E      = .exe
  ICO    = ico.rco
endif
ifeq ($(DEST),lnxsdl) 
  TARGET = SIL_TARGET_LNXSDL
  CFLAGS =  -lSDL2main -lSDL2 -DSDL_MAIN_HANDLED 
  CC     = gcc
endif
ifeq ($(DEST),macsdl) 
  TARGET = SIL_TARGET_MACSDL
  CFLAGS =  -L/opt/homebrew/lib -I/opt/homebrew/include/ -lSDL2main -lSDL2 -DSDL_MAIN_HANDLED 
  CC     = gcc
endif
ifeq ($(DEST),x11) 
  TARGET = SIL_TARGET_X11
  CFLAGS = -lX11 
  CC    = cc
endif
ifeq ($(DEST),fb) 
  TARGET = SIL_TARGET_FB
  CC = cc
endif
DEBUG = -g
DEPS = ../src/sil.h ../src/sil_int.h ../src/log.h ../src/lodepng.h
OBJ = log.o sil.o framebuffer.o layer.o drawing.o filter.o lodepng.o font.o group.o display.o
CFLAGS +=-I ../src/


.PRECIOUS: %.o

%.o: ../src/%.c $(DEPS)
	$(CC) $(DEBUG) -c $< -o $@ $(CFLAGS) -D$(TARGET)

example_%: $(OBJ) 
	$(CC) $(DEBUG) -c $@.c -o $@.o $(CFLAGS) -D$(TARGET)
	$(CC) $(ICO) -o ../bin/$@$E $@.o $^ $(CFLAGS)

combined     : example_combined
filters      : example_filters
ordercopy    : example_ordercopy
draw         : example_draw
text         : example_text
basic        : example_basic
clickhandler : example_clickhandler

# example for simple single line compile, even without Makefile if you type out $(...) values
# this will compile "example_basic" to "example_simple" in bin directory
# dont forget to include the right -DSIL_TARGET_..  option, otherwise you might get linking 
# errors because of empty display.c..

simple: 
	$(CC) $(ICO) -o ../bin/example_simple ../src/log.c ../src/sil.c ../src/framebuffer.c ../src/layer.c ../src/drawing.c ../src/filter.c ../src/lodepng.c ../src/font.c ../src/group.c ../src/display.c example_basic.c $(CFLAGS) -D$(TARGET)

all: combined filters ordercopy draw text basic clickhandler

clean: 
	rm -rf ../../bin/example_* *.o 
