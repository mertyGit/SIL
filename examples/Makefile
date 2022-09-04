# Make one choice of target list below

# winsdl = Windows + SDL 2.0.20 or later
# lnxsdl = Linux + SDL 2.0.20 or later
# macsdl = Mac OS X + SDL 2.0.20 or later
# gdi    = Windows native GDI
# x11    = Linux + X11 xlib
# fb     = linux /dev/fb0 + /dev/input/event2 , like raspberry + touchscreen
DEST=winsdl

# flags to remove parts of SIL 

# Don't use math library (drops support for sil_rotate... functions)
# SIL_NO_MATH = 1

# ignore any Logging (or use your own) 
# log.c and log.h still has to be included, but will become empty functions
# SIL_NO_LOG  = 1

# don't compile drawing primitives. Only SetPixelLayer, puPixelLKayer, 
# paintLayer and PNGtoNewLayer can be called
# you can skip compiling/including "drawing.c" 
# SIL_NO_DRAWING = 1

# don't compile font/text routines
# you can skip compiling/including "font.c" 
# SIL_NO_FONT = 1

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
OBJ = log.o sil.o framebuffer.o layer.o drawing.o filter.o lodepng.o font.o display.o
CFLAGS +=-I../src/ -L../src/

ifndef SIL_NO_MATH
  CFLAGS +=-lm
else
  CFLAGS +=-DSIL_NO_MATH
endif

ifdef SIL_NO_LOG
  CFLAGS +=-DSIL_NO_LOG
endif

ifdef SIL_NO_DRAWING
  CFLAGS +=-DSIL_NO_DRAWING
endif

ifdef SIL_NO_FONT
  CFLAGS +=-DSIL_NO_FONT
endif
 
 
 


#.PRECIOUS: %.o

%.o: ../src/%.c $(DEPS)
	$(CC) $(DEBUG) -c $< -o $@ $(CFLAGS) -D$(TARGET)

example_%: $(OBJ) 
	$(CC) $(DEBUG) -c $@.c -o $@.o $(CFLAGS) -D$(TARGET)
	$(CC) $(ICO) -o ../bin/$@$E $@.o $^ $(CFLAGS) -D$(TARGET)

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
	$(CC) $(ICO) -o ../bin/example_simple ../src/log.c ../src/sil.c ../src/framebuffer.c ../src/layer.c ../src/filter.c ../src/lodepng.c ../src/display.c example_basic.c $(CFLAGS) -D$(TARGET) -DSIL_NO_MATH -DSIL_NO_LOG -DSIL_NO_DRAWING -DSIL_NO_FONT

all: combined filters ordercopy draw text basic clickhandler

clean: 
	rm -rf ../../bin/example_* *.o 
	rm -rf ../../src/*.o 
	rm -rf ../../exmaples/*.o 
