/*

   winSDLdisplay.c CopyRight 2021 Remco Schellekens, see LICENSE for more details.

   This file contains all functions for displaying the layers on a Windows environment, using SDL
   libary.
   Every updated layer(-framebuffer) will be translated to an texture that can be loaded into
   the gpu and mapped on eachother, using the power of hardware acceleration.

   every "...display.c" file should have these functions
   -sil_initDisplay        ; create initial display, called via initializing SIL
   -sil_updateDisplay      ; update display, will check all layers updates display accordingly
   -sil_destroyDisplay     ; remove display, called via destroying SIL
   -sil_getEventDisplay    ; Wait or get first event ( mouse / keys or closing window )
   -sil_getTypefromDisplay ; will return the "native" color type of the display
   -sil_setTimerDisplay    ; will set a repeating timer to interrupt the wait loop
   -sil_stopTimerDisplay   ; stops the repeating timer
   -sil_setCursor          ; sets the mouse cursor (in windowed environments)
   -sil_getMouse           ; returns mouse pointer location and buttons pressed


*/

#include <SDL2/SDL.h> 
#include <stdio.h>
#include "sil.h"
#include "sil_int.h"
#include "log.h"
#include "sys/time.h"


typedef struct _GDISP {
  SDL_Window *window;
  SDL_Event event;
  SDL_Renderer *renderer;
  SILFB *scratch;
  SILEVENT se;
  UINT txt;
  BYTE ctype;
  struct timeval lasttimer;
  SDL_Cursor *cursor;
  SDL_TimerID timerid;
  int width;
  int height;
  void (*syshandler)(SILEVENT *);
} GDISP; 

static GDISP gv;

/*****************************************************************************

  retrieve color type from Display (SILTYPE... , see framebuffer.c for info)
  Used as "default" when no type is given when creating framebuffer

 *****************************************************************************/

UINT sil_getTypefromDisplay() {
  return SILTYPE_ARGB;
}

void sil_getWindowSize(int *w, int *h) {
  if (w) *w=gv.width;
  if (h) *h=gv.height;
}


/*****************************************************************************
  
  All other non-SDL display functions uses LayersToFB function to put all 
  layers inside a single FB and update the display. However, with SDL we 
  already have all layers as textures inside GPU, so in this case we have to
  update / add missing textures if needed and just place them on top of each
  other...
  Note that it will be slower when we are not using SILTYPE_ARGB as type

 *****************************************************************************/

static void LayersToDisplay() {
  SDL_Rect SR,DR;
  UINT scratchw,scratchh;
  BYTE red,green,blue,alpha;

  SILLYR *layer=sil_getBottom();
  SDL_RenderClear(gv.renderer);
  /* loop from bottom to top layer */
  while (layer) {
    if (layer->fb->resized) {
      if (layer->texture) SDL_DestroyTexture(layer->texture);
      layer->texture=NULL;
    }
    if (NULL==layer->texture) {
      /* no texture yet for this layer */
      layer->texture=SDL_CreateTexture(gv.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,layer->fb->width,layer->fb->height);
      if (NULL==layer->texture) {
        printf("Warning: can't create texture for layer: %s\n", SDL_GetError());
        layer=layer->next;
        continue;
      }
      SDL_SetTextureBlendMode(layer->texture,SDL_BLENDMODE_BLEND);
    }
    /* normally, we would alter every alpha value when copying pixels to destination framebuffer    */
    /* however, we don't do framebuffer handling directly, so we have to override it some other way */
    if (layer->internal&SILFLAG_ALPHACHANGED) {
      SDL_SetTextureAlphaMod(layer->texture,(BYTE) (layer->alpha*255));
      layer->internal^=SILFLAG_ALPHACHANGED;
    }
    if (!(layer->flags&SILFLAG_INVISIBLE)) {
      if (layer->fb->changed) {
        if (layer->fb->type==SILTYPE_ARGB) {
          SDL_UpdateTexture(layer->texture,NULL,layer->fb->buf,(layer->fb->width)*4);
        } else {
          /* not ARGB , convert it to ARGB                                                 */
          /* use scratch buffer, but to do so, alter its width & height temporarly         */
          
          /* if scratch isn't large enough to fit layer, recreate a new one */
          if ((layer->fb->width>gv.scratch->width)||(layer->fb->height>gv.scratch->height)) {
            sil_destroyFB(gv.scratch);
            gv.scratch=sil_initFB(layer->fb->width,layer->fb->height,SILTYPE_ARGB);
            if (NULL==gv.scratch) {
              log_info("ERR: Can't create resized scratch framebuffer for display");
              return;
            }
          }

          /* we adjust width height temporary for smaller framebufs */
          scratchw=gv.scratch->width;
          scratchh=gv.scratch->height;

          if (gv.scratch->width>layer->fb->width) gv.scratch->width=layer->fb->width;
          if (gv.scratch->height>layer->fb->height) gv.scratch->height=layer->fb->height;
          for (UINT x=0;x<gv.scratch->width;x++) {
            for (UINT y=0;y<gv.scratch->height;y++) {
                sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha);            
                sil_putPixelFB(gv.scratch,x,y,red,green,blue,alpha);
            }
          }
          SDL_UpdateTexture(layer->texture,NULL,gv.scratch->buf,gv.scratch->width*4);

          /* ...and we set the dimensions back to latest size.. */
          gv.scratch->width=scratchw;
          gv.scratch->height=scratchh;
        }
      }
      SR.x=layer->view.minx;
      SR.y=layer->view.miny;
      SR.w=layer->view.width;
      SR.h=layer->view.height;
      DR.x=layer->relx;
      DR.y=layer->rely;
      DR.w=SR.w;
      DR.h=SR.h;
      SDL_RenderCopy(gv.renderer,layer->texture,&SR,&DR);
    }
    layer=layer->next;
  }

  /* clear all fb->changed flags, noting that all textures are updated  */
  layer=sil_getBottom();
  while(layer) {
    layer->fb->changed=0;
    layer->fb->resized=0;
    layer=layer->next;
  }
}

/*****************************************************************************

  Initialize Display (called by initSIL) 
  first argument is ignored (only needed for GDI one)
  rest is width,height and title of generated window

 *****************************************************************************/


UINT sil_initDisplay(void *nop, UINT width, UINT height, char *title) {

  /* initialize global vars */
  gv.window=NULL;
  gv.renderer=NULL;
  gv.scratch=NULL;
  gv.txt=0;
  gv.ctype=0;

  /* initialize SDL */
  SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO);


  /* Create window */
  gv.window=SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,width,height,0);
  if (NULL==gv.window) {
    printf("ERROR: can't create window for display: %s\n",SDL_GetError());
    return SILERR_NOTINIT;
  }

  /* create "scratch" FB, used for conversion to/from non BGRA layers */
  gv.scratch=sil_initFB(width,height,SILTYPE_ARGB);
  if (NULL==gv.scratch) {
    log_info("ERR: Can't create scratch framebuffer for display");
    return SILERR_NOTINIT;
  }

  /* Raise created window */
  SDL_SetWindowAlwaysOnTop(gv.window, SDL_TRUE);
  SDL_SetWindowAlwaysOnTop(gv.window, SDL_FALSE);

  /* Set resizable */
  SDL_SetWindowResizable(gv.window, SDL_TRUE);

  /* Create hardware renderer, used for placing textures on it */
  gv.renderer=SDL_CreateRenderer(gv.window, -1, SDL_RENDERER_ACCELERATED  );
  if (NULL==gv.renderer) {
      printf("Could not create renderer: %s\n", SDL_GetError());
      return SILERR_NOTINIT;
  }

  /* We want alpha working in layers */
  SDL_SetRenderDrawBlendMode(gv.renderer,SDL_BLENDMODE_BLEND);
  return SILERR_ALLOK;
}

/*****************************************************************************

  Update Display

 *****************************************************************************/
void sil_updateDisplay() {
  LayersToDisplay();
  SDL_RenderPresent(gv.renderer);
}

/*****************************************************************************

  Internal function to get status of special keys (shift,menu,control,caps)
  returns byte with bits set for given keys. Note: it doesn't distinguish
  between keys on the right or left side of keyboard

 *****************************************************************************/
BYTE sil_getModifiers() {
  SDL_Keymod mod;
  BYTE ret=0;
  mod=SDL_GetModState();
  if (mod&(KMOD_LSHIFT|KMOD_RSHIFT)) ret|=SILKM_SHIFT;
  if (mod&(KMOD_LCTRL|KMOD_RCTRL)) ret|=SILKM_CTRL;
  if (mod&(KMOD_LALT|KMOD_RALT)) ret|=SILKM_ALT;
  if (mod&KMOD_CAPS) ret|=SILKM_CAPS;
  return ret;
}

/*****************************************************************************

  Function to get status of mout pointer and buttons
  if rx,ry != NULL, current mousepointer will be returned
  remember, values are int and can be negative (outside window)

 *****************************************************************************/

BYTE sil_getMouse(int *rx,int *ry) {
  int x, y;
  Uint32 buttons;
  BYTE ret=0;

  SDL_PumpEvents();  

  buttons = SDL_GetMouseState(&x, &y);
  if (rx) *rx=x;
  if (ry) *ry=y;
  if (buttons & SDL_BUTTON_LMASK) ret|=SIL_BTN_LEFT;
  if (buttons & SDL_BUTTON_RMASK) ret|=SIL_BTN_RIGHT;
  if (buttons & SDL_BUTTON_MMASK) ret|=SIL_BTN_MIDDLE;
  return buttons;
}

/*****************************************************************************

  Internal function to check if mouse pointer is outside window

 *****************************************************************************/

BYTE sil_outsideWindow() {
  int x,y;
  sil_getMouse(&x,&y);
  if (x<0) return 1;
  if (y<0) return 1;
  if (x>=gv.width) return 1;
  if (y>=gv.height) return 1;
  return 0;
}
/*****************************************************************************

  Internal function to map SDL key codes to SIL keycodes
  Left and Right keycodes are mapped to same SIL keycode

 *****************************************************************************/

static UINT keycode2sil(UINT code) {

  switch(code) {
    case SDLK_BACKSPACE:  return SILKY_BACK;
    case SDLK_TAB:        return SILKY_TAB;
    case SDLK_RETURN:     return SILKY_ENTER;
    case SDLK_RSHIFT:     return SILKY_SHIFT;
    case SDLK_LSHIFT:     return SILKY_SHIFT;
    case SDLK_LCTRL:      return SILKY_CTRL;
    case SDLK_RCTRL:      return SILKY_CTRL;
    case SDLK_MENU:       return SILKY_ALT;
    case SDLK_PAUSE:      return SILKY_PAUSE;
    case SDLK_CAPSLOCK:   return SILKY_CAPS;
    case SDLK_ESCAPE:     return SILKY_ESC;
    case SDLK_SPACE:      return SILKY_SPACE;
    case SDLK_PAGEUP:     return SILKY_PAGEUP;
    case SDLK_PAGEDOWN:   return SILKY_PAGEDOWN;
    case SDLK_END:        return SILKY_END;
    case SDLK_HOME:       return SILKY_HOME;
    case SDLK_LEFT:       return SILKY_LEFT;
    case SDLK_UP:         return SILKY_UP;
    case SDLK_RIGHT:      return SILKY_RIGHT;
    case SDLK_DOWN:       return SILKY_DOWN;
    case SDLK_PRINTSCREEN:return SILKY_PRINTSCREEN;
    case SDLK_INSERT:     return SILKY_INSERT;
    case SDLK_DELETE:     return SILKY_DELETE;
    case SDLK_0:          return SILKY_0;
    case SDLK_1:          return SILKY_1;
    case SDLK_2:          return SILKY_2;
    case SDLK_3:          return SILKY_3;
    case SDLK_4:          return SILKY_4;
    case SDLK_5:          return SILKY_5;
    case SDLK_6:          return SILKY_6;
    case SDLK_7:          return SILKY_7;
    case SDLK_8:          return SILKY_8;
    case SDLK_9:          return SILKY_9;
    case SDLK_a:          return SILKY_A;
    case SDLK_b:          return SILKY_B;
    case SDLK_c:          return SILKY_C;
    case SDLK_d:          return SILKY_D;
    case SDLK_e:          return SILKY_E;
    case SDLK_f:          return SILKY_F;
    case SDLK_g:          return SILKY_G;
    case SDLK_h:          return SILKY_H;
    case SDLK_i:          return SILKY_I;
    case SDLK_j:          return SILKY_J;
    case SDLK_k:          return SILKY_K;
    case SDLK_l:          return SILKY_L;
    case SDLK_m:          return SILKY_M;
    case SDLK_n:          return SILKY_N;
    case SDLK_o:          return SILKY_O;
    case SDLK_p:          return SILKY_P;
    case SDLK_q:          return SILKY_Q;
    case SDLK_r:          return SILKY_R;
    case SDLK_s:          return SILKY_S;
    case SDLK_t:          return SILKY_T;
    case SDLK_u:          return SILKY_U;
    case SDLK_v:          return SILKY_V;
    case SDLK_w:          return SILKY_W;
    case SDLK_x:          return SILKY_X;
    case SDLK_y:          return SILKY_Y;
    case SDLK_z:          return SILKY_Z;
    case SDLK_KP_0:         return SILKY_NUM0;
    case SDLK_KP_1:         return SILKY_NUM1;
    case SDLK_KP_2:         return SILKY_NUM2;
    case SDLK_KP_3:         return SILKY_NUM3;
    case SDLK_KP_4:         return SILKY_NUM4;
    case SDLK_KP_5:         return SILKY_NUM5;
    case SDLK_KP_6:         return SILKY_NUM6;
    case SDLK_KP_7:         return SILKY_NUM7;
    case SDLK_KP_8:         return SILKY_NUM8;
    case SDLK_KP_9:         return SILKY_NUM9;
    case SDLK_KP_MULTIPLY:  return SILKY_NUMMULTIPLY;
    case SDLK_KP_PLUS:      return SILKY_NUMPLUS;
    case SDLK_KP_MINUS:     return SILKY_NUMMINUS;
    case SDLK_KP_PERIOD:    return SILKY_NUMPOINT;
    case SDLK_KP_DIVIDE:    return SILKY_NUMSLASH;
    case SDLK_F1:           return SILKY_F1;
    case SDLK_F2:           return SILKY_F2;
    case SDLK_F3:           return SILKY_F3;
    case SDLK_F4:           return SILKY_F4;
    case SDLK_F5:           return SILKY_F5;
    case SDLK_F6:           return SILKY_F6;
    case SDLK_F7:           return SILKY_F7;
    case SDLK_F8:           return SILKY_F8;
    case SDLK_F9:           return SILKY_F9;
    case SDLK_F10:          return SILKY_F10;
    case SDLK_F11:          return SILKY_F11;
    case SDLK_F12:          return SILKY_F12;
    case SDLK_NUMLOCKCLEAR: return SILKY_NUMLOCK;
    case SDLK_SCROLLLOCK:   return SILKY_SCROLLLOCK;
    case SDLK_SEMICOLON:    return SILKY_SEMICOLON;
    case SDLK_PLUS:         return SILKY_PLUS;
    case SDLK_COMMA:        return SILKY_COMMA;
    case SDLK_MINUS:        return SILKY_MINUS;
    case SDLK_PERIOD:       return SILKY_PERIOD;
    case SDLK_SLASH:        return SILKY_SLASH;
    case SDLK_BACKQUOTE:    return SILKY_ACUTE;
    case SDLK_LEFTBRACKET:  return SILKY_OPENBRACKET;
    case SDLK_BACKSLASH:    return SILKY_BACKSLASH;
    case SDLK_RIGHTBRACKET: return SILKY_CLOSEBRACKET;
    case SDLK_QUOTE:        return SILKY_APOSTROPHE;
  }
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
   * code      => 'native' keycode (in this case xwindows scancode)
   * key       => translated 'common' SIL keycode
   * modifiers => byte with bits reflecting status of shift,alt,ctrl and caps

   Note that keycodes are -trying to- refferring to the pressed key,
   *not the ascii or unicode code that is printed on it *. To figure out if 'a'
   or 'A' is typed , check for keycode 'a' + modifiers "Shift" or just get the
   'val', where OS is trying to translate keycode to ascii.
   (Actually, unicode, but I'm lazy and only want single-byte codes ... )

 *****************************************************************************/


SILEVENT *sil_getEventDisplay() {
  BYTE back=0;
  struct timeval tv;

  gv.se.type=SILDISP_NOTHING;
  gv.se.val=0;
  gv.se.x=0;
  gv.se.y=0;
  gv.se.dx=0;
  gv.se.dy=0;

  while(!back) {
    while(SDL_PollEvent(&(gv.event))) {
      switch(gv.event.type) {

        case SDL_USEREVENT:
          /* timer went off */
          gv.se.type=SILDISP_TIMER;
          gv.se.code=gv.event.user.code;
          gettimeofday(&tv,NULL);
          gv.se.val=(tv.tv_sec-gv.lasttimer.tv_sec)*1000+(tv.tv_usec-gv.lasttimer.tv_usec)/1000;
          gettimeofday(&gv.lasttimer,NULL);
          back=1;
          break;

        case SDL_QUIT:
          gv.se.type=SILDISP_QUIT;
          back=1;
          break;

        case SDL_KEYDOWN:
          gv.se.type=SILDISP_KEY_DOWN;
          gv.se.val=0;
          gv.se.code=gv.event.key.keysym.scancode;
          gv.se.modifiers=sil_getModifiers();
          gv.se.key=keycode2sil(gv.event.key.keysym.sym);
          if ((!(gv.event.key.keysym.sym & 0x40000000))&&(gv.event.key.keysym.sym>31)) {
            gv.txt=1;
            back=0;
          } else {
            gv.txt=0;
            back=1;
          }
          break;

        case SDL_KEYUP:
          gv.se.type=SILDISP_KEY_UP;
          gv.se.val=gv.txt;
          gv.se.code=gv.event.key.keysym.scancode;
          gv.se.modifiers=sil_getModifiers();
          gv.se.key=keycode2sil(gv.event.key.keysym.sym);
          gv.txt=0;
          back=1;
          break;

        case SDL_TEXTINPUT:
          if (gv.txt) {
            gv.txt=gv.event.text.text[0];
            if (gv.txt>1) {
              gv.se.val=gv.txt;
            } 
            gv.txt=0;
            back=1;
          } 
          break;

        case SDL_MOUSEMOTION:
          gv.se.type=SILDISP_MOUSE_MOVE;
          gv.se.x=gv.event.motion.x;
          gv.se.y=gv.event.motion.y;
          gv.se.dx=gv.event.motion.xrel;
          gv.se.dy=gv.event.motion.yrel;
          back=1;
          break;

        case SDL_MOUSEWHEEL:
          gv.se.type=SILDISP_MOUSEWHEEL;
          /* whoops, SDL don't give mousepointer location at this event */
          /* so, retrieve it to include them                            */
          sil_getMouse(&(gv.se.x),&(gv.se.y));
          if ((gv.event.wheel.y)>0) {
            gv.se.val=1;
          } else {
            gv.se.val=2;
          }
          back=1;
          break;

        case SDL_MOUSEBUTTONDOWN:
          if ((gv.event.button.x>-1)&&(gv.event.button.y>-1)) {
            gv.se.type=SILDISP_MOUSE_DOWN;
            gv.se.x=gv.event.button.x;
            gv.se.y=gv.event.button.y;
            gv.se.val=0;
            if (gv.event.button.button==SDL_BUTTON_LEFT) {
              gv.se.val|=SIL_BTN_LEFT;
            } else {
              if (gv.event.button.button==SDL_BUTTON_MIDDLE) {
                gv.se.val|=SIL_BTN_MIDDLE;
              } else {
                if (gv.event.button.button==SDL_BUTTON_RIGHT) gv.se.val|=SIL_BTN_RIGHT;
              }
            }
            if (gv.se.val) back=1;
          }
          break;


        case SDL_MOUSEBUTTONUP:
          gv.se.type=SILDISP_MOUSE_UP;
          gv.se.x=gv.event.button.x;
          gv.se.y=gv.event.button.y;
          gv.se.val=0;
          if (gv.event.button.button==SDL_BUTTON_LEFT) {
            gv.se.val|=SIL_BTN_LEFT;
          } else {
            if (gv.event.button.button==SDL_BUTTON_MIDDLE) {
              gv.se.val|=SIL_BTN_MIDDLE;
            } else {
              if (gv.event.button.button==SDL_BUTTON_RIGHT) gv.se.val|=SIL_BTN_RIGHT;
            }
          }
          if (gv.se.val) back=1;
          break;

        case SDL_WINDOWEVENT:
          switch(gv.event.window.event) {
            case SDL_WINDOWEVENT_SIZE_CHANGED:
            case SDL_WINDOWEVENT_RESIZED:
              if (gv.syshandler) {
                gv.se.type=SILDISP_RESIZED;
                gv.se.x=gv.event.window.data1;
                gv.se.y=gv.event.window.data2;
                gv.syshandler(&(gv.se));
              }
              gv.width=gv.event.window.data1;
              gv.height=gv.event.window.data2;
              sil_updateDisplay();
            break;
          }
          back=1;
          break;

      } // switch event
      if (1==back) break;
    } // while  event
  } // while !back
  return &(gv.se);
}

static Uint32 timercallback(Uint32 interval, void *param) {
    SDL_Event event;
    SDL_UserEvent userevent;

    /* throw it on message queue to get it picked up by getEvent */
    userevent.type = SDL_USEREVENT;
    userevent.code = 666;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return(interval);
}

void sil_setTimerDisplay(UINT amount) {
  gettimeofday(&gv.lasttimer,NULL);
  gv.timerid=SDL_AddTimer(amount, &timercallback,NULL);
}

void sil_stopTimerDisplay() {
  SDL_RemoveTimer(gv.timerid);
}

void sil_setCursor(BYTE type) {
  /* only load if cursor has been changed */
  if ((type!=gv.ctype)||(type!=SILCUR_ARROW)) {
    gv.cursor=NULL;
    switch(type) {
      case SILCUR_ARROW:
        gv.cursor=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
        break;
      case SILCUR_HAND:
        gv.cursor=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
        break;
      case SILCUR_HELP:
        /* can't find one for SDL, not implemented */
        break;
      case SILCUR_NO:
        gv.cursor=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
        break;
      case SILCUR_IBEAM:
        gv.cursor=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
        break;
      case SILCUR_VERSIZE:
        gv.cursor=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
        break;
      case SILCUR_HORSIZE:
        gv.cursor=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
        break;
      case SILCUR_DIAGRSIZE:
        gv.cursor=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
        break;
      case SILCUR_DIAGLSIZE:
        gv.cursor=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
        break;
      case SILCUR_SIZEALL:
        gv.cursor=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
        break;
    }
    if (gv.cursor) {
      gv.ctype=type;
      SDL_SetCursor(gv.cursor);
    }
  }
}

void sil_setSysHandler(void (*syshandler)(SILEVENT *)) {
  gv.syshandler=syshandler;
}

/*****************************************************************************

  Destroy display information (called by destroy SIL, to cleanup everything )

 *****************************************************************************/

void sil_destroyDisplay() {
  SILLYR *layer=sil_getBottom();
  while(layer) {
    if (layer->texture) SDL_DestroyTexture(layer->texture);
    layer=layer->next;
  }
  SDL_DestroyWindow(gv.window);
  SDL_Quit();
}
