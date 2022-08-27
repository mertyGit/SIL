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
  int oldx;
  int oldy;
} GDISP ;

static GDISP gv;



/*****************************************************************************

  retrieve color type from Display (SILTYPE... , see framebuffer.c for info)
  Used as "default" when no type is given when creating framebuffer

 *****************************************************************************/

UINT sil_getTypefromDisplay() {
  return gv.fb->type;
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

	Status s = XGetGeometry(gv.display, gv.window, &root, &x, &y, &width, &height,
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

BYTE sil_getMouse(int *x,int *y) {
  Window root, child;
  int rx, ry,wx,wy;
  unsigned int mask;
  BYTE ret=0;

  XQueryPointer(gv.display,gv.window,&root,&child,&rx,&ry,&wx,&wy,&mask);
  if (mask&(1<< 8)) ret|=SIL_BTN_LEFT;
  if (mask&(1<< 9)) ret|=SIL_BTN_MIDDLE;
  if (mask&(1<<10)) ret|=SIL_BTN_RIGHT;
  if (x) *x=wx;
  if (y) *y=wx;

  return ret;
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
    XQueryKeymap(gv.display,keys_return);
    downkeys=0;
    for (int i=0;i<32;i++) 
      if (keys_return[i]) downkeys++;
    if (!downkeys) {
      if (gv.keys) {
        /* missed a KEY_UP event, lets just send it again */
        memcpy(&gv.se,&gv.pressed[--gv.keys],sizeof(gv.se));
        gv.se.type=SILDISP_KEY_UP;
        gv.se.modifiers=0;
        return &gv.se;
      } 
    }

    gv.se.type=SILDISP_NOTHING;
    gv.se.val=0;
    gv.se.x=0;
    gv.se.y=0;

    FD_ZERO(&rs);
    FD_SET(gv.con,&rs);

    if ((gv.tval.tv_sec>0)||(gv.tval.tv_usec>0)) {
      tt.tv_sec=gv.tval.tv_sec;
      tt.tv_usec=gv.tval.tv_usec;
      tp=&tt;
    } else {
      tp=NULL;
    }

    if (0==XPending(gv.display)) {
      if (0==select(gv.con+1,&rs,NULL,NULL,tp)) {
        /* timer expired */
        gv.se.type=SILDISP_TIMER;
        gv.se.code=666;
        gettimeofday(&tv,NULL);
        gv.se.val=(tv.tv_sec-gv.lasttimer.tv_sec)*1000+(tv.tv_usec-gv.lasttimer.tv_usec)/1000;
        gettimeofday(&gv.lasttimer,NULL);
        stop=1;
        break;
      }  else {
        XNextEvent(gv.display, &event);
      }
    } else {
     XNextEvent(gv.display, &event);
   }

    switch (event.type) {
      case ButtonPress:
        gv.se.type=SILDISP_MOUSE_DOWN;
        gv.se.val=0;
        switch (event.xbutton.button) {
           case Button1:
             gv.se.val=1;
             break;
           case Button2:
             gv.se.val=2;
             break;
           case Button3:
             gv.se.val=3;
             break;
           case Button4:
             gv.se.type=SILDISP_MOUSEWHEEL;
             gv.se.val=2;
             break;
           case Button5:
             gv.se.type=SILDISP_MOUSEWHEEL;
             gv.se.val=1;
           break;
        }
        gv.se.x  =event.xbutton.x;
        gv.se.y  =event.xbutton.y;
        stop=1;
        break;

        case ButtonRelease:
          gv.se.type=SILDISP_MOUSE_UP;
          gv.se.val=0;
          switch (event.xbutton.button) {
             case Button1:
               gv.se.val=1;
               break;
             case Button2:
               gv.se.val=2;
               break;
             case Button3:
               gv.se.val=3;
               break;
             case Button4:
             case Button5:
               gv.se.type=SILDISP_NOTHING;
               break;
          }
          if (gv.se.type!=SILDISP_NOTHING) {
            gv.se.x  =event.xbutton.x;
            gv.se.y  =event.xbutton.y;
            stop=1;
          }
          break;

        case MotionNotify:
          gv.se.type=SILDISP_MOUSE_MOVE;
          gv.se.x = event.xmotion.x;
          gv.se.y = event.xmotion.y;
          gv.se.dx = gv.se.x-gv.oldx;
          gv.se.dy = gv.se.y-gv.oldy;
          gv.oldx=gv.se.x;
          gv.oldy=gv.se.y;
          stop=1;
          break;

        case KeyPress:
          gv.se.type=SILDISP_KEY_DOWN;
          XLookupString(&event.xkey,&buf,1,NULL,NULL);
          gv.se.val=buf;
          gv.se.code=XLookupKeysym(&event.xkey,0);
          gv.se.key=keycode2sil(gv.se.code);
          gv.se.modifiers=modifiers2sil(event.xkey.state);
          if (gv.keys<100) {
            memcpy(&gv.pressed[gv.keys++],&gv.se,sizeof(gv.se));
          }
          stop=1;
          break;

        case KeyRelease:
          if (was_it_auto_repeat(gv.display,&event, KeyRelease,KeyPress)) {
            XNextEvent(gv.display, &event); /* ignore next event */
            stop=1;
          } else {
            gv.keys--;
            gv.se.type=SILDISP_KEY_UP;
            XLookupString(&event.xkey,&buf,1,NULL,NULL);
            gv.se.val=buf;
            gv.se.code=XLookupKeysym(&event.xkey,0);
            gv.se.key=keycode2sil(gv.se.code);
            gv.se.modifiers=modifiers2sil(event.xkey.state);
            /* weird X11 behaviour, grab keyboard focus back when keys are still being pressed */
            if (gv.keys) {
              gv.keys--;
              if (gv.keys) {
                XGrabKeyboard(gv.display,gv.window,False,GrabModeAsync,GrabModeAsync,CurrentTime);
              } else {
                XFlush(gv.display);
                XUngrabKeyboard(gv.display,CurrentTime);
              }
            }
            stop=1;
          } 
          break;

        case ClientMessage:
          if (event.xclient.data.l[0] == (int)gv.wm_delete_message) {
            gv.se.type=SILDISP_QUIT;
            stop=1;
          } 
          break;
        case Expose:
          expose(&event);
          stop=1;
          break;
      }
  } while (!stop);
	return &gv.se;
}

void sil_setTimerDisplay(UINT amount) {
  gettimeofday(&gv.lasttimer,NULL);
  gv.tval.tv_sec=amount/1000;
  gv.tval.tv_usec=(amount-gv.tval.tv_sec*1000)*1000;
  //gv.timerid=SDL_AddTimer(amount, &timercallback,NULL);
}

void sil_stopTimerDisplay() {
  gv.tval.tv_sec=0;
  gv.tval.tv_usec=0;
}

void sil_setCursor(BYTE type) {
  /* only load if cursor has been changed */
  if ((type!=gv.ctype)||(type!=SILCUR_ARROW)) {
    switch(type) {
      case SILCUR_ARROW:
        XFreeCursor(gv.display,gv.cursor);
        gv.cursor=XCreateFontCursor(gv.display, XC_arrow);
        break;
      case SILCUR_HAND:
        XFreeCursor(gv.display,gv.cursor);
        gv.cursor=XCreateFontCursor(gv.display, XC_hand2);
        break;
      case SILCUR_HELP:
        XFreeCursor(gv.display,gv.cursor);
        gv.cursor=XCreateFontCursor(gv.display, XC_question_arrow);
        break;
      case SILCUR_NO:
        XFreeCursor(gv.display,gv.cursor);
        gv.cursor=XCreateFontCursor(gv.display, XC_circle);
        break;
      case SILCUR_IBEAM:
        XFreeCursor(gv.display,gv.cursor);
        gv.cursor=XCreateFontCursor(gv.display, XC_xterm);
        break;
    }
  }
  if (gv.cursor) {
    gv.ctype=type;
    XDefineCursor(gv.display,gv.window,gv.cursor);
  }
}





/*****************************************************************************

  Initialize Display (called by initSIL) ignoring first option, only needed
  for windows/GDI
  other options are width,height and title of window

 *****************************************************************************/


UINT sil_initDisplay(void *dummy, UINT width, UINT height, char * title) {
  UINT ret;

  gv.display=NULL;
  gv.ximage =NULL;
  gv.keys=0;
  gv.ctype=SILCUR_ARROW;
  gv.oldx=0;
  gv.oldy=0;

  
  /* first, create a framebuffer where all layer will go to */
  gv.fb=sil_initFB(width, height, SILTYPE_ARGB);
  if (NULL==gv.fb) {
    log_info("ERR: Can't create framebuffer for display");
    return SILERR_NOTINIT;
  }


  /* set handlers */
	XSetErrorHandler(errorHandler);
	XSetIOErrorHandler(fatalHandler);

  /* connect to X11 server (make sure to set DISPLAY environment var ! )*/
	gv.display = XOpenDisplay(NULL);

	if (NULL==gv.display) log_fatal("cannot open display");

	XSizeHints *sizeHints = XAllocSizeHints();
	if (NULL==sizeHints) log_fatal("cannot allocate sizehints");


  /* just take first display */
	gv.screen = DefaultScreen(gv.display);

  /* create window */
	gv.window = XCreateSimpleWindow(gv.display, DefaultRootWindow(gv.display), 0, 0,
    width, height, 5, WhitePixel(gv.display, gv.screen),
    BlackPixel(gv.display, gv.screen));

  /* listen for delete messages (clicking on close window, or Xwindows stopping ) */
	gv.wm_delete_message = XInternAtom(gv.display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(gv.display, gv.window, &gv.wm_delete_message, 1);

	sizeHints->flags = PMinSize;
	sizeHints->min_width = width;
	sizeHints->min_height = height;

	Xutf8SetWMProperties(gv.display,gv.window,title,title,NULL,0,sizeHints,NULL,NULL);
	XFree(sizeHints);

  /* tell them we need keys,pointers & button events */
	XSelectInput(gv.display, gv.window, ExposureMask | KeyPressMask | PointerMotionMask| ButtonPressMask | ButtonReleaseMask);
	gv.context = XCreateGC(gv.display, gv.window, 0, 0);

  /* raise the window */
	XMapRaised(gv.display, gv.window);

  /* set the default cursor */
  gv.cursor=XCreateFontCursor(gv.display, XC_arrow);
  if (gv.cursor) XDefineCursor(gv.display,gv.window,gv.cursor);

  /* store connection number to display */
  gv.con=XConnectionNumber(gv.display);

  gv.visual=DefaultVisual(gv.display,gv.screen);
  return SILERR_ALLOK;
}

/*****************************************************************************

  Update Display

 *****************************************************************************/

void sil_updateDisplay() {
  GC gc;

	if (NULL==gv.fb) log_fatal("framebuffer not initialized");
  LayersToFB(gv.fb);
  
  /* create colormap */
  gc=XCreateGC(gv.display, gv.window, 0, &gv.gcvalues);

  /* create image from framebuffer */
  gv.ximage = XCreateImage(gv.display,gv.visual,24,ZPixmap,0,(char *)gv.fb->buf, gv.fb->width,gv.fb->height, 16,0);

  /* RGBA like intel platforms */
  gv.ximage->byte_order=LSBFirst;

  /* place image on screen */
  XPutImage(gv.display,gv.window,gc,gv.ximage,0,0,0,0, gv.fb->width, gv.fb->height);
}

/*****************************************************************************

  Destroy display information (called by destroy SIL, to cleanup everything )

 *****************************************************************************/

void sil_destroyDisplay() {
	if (NULL != gv.display) {
		XCloseDisplay(gv.display);
	}
  if (NULL!=gv.fb) sil_destroyFB(gv.fb);
}
