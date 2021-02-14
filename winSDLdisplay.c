/*

   winGDIdisplay.c CopyRight 2021 Remco Schellekens, see LICENSE for more details.

   This file contains all functions for displaying the layers on a Windows environment, using standard
   (non-hardware accelerated) GDI API.
   It will do so by using a framebuffer as source for a bitmap inside a fixed size window. Every time
   framebuffer is changed, bitmap is replaced.
   This isn't the fastest method, however, it will come close to the usual "software framebuffer" use
   on non-hardware accelerated platforms. If you want speed, use additional SDL one (winSDLdisplay.c)

   every "...display.c" file should have 5 functions
   -sil_initDisplay        ; create initial display, called via initializing SIL
   -sil_updateDisplay      ; update display, will check all layers updates display accordingly
   -sil_destroyDisplay     ; remove display, called via destroying SIL
   -sil_getEventDisplay    ; Wait or get first event ( mouse / keys or closing window )
   -sil_getTypefromDisplay ; will return the "native" color type of the display

*/

#include <SDL2/SDL.h> 
#include <stdio.h>
#include "sil.h"
#include "log.h"

static SDL_Window *window=NULL;
static SDL_Surface *surface;
static SDL_Event event;
static SDL_Renderer *renderer=NULL;
static SILFB *scratch=NULL;
static SILEVENT se;
static UINT txt;

/*****************************************************************************

  retrieve color type from Display (SILTYPE... , see framebuffer.c for info)
  Used as "default" when no type is given when creating framebuffer

 *****************************************************************************/

UINT sil_getTypefromDisplay() {
  return SILTYPE_ARGB;
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
  BYTE red2,green2,blue2,alpha2;

  SILLYR *layer=sil_getBottomLayer();
  SDL_RenderClear(renderer);
  /* loop from bottom to top layer */
  while (layer) {
    if (NULL==layer->texture) {
      /* no texture yet for this layer */
      layer->texture=SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,layer->fb->width,layer->fb->height);
      if (NULL==layer->texture) {
        printf("Warning: can't create texture for layer: %s\n", SDL_GetError());
        continue;
      }
      SDL_SetTextureBlendMode(layer->texture,SDL_BLENDMODE_BLEND);
    }
    /* normally, we would alter every alpha value when copying pixels to destination framebuffer    */
    /* however, we don't do framebuffer handling directly, so we have to override it some other way */
    if (layer->flags&SILFLAG_ALPHACHANGED) {
      SDL_SetTextureAlphaMod(layer->texture,(BYTE) (layer->alpha*255));
      layer->flags^=SILFLAG_ALPHACHANGED;
    }
    if (layer->flags&SILFLAG_INVISIBLE) continue;
    if (layer->fb->changed) {
      if (layer->fb->type==SILTYPE_ARGB) {
        SDL_UpdateTexture(layer->texture,NULL,layer->fb->buf,(layer->fb->width)*4);
      } else {
        /* not ARGB , convert it to ARGB                                         */
        /* use scratch buffer, but to do so, alter its width & height temporarly */
        scratchw=scratch->width;
        scratchh=scratch->height;
        if (scratch->width>layer->fb->width) scratch->width=layer->fb->width;
        if (scratch->height>layer->fb->height) scratch->height=layer->fb->height;
        for (UINT x=0;x<scratch->width;x++) {
          for (UINT y=0;y<scratch->height;y++) {
              sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha);            
              sil_putPixelFB(scratch,x,y,red,green,blue,alpha);
          }
        }
        SDL_UpdateTexture(layer->texture,NULL,scratch->buf,scratch->width*4);
        scratch->width=scratchw;
        scratch->height=scratchh;
      }
      layer->fb->changed=0;
    }
    SR.x=layer->view.minx;
    SR.y=layer->view.miny;
    SR.w=layer->view.maxx-SR.x;
    SR.h=layer->view.maxy-SR.y;
    DR.x=layer->relx;
    DR.y=layer->rely;
    DR.w=SR.w;
    DR.h=SR.h;
    SDL_RenderCopy(renderer,layer->texture,&SR,&DR);
    layer=layer->next;
  }
}

/*****************************************************************************

  Initialize Display (called by initSIL) 
  first argument is ignored (only needed for GDI one)
  rest is width,height and title of generated window

 *****************************************************************************/


UINT sil_initDisplay(void *nop, UINT width, UINT height, char *title) {
  UINT err=0;

  /* Create window */
  window=SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,width,height,0);
  if (NULL==window) {
    printf("ERROR: can't create window for display: %s\n",SDL_GetError());
    sil_setErr(SILERR_NOTINIT);
    return SILERR_NOTINIT;
  }

  /* create "scratch" FB, used for conversion to/from non BGRA layers */
  scratch=sil_initFB(width,height,SILTYPE_ARGB);
  if (NULL==scratch) {
    log_info("ERR: Can't create scratch framebuffer for display");
    sil_setErr(SILERR_NOTINIT);
    return SILERR_NOTINIT;
  }

  /* Raise created window */
  SDL_RaiseWindow(window);

  /* Create hardware renderer, used for placing textures on it */
  renderer=SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED  );
  if (NULL==renderer) {
      printf("Could not create renderer: %s\n", SDL_GetError());
      sil_setErr(SILERR_NOTINIT);
      return SILERR_NOTINIT;
  }

  /* We want alpha working in layers */
  SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
  return SILERR_ALLOK;
}

/*****************************************************************************

  Update Display

 *****************************************************************************/
void sil_updateDisplay() {
  LayersToDisplay();
  SDL_RenderPresent(renderer);
}

/*****************************************************************************

  Internal function to get status of special keys (shift,menu,control,caps)
  returns byte with bits set for given keys. Note: it doesn't distinguish
  between keys on the right or left side of keyboard

 *****************************************************************************/
static BYTE modifiers2sil() {
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


SILEVENT *sil_getEventDisplay(BYTE wait) {
  BYTE back=0;

  se.type=SILDISP_NOTHING;
  se.val=0;
  se.x=0;
  se.y=0;

  while(!back) {
    if (0==wait) 
      if (!(SDL_PollEvent)) return &se;
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_QUIT:
          se.type=SILDISP_QUIT;
          back=1;
          break;

        case SDL_KEYDOWN:
          se.type=SILDISP_KEY_DOWN;
          se.val=0;
          se.code=event.key.keysym.scancode;
          se.modifiers=modifiers2sil();
          se.key=keycode2sil(event.key.keysym.sym);
          if (!(event.key.keysym.sym & 0x40000000)) {
            txt=1;
            back=0;
          } else {
            txt=0;
            back=1;
          }
          break;

        case SDL_KEYUP:
          se.type=SILDISP_KEY_UP;
          se.val=txt;
          se.code=event.key.keysym.scancode;
          se.modifiers=modifiers2sil();
          se.key=keycode2sil(event.key.keysym.sym);
          txt=0;
          back=1;
          break;

        case SDL_TEXTINPUT:
          if (txt) {
            txt=event.text.text[0];
            if (txt>1) {
              se.val=txt;
            } else {
            }
            back=1;
          } 
          break;

        case SDL_MOUSEMOTION:
          se.type=SILDISP_MOUSE_MOVE;
          se.x=event.motion.x;
          se.y=event.motion.y;
          back=1;
          break;

        case SDL_MOUSEWHEEL:
          se.type=SILDISP_MOUSEWHEEL;
          if ((event.wheel.y)>0) {
            se.val=1;
          } else {
            se.val=2;
          }
          back=1;
          break;

        case SDL_MOUSEBUTTONDOWN:
          se.type=SILDISP_MOUSE_DOWN;
          se.x=event.button.x;
          se.y=event.button.y;
          se.val=0;
          if (event.button.button==SDL_BUTTON_LEFT) {
            se.val=1;
          } else {
            if (event.button.button==SDL_BUTTON_MIDDLE) {
              se.val=2;
            } else {
              if (event.button.button==SDL_BUTTON_RIGHT) se.val=3;
            }
          }
          if (se.val) back=1;
          break;


        case SDL_MOUSEBUTTONUP:
          se.type=SILDISP_MOUSE_UP;
          se.x=event.button.x;
          se.y=event.button.y;
          se.val=0;
          if (event.button.button==SDL_BUTTON_LEFT) {
            se.val=1;
          } else {
            if (event.button.button==SDL_BUTTON_MIDDLE) {
              se.val=2;
            } else {
              if (event.button.button==SDL_BUTTON_RIGHT) se.val=3;
            }
          }
          if (se.val) back=1;
          break;
      }
    }
  }
  return &se;
}

/*****************************************************************************

  Destroy display information (called by destroy SIL, to cleanup everything )

 *****************************************************************************/

void sil_destroyDisplay() {
  SILLYR *layer=sil_getBottomLayer();
  while(layer) {
    if (layer->texture) SDL_DestroyTexture(layer->texture);
    layer=layer->next;
  }
  SDL_DestroyWindow(window);
  SDL_Quit();
}
