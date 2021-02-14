#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sil.h"
#include "log.h"


/* X11 display variables only used here */
static Atom      wm_delete_message;
static Display   *display;
static Window    window;
static GC        context;
static Visual    *visual;
static int       screen;
static XImage    *ximage;
static XGCValues gcvalues;
static SILFB     *fb;
static SILEVENT se;
static SILEVENT pressed[100];
static UINT keys=0;

UINT sil_getTypefromDisplay() {
  return fb->type;
}


UINT was_it_auto_repeat(Display * d, XEvent * event, int current_type, int next_type){
  /*  Holding down a key will cause 'autorepeat' to send fake keyup/keydown events, but we want to ignore these: '*/
  if(event->type == current_type && XEventsQueued(d, QueuedAfterReading)){
    XEvent nev;
    XPeekEvent(d, &nev);
    if (nev.type == next_type && nev.xkey.time == event->xkey.time && nev.xkey.keycode == event->xkey.keycode) return 1;
  }
  return 0;
}


static BYTE modifiers2sil(UINT state) {
  BYTE ret=0;
  if (state&ShiftMask)   ret|=SILKM_SHIFT;
  if (state&ControlMask) ret|=SILKM_CTRL;
  if (state&Mod1Mask)    ret|=SILKM_ALT;
  if (state&LockMask)    ret|=SILKM_CAPS;
  return ret;
}

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


static void LayersToDisplay() {
  SILLYR *layer=sil_getBottomLayer();
  BYTE red,green,blue,alpha;
  BYTE mixred,mixgreen,mixblue,mixalpha;
  UINT pos,pos2;
  float af;
  float negaf;
  SILBOX rview;
  int absx,absy;

  sil_clearFB(fb);
  while (layer) {
    if (!(layer->flags&SILFLAG_INVISIBLE)) {
      for (int x=layer->view.minx; x<layer->view.maxx; x++) {
        for (int y=layer->view.miny; y<layer->view.maxy; y++) {
          int absx=x+layer->relx-layer->view.minx;
          int absy=y+layer->rely-layer->view.miny;
          int rx=x;
          int ry=y;

          /* prevent drawing outside borders of display */
          if ((absx>=fb->width)||(absy>=fb->height)) continue;

          sil_getPixelLayer(layer,rx,ry,&red,&green,&blue,&alpha);
          if (0==alpha) continue; /* nothing to do if completely transparant */
          alpha=alpha*layer->alpha;
          if (255==alpha) {
            sil_putPixelFB(fb,absx,absy,red,green,blue,255);
          } else {
            sil_getPixelFB(fb,absx,absy,&mixred,&mixgreen,&mixblue,&mixalpha);
            af=((float)alpha)/255;
            negaf=1-af;
            red=red*af+negaf*mixred;
            green=green*af+negaf*mixgreen;
            blue=blue*af+negaf*mixblue;
            sil_putPixelFB(fb,absx,absy,red,green,blue,255);
          }
        }
      }
    }
    layer=layer->next;
  }
}



/* Standard handlers for X11 window events & errors */

static int expose(XEvent *e) {
	Window root;
	int x, y;
	unsigned int border, depth, width, height;

  /* don't bother with old messages */
	if (e->xexpose.count != 0) return 0;

	Status s = XGetGeometry(display, window, &root, &x, &y, &width, &height,
			&border, &depth);
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

SILEVENT *sil_getEventDisplay(BYTE wait) {
	int stop= 0;
	XEvent event;
  char buf='\0';
  char keys_return[32];
  int downkeys=0;

  do {
    XQueryKeymap(display,keys_return);
    downkeys=0;
    for (int i=0;i<32;i++) 
      if (keys_return[i]) downkeys++;
    if (!downkeys) {
      if (keys) {
        /* missed a KEY_UP event, lets just send it again */
        memcpy(&se,&pressed[--keys],sizeof(se));
        se.type=SILDISP_KEY_UP;
        se.modifiers=0;
        return &se;
      } 
    }

    se.type=SILDISP_NOTHING;
    se.val=0;
    se.x=0;
    se.y=0;
    while ((XPending(display) > 0)&&(!stop)) {
      XNextEvent(display, &event);

      switch (event.type) {
        case ButtonPress:
          se.type=SILDISP_MOUSE_DOWN;
          se.val=0;
          switch (event.xbutton.button) {
             case Button1:
               se.val=1;
               break;
             case Button2:
               se.val=2;
               break;
             case Button3:
               se.val=3;
               break;
             case Button4:
               se.type=SILDISP_MOUSEWHEEL;
               se.val=2;
               break;
             case Button5:
               se.type=SILDISP_MOUSEWHEEL;
               se.val=1;
             break;
          }
          se.x  =event.xbutton.x;
          se.y  =event.xbutton.y;
          stop=1;
          break;

        case ButtonRelease:
          se.type=SILDISP_MOUSE_UP;
          se.val=0;
          switch (event.xbutton.button) {
             case Button1:
               se.val=1;
               break;
             case Button2:
               se.val=2;
               break;
             case Button3:
               se.val=3;
               break;
             case Button4:
             case Button5:
             /* ignore "button up" of scrollwheel */
               se.type=SILDISP_NOTHING;
               break;
          }
          if (se.type!=SILDISP_NOTHING) {
            se.x  =event.xbutton.x;
            se.y  =event.xbutton.y;
            stop=1;
          }
          break;

        case MotionNotify:
          se.type=SILDISP_MOUSE_MOVE;
          se.x = event.xmotion.x;
          se.y = event.xmotion.y;
          stop=1;
          break;

        case KeyPress:
          se.type=SILDISP_KEY_DOWN;
          XLookupString(&event.xkey,&buf,1,NULL,NULL);
          se.val=buf;
          se.code=XLookupKeysym(&event.xkey,0);
          se.key=keycode2sil(se.code);
          se.modifiers=modifiers2sil(event.xkey.state);
          if (keys<100) {
            memcpy(&pressed[keys++],&se,sizeof(se));
          }
          stop=1;
          break;

        case KeyRelease:
          if (was_it_auto_repeat(display,&event, KeyRelease,KeyPress)) {
            XNextEvent(display, &event); /* ignore next event */
            stop=1;
          } else {
            keys--;
            se.type=SILDISP_KEY_UP;
            XLookupString(&event.xkey,&buf,1,NULL,NULL);
            se.val=buf;
            se.code=XLookupKeysym(&event.xkey,0);
            se.key=keycode2sil(se.code);
            se.modifiers=modifiers2sil(event.xkey.state);
            /* weird X11 behaviour, grab keyboard focus back when keys are still being pressed */
            if (keys) {
              keys--;
              if (keys) {
                XGrabKeyboard(display,window,False,GrabModeAsync,GrabModeAsync,CurrentTime);
              } else {
                XFlush(display);
                XUngrabKeyboard(display,CurrentTime);
              }
            }
            stop=1;
          } 
          break;

        case ClientMessage:
          if (event.xclient.data.l[0] == (int)wm_delete_message) {
            se.type=SILDISP_QUIT;
            stop=1;
          } 
          break;
        case Expose:
          expose(&event);
          stop=1;
          break;
      }
    }
  } while (!stop);
	return &se;
}

UINT sil_initDisplay(void *dummy, UINT width, UINT height, char * title) {
  display=NULL;
  ximage =NULL;
  UINT ret;

  fb=sil_initFB(width, height, SILTYPE_ARGB);
  if (NULL==fb) {
    log_info("ERR: Can't create framebuffer for display");
    sil_setErr(SILERR_NOTINIT);
    return SILERR_NOTINIT;
  }


	XSetErrorHandler(errorHandler);
	XSetIOErrorHandler(fatalHandler);

	display = XOpenDisplay(NULL);
	if (NULL==display) log_fatal("cannot open display");

	XSizeHints *sizeHints = XAllocSizeHints();
	if (NULL==sizeHints) log_fatal("cannot allocate sizehints");


	screen = DefaultScreen(display);

	window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0,
    width, height, 5, WhitePixel(display, screen),
    BlackPixel(display, screen));

	wm_delete_message = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, window, &wm_delete_message, 1);

	sizeHints->flags = PMinSize;
	sizeHints->min_width = width;
	sizeHints->min_height = height;

	Xutf8SetWMProperties(display,window,title,title,NULL,0,sizeHints,NULL,NULL);
	XFree(sizeHints);
	XSelectInput(display, window, ExposureMask | KeyPressMask | PointerMotionMask| ButtonPressMask | ButtonReleaseMask);
	context = XCreateGC(display, window, 0, 0);
	XMapRaised(display, window);

  visual=DefaultVisual(display,screen);
  sil_setErr(SILERR_NOTINIT);
  return SILERR_ALLOK;
}

void sil_updateDisplay() {
  GC gc;

  LayersToDisplay();
	if (NULL==fb) log_fatal("framebuffer not initialized");
  gc=XCreateGC(display, window, 0, &gcvalues);
  //printf("first pixels are: %d,%d,%d\n",fb.buf[0],fb.buf[1],fb.buf[2]);
  ximage = XCreateImage(display,visual,24,ZPixmap,0,(char *)fb->buf, fb->width,fb->height, 16,0);
  ximage->byte_order=LSBFirst;
  XPutImage(display,window,gc,ximage,0,0,0,0, fb->width, fb->height);
}

void sil_destroyDisplay() {
	if (NULL != display) {
		XCloseDisplay(display);
	}
  if (NULL!=fb) sil_destroyFB(fb);
}
