/*

   x11display.c CopyRight 2021 Remco Schellekens, see LICENSE for more details.

   This file contains all functions for displaying the layers on a X-Windows environment using standard
   X11 library.
   It will do so by using a framebuffer as source for a bitmap inside a fixed size window. Every time
   framebuffer is changed, bitmap is replaced.

   every "...display.c" file should have these functions
   -sil_initDisplay        ; create initial display, called via initializing SIL
   -sil_updateDisplay      ; update display, will check all layers updates display accordingly
   -sil_destroyDisplay     ; remove display, called via destroying SIL
   -sil_getEventDisplay    ; Wait or get first event ( mouse / keys or closing window )
   -sil_getTypefromDisplay ; will return the "native" color type of the display
   -sil_setTimerDisplay    ; will set a repeating timer to interrupt the wait loop
   -sil_stopTimerDisplay   ; stops the repeating timer
   -sil_setCursor          ; sets the mouse cursor (in windowed environments)


*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sil.h"
#include "log.h"


/* X11 display variables only used here */

typedef struct _GDISP {
  Atom      wm_delete_message;
  Display   *display;
  Window    window;
  GC        context;
  Visual    *visual;
  int       screen;
  XImage    *ximage;
  XGCValues gcvalues;
  SILFB     *fb;
  SILEVENT se;
  SILEVENT pressed[100];
  UINT keys;
  BYTE ctype;
  Cursor cursor;
  struct timeval lasttimer;
  struct timeval tval;
  int con;
} GDISP;

static GDISP gdisp;



/*****************************************************************************

  retrieve color type from Display (SILTYPE... , see framebuffer.c for info)
  Used as "default" when no type is given when creating framebuffer

 *****************************************************************************/

UINT sil_getTypefromDisplay() {
  return gdisp.fb->type;
}


/*****************************************************************************

  Ugly hack to prevent autorepeat maddness

 *****************************************************************************/

UINT was_it_auto_repeat(Display * d, XEvent * event, int current_type, int next_type){
  /*  Holding down a key will cause 'autorepeat' to send fake keyup/keydown events, but we want to ignore these: '*/
  if(event->type == current_type && XEventsQueued(d, QueuedAfterReading)){
    XEvent nev;
    XPeekEvent(d, &nev);
    if (nev.type == next_type && nev.xkey.time == event->xkey.time && nev.xkey.keycode == event->xkey.keycode) return 1;
  }
  return 0;
}

/*****************************************************************************

  Internal function to get status of special keys (shift,menu,control,caps)
  returns byte with bits set for given keys. Note: it doesn't distinguish
  between keys on the right or left side of keyboard

 *****************************************************************************/

static BYTE modifiers2sil(UINT state) {
  BYTE ret=0;
  if (state&ShiftMask)   ret|=SILKM_SHIFT;
  if (state&ControlMask) ret|=SILKM_CTRL;
  if (state&Mod1Mask)    ret|=SILKM_ALT;
  if (state&LockMask)    ret|=SILKM_CAPS;
  return ret;
}

/*****************************************************************************

  Internal function to X-Windows virtual key codes to SIL keycodes
  note that there will be no difference in left or right keys (ctrl,alt,shift)

 *****************************************************************************/

static UINT keycode2sil(UINT code) {

  switch(code) {
    case XK_BackSpace:  return SILKY_BACK;
    case XK_Tab:        return SILKY_TAB;
    case XK_Return:     return SILKY_ENTER;
    case XK_Shift_R:    return SILKY_SHIFT;
    case XK_Shift_L:    return SILKY_SHIFT;
    case XK_Control_L:  return SILKY_CTRL;
    case XK_Control_R:   return SILKY_CTRL;
    case XK_Alt_R:      return SILKY_ALT;
    case XK_Alt_L:      return SILKY_ALT;
    case XK_Pause:      return SILKY_PAUSE;
    case XK_Caps_Lock:  return SILKY_CAPS;
    case XK_Escape:     return SILKY_ESC;
    case XK_space:      return SILKY_SPACE;
    case XK_Prior:      return SILKY_PAGEUP;
    case XK_Next:       return SILKY_PAGEDOWN;
    case XK_End:        return SILKY_END;
    case XK_Home:       return SILKY_HOME;
    case XK_Left:       return SILKY_LEFT;
    case XK_Up:         return SILKY_UP;
    case XK_Right:      return SILKY_RIGHT;
    case XK_Down:       return SILKY_DOWN;
    case XK_Print:      return SILKY_PRINTSCREEN;
    case XK_Insert:     return SILKY_INSERT;
    case XK_Delete:     return SILKY_DELETE;
    case XK_0:          return SILKY_0;
    case XK_1:          return SILKY_1;
    case XK_2:          return SILKY_2;
    case XK_3:          return SILKY_3;
    case XK_4:          return SILKY_4;
    case XK_5:          return SILKY_5;
    case XK_6:          return SILKY_6;
    case XK_7:          return SILKY_7;
    case XK_8:          return SILKY_8;
    case XK_9:          return SILKY_9;
    case XK_a:          return SILKY_A;
    case XK_b:          return SILKY_B;
    case XK_c:          return SILKY_C;
    case XK_d:          return SILKY_D;
    case XK_e:          return SILKY_E;
    case XK_f:          return SILKY_F;
    case XK_g:          return SILKY_G;
    case XK_h:          return SILKY_H;
    case XK_i:          return SILKY_I;
    case XK_j:          return SILKY_J;
    case XK_k:          return SILKY_K;
    case XK_l:          return SILKY_L;
    case XK_m:          return SILKY_M;
    case XK_n:          return SILKY_N;
    case XK_o:          return SILKY_O;
    case XK_p:          return SILKY_P;
    case XK_q:          return SILKY_Q;
    case XK_r:          return SILKY_R;
    case XK_s:          return SILKY_S;
    case XK_t:          return SILKY_T;
    case XK_u:          return SILKY_U;
    case XK_v:          return SILKY_V;
    case XK_w:          return SILKY_W;
    case XK_x:          return SILKY_X;
    case XK_y:          return SILKY_Y;
    case XK_z:          return SILKY_Z;
    case XK_KP_0:         return SILKY_NUM0;
    case XK_KP_1:         return SILKY_NUM1;
    case XK_KP_2:         return SILKY_NUM2;
    case XK_KP_3:         return SILKY_NUM3;
    case XK_KP_4:         return SILKY_NUM4;
    case XK_KP_5:         return SILKY_NUM5;
    case XK_KP_6:         return SILKY_NUM6;
    case XK_KP_7:         return SILKY_NUM7;
    case XK_KP_8:         return SILKY_NUM8;
    case XK_KP_9:         return SILKY_NUM9;
    case XK_KP_Multiply:  return SILKY_NUMMULTIPLY;
    case XK_KP_Add:       return SILKY_NUMPLUS;
    case XK_KP_Subtract:  return SILKY_NUMMINUS;
    case XK_KP_Decimal:   return SILKY_NUMPOINT;
    case XK_KP_Divide:    return SILKY_NUMSLASH;
    case XK_F1:           return SILKY_F1;
    case XK_F2:           return SILKY_F2;
    case XK_F3:           return SILKY_F3;
    case XK_F4:           return SILKY_F4;
    case XK_F5:           return SILKY_F5;
    case XK_F6:           return SILKY_F6;
    case XK_F7:           return SILKY_F7;
    case XK_F8:           return SILKY_F8;
    case XK_F9:           return SILKY_F9;
    case XK_F10:          return SILKY_F10;
    case XK_F11:          return SILKY_F11;
    case XK_F12:          return SILKY_F12;
    case XK_Num_Lock:     return SILKY_NUMLOCK;
    case XK_Scroll_Lock:  return SILKY_SCROLLLOCK;
    case XK_semicolon:    return SILKY_SEMICOLON;
    case XK_plus:         return SILKY_PLUS;
    case XK_comma:        return SILKY_COMMA;
    case XK_minus:        return SILKY_MINUS;
    case XK_period:       return SILKY_PERIOD;
    case XK_slash:        return SILKY_SLASH;
    case XK_quoteleft:    return SILKY_ACUTE;
    case XK_bracketleft:  return SILKY_OPENBRACKET;
    case XK_backslash:    return SILKY_BACKSLASH;
    case XK_bracketright: return SILKY_CLOSEBRACKET;
    case XK_apostrophe:   return SILKY_APOSTROPHE;
  }
  return 0;
}

/*****************************************************************************

  Standard internal handlers for X11 window events & errors

 *****************************************************************************/
static int expose(XEvent *e) {
	Window root;
	int x, y;
	unsigned int border, depth, width, height;

  /* don't bother with old messages */
	if (e->xexpose.count != 0) return 0;

	Status s = XGetGeometry(gdisp.display, gdisp.window, &root, &x, &y, &width, &height,
			&border, &depth);
  /* can't do nothing if parameters are wrong, just exit */
	if (0==s)      log_fatal("XGetGeometry Failed");
  if (24!=depth) log_fatal("Colordepth isn't 24 bits RGB");
  sil_updateDisplay();
	return 0;
}

static int errorHandler(Display *d, XErrorEvent *e) {
	fprintf(stderr, "X11 error: display=%p event=%p\n", (void *)d, (void *)e);
	return 0;
}

static int fatalHandler(Display *d) {
	fprintf(stderr, "X11 fatal: display=%p\n", (void *)d);
	log_fatal("fatal X11 error");
	return 0;
}

/*****************************************************************************

  Get event from display
  depending on abilities to do so, setting "wait" on 1 will wait until event
  comes (blocking) and setting to zero will only poll.

  It will return a SILEVENT, containing:
   * type (SILDISP... types, from key-up/down till mouse events)
   * val => MOUSE_DOWN/UP: which button is pressed 1:left 2:middle 3:right
            MOUSEWHEEL   : direction of wheel 1:up 2:down
            KEY_DOWN/UP  : ascii value of key(-combination)

   * x,y       => position of mouse at that time
   * code      => 'native' keycode (in this case windows 'virtual keycode')
   * key       => translated 'common' SIL keycode
   * modifiers => byte with bits reflecting status of shift,alt,ctrl and caps

   Note that keycodes are -trying to- refferring to the pressed key,
   *not the ascii or unicode code that is printed on it *. To figure out if 'a'
   or 'A' is typed , check for keycode 'a' + modifiers "Shift" or just get the
   'val', where OS is trying to translate keycode to ascii.
   (Actually, unicode, but I'm lazy and only want single-byte codes ... )

   Warning: This code is ugly and support for keyboard events are shaky....

 *****************************************************************************/

SILEVENT *sil_getEventDisplay() {
	int stop= 0;
	XEvent event;
  char buf='\0';
  char keys_return[32];
  int downkeys=0;
  fd_set rs;
  struct timeval *tp;
  struct timeval tt,tv;

  do {
    /* another ugly hack to check if keys are *realy* pressed */
    XQueryKeymap(gdisp.display,keys_return);
    downkeys=0;
    for (int i=0;i<32;i++) 
      if (keys_return[i]) downkeys++;
    if (!downkeys) {
      if (gdisp.keys) {
        /* missed a KEY_UP event, lets just send it again */
        memcpy(&gdisp.se,&gdisp.pressed[--gdisp.keys],sizeof(gdisp.se));
        gdisp.se.type=SILDISP_KEY_UP;
        gdisp.se.modifiers=0;
        return &gdisp.se;
      } 
    }

    gdisp.se.type=SILDISP_NOTHING;
    gdisp.se.val=0;
    gdisp.se.x=0;
    gdisp.se.y=0;

    FD_ZERO(&rs);
    FD_SET(gdisp.con,&rs);

    if ((gdisp.tval.tv_sec>0)||(gdisp.tval.tv_usec>0)) {
      tt.tv_sec=gdisp.tval.tv_sec;
      tt.tv_usec=gdisp.tval.tv_usec;
      tp=&tt;
    } else {
      tp=NULL;
    }

    if (0==XPending(gdisp.display)) {
      if (0==select(gdisp.con+1,&rs,NULL,NULL,tp)) {
        /* timer expired */
        gdisp.se.type=SILDISP_TIMER;
        gdisp.se.code=666;
        gettimeofday(&tv,NULL);
        gdisp.se.val=(tv.tv_sec-gdisp.lasttimer.tv_sec)*1000+(tv.tv_usec-gdisp.lasttimer.tv_usec)/1000;
        gettimeofday(&gdisp.lasttimer,NULL);
        stop=1;
        break;
      }  else {
        XNextEvent(gdisp.display, &event);
      }
    } else {
     XNextEvent(gdisp.display, &event);
   }

    switch (event.type) {
      case ButtonPress:
        gdisp.se.type=SILDISP_MOUSE_DOWN;
        gdisp.se.val=0;
        switch (event.xbutton.button) {
           case Button1:
             gdisp.se.val=1;
             break;
           case Button2:
             gdisp.se.val=2;
             break;
           case Button3:
             gdisp.se.val=3;
             break;
           case Button4:
             gdisp.se.type=SILDISP_MOUSEWHEEL;
             gdisp.se.val=2;
             break;
           case Button5:
             gdisp.se.type=SILDISP_MOUSEWHEEL;
             gdisp.se.val=1;
           break;
        }
        gdisp.se.x  =event.xbutton.x;
        gdisp.se.y  =event.xbutton.y;
        stop=1;
        break;

        case ButtonRelease:
          gdisp.se.type=SILDISP_MOUSE_UP;
          gdisp.se.val=0;
          switch (event.xbutton.button) {
             case Button1:
               gdisp.se.val=1;
               break;
             case Button2:
               gdisp.se.val=2;
               break;
             case Button3:
               gdisp.se.val=3;
               break;
             case Button4:
             case Button5:
               gdisp.se.type=SILDISP_NOTHING;
               break;
          }
          if (gdisp.se.type!=SILDISP_NOTHING) {
            gdisp.se.x  =event.xbutton.x;
            gdisp.se.y  =event.xbutton.y;
            stop=1;
          }
          break;

        case MotionNotify:
          gdisp.se.type=SILDISP_MOUSE_MOVE;
          gdisp.se.x = event.xmotion.x;
          gdisp.se.y = event.xmotion.y;
          stop=1;
          break;

        case KeyPress:
          gdisp.se.type=SILDISP_KEY_DOWN;
          XLookupString(&event.xkey,&buf,1,NULL,NULL);
          gdisp.se.val=buf;
          gdisp.se.code=XLookupKeysym(&event.xkey,0);
          gdisp.se.key=keycode2sil(gdisp.se.code);
          gdisp.se.modifiers=modifiers2sil(event.xkey.state);
          if (gdisp.keys<100) {
            memcpy(&gdisp.pressed[gdisp.keys++],&gdisp.se,sizeof(gdisp.se));
          }
          stop=1;
          break;

        case KeyRelease:
          if (was_it_auto_repeat(gdisp.display,&event, KeyRelease,KeyPress)) {
            XNextEvent(gdisp.display, &event); /* ignore next event */
            stop=1;
          } else {
            gdisp.keys--;
            gdisp.se.type=SILDISP_KEY_UP;
            XLookupString(&event.xkey,&buf,1,NULL,NULL);
            gdisp.se.val=buf;
            gdisp.se.code=XLookupKeysym(&event.xkey,0);
            gdisp.se.key=keycode2sil(gdisp.se.code);
            gdisp.se.modifiers=modifiers2sil(event.xkey.state);
            /* weird X11 behaviour, grab keyboard focus back when keys are still being pressed */
            if (gdisp.keys) {
              gdisp.keys--;
              if (gdisp.keys) {
                XGrabKeyboard(gdisp.display,gdisp.window,False,GrabModeAsync,GrabModeAsync,CurrentTime);
              } else {
                XFlush(gdisp.display);
                XUngrabKeyboard(gdisp.display,CurrentTime);
              }
            }
            stop=1;
          } 
          break;

        case ClientMessage:
          if (event.xclient.data.l[0] == (int)gdisp.wm_delete_message) {
            gdisp.se.type=SILDISP_QUIT;
            stop=1;
          } 
          break;
        case Expose:
          expose(&event);
          stop=1;
          break;
      }
  } while (!stop);
	return &gdisp.se;
}

void sil_setTimerDisplay(UINT amount) {
  gettimeofday(&gdisp.lasttimer,NULL);
  gdisp.tval.tv_sec=amount/1000;
  gdisp.tval.tv_usec=(amount-gdisp.tval.tv_sec*1000)*1000;
  //gdisp.timerid=SDL_AddTimer(amount, &timercallback,NULL);
}

void sil_stopTimerDisplay() {
  gdisp.tval.tv_sec=0;
  gdisp.tval.tv_usec=0;
}

void sil_setCursor(BYTE type) {
  /* only load if cursor has been changed */
  if ((type!=gdisp.ctype)||(type!=SILCUR_ARROW)) {
    switch(type) {
      case SILCUR_ARROW:
        XFreeCursor(gdisp.display,gdisp.cursor);
        gdisp.cursor=XCreateFontCursor(gdisp.display, XC_arrow);
        break;
      case SILCUR_HAND:
        XFreeCursor(gdisp.display,gdisp.cursor);
        gdisp.cursor=XCreateFontCursor(gdisp.display, XC_hand2);
        break;
      case SILCUR_HELP:
        XFreeCursor(gdisp.display,gdisp.cursor);
        gdisp.cursor=XCreateFontCursor(gdisp.display, XC_question_arrow);
        break;
      case SILCUR_NO:
        XFreeCursor(gdisp.display,gdisp.cursor);
        gdisp.cursor=XCreateFontCursor(gdisp.display, XC_circle);
        break;
      case SILCUR_IBEAM:
        XFreeCursor(gdisp.display,gdisp.cursor);
        gdisp.cursor=XCreateFontCursor(gdisp.display, XC_xterm);
        break;
    }
  }
  if (gdisp.cursor) {
    gdisp.ctype=type;
    XDefineCursor(gdisp.display,gdisp.window,gdisp.cursor);
  }
}





/*****************************************************************************

  Initialize Display (called by initSIL) ignoring first option, only needed
  for windows/GDI
  other options are width,height and title of window

 *****************************************************************************/


UINT sil_initDisplay(void *dummy, UINT width, UINT height, char * title) {
  UINT ret;

  gdisp.display=NULL;
  gdisp.ximage =NULL;
  gdisp.keys=0;
  gdisp.ctype=SILCUR_ARROW;

  
  /* first, create a framebuffer where all layer will go to */
  gdisp.fb=sil_initFB(width, height, SILTYPE_ARGB);
  if (NULL==gdisp.fb) {
    log_info("ERR: Can't create framebuffer for display");
    return SILERR_NOTINIT;
  }


  /* set handlers */
	XSetErrorHandler(errorHandler);
	XSetIOErrorHandler(fatalHandler);

  /* connect to X11 server (make sure to set DISPLAY environment var ! )*/
	gdisp.display = XOpenDisplay(NULL);

	if (NULL==gdisp.display) log_fatal("cannot open display");

	XSizeHints *sizeHints = XAllocSizeHints();
	if (NULL==sizeHints) log_fatal("cannot allocate sizehints");


  /* just take first display */
	gdisp.screen = DefaultScreen(gdisp.display);

  /* create window */
	gdisp.window = XCreateSimpleWindow(gdisp.display, DefaultRootWindow(gdisp.display), 0, 0,
    width, height, 5, WhitePixel(gdisp.display, gdisp.screen),
    BlackPixel(gdisp.display, gdisp.screen));

  /* listen for delete messages (clicking on close window, or Xwindows stopping ) */
	gdisp.wm_delete_message = XInternAtom(gdisp.display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(gdisp.display, gdisp.window, &gdisp.wm_delete_message, 1);

	sizeHints->flags = PMinSize;
	sizeHints->min_width = width;
	sizeHints->min_height = height;

	Xutf8SetWMProperties(gdisp.display,gdisp.window,title,title,NULL,0,sizeHints,NULL,NULL);
	XFree(sizeHints);

  /* tell them we need keys,pointers & button events */
	XSelectInput(gdisp.display, gdisp.window, ExposureMask | KeyPressMask | PointerMotionMask| ButtonPressMask | ButtonReleaseMask);
	gdisp.context = XCreateGC(gdisp.display, gdisp.window, 0, 0);

  /* raise the window */
	XMapRaised(gdisp.display, gdisp.window);

  /* set the default cursor */
  gdisp.cursor=XCreateFontCursor(gdisp.display, XC_arrow);
  if (gdisp.cursor) XDefineCursor(gdisp.display,gdisp.window,gdisp.cursor);

  /* store connection number to display */
  gdisp.con=XConnectionNumber(gdisp.display);

  gdisp.visual=DefaultVisual(gdisp.display,gdisp.screen);
  return SILERR_ALLOK;
}

/*****************************************************************************

  Update Display

 *****************************************************************************/

void sil_updateDisplay() {
  GC gc;

	if (NULL==gdisp.fb) log_fatal("framebuffer not initialized");
  LayersToFB(gdisp.fb);
  
  /* create colormap */
  gc=XCreateGC(gdisp.display, gdisp.window, 0, &gdisp.gcvalues);

  /* create image from framebuffer */
  gdisp.ximage = XCreateImage(gdisp.display,gdisp.visual,24,ZPixmap,0,(char *)gdisp.fb->buf, gdisp.fb->width,gdisp.fb->height, 16,0);

  /* RGBA like intel platforms */
  gdisp.ximage->byte_order=LSBFirst;

  /* place image on screen */
  XPutImage(gdisp.display,gdisp.window,gc,gdisp.ximage,0,0,0,0, gdisp.fb->width, gdisp.fb->height);
}

/*****************************************************************************

  Destroy display information (called by destroy SIL, to cleanup everything )

 *****************************************************************************/

void sil_destroyDisplay() {
	if (NULL != gdisp.display) {
		XCloseDisplay(gdisp.display);
	}
  if (NULL!=gdisp.fb) sil_destroyFB(gdisp.fb);
}
