/*

   fbdisplay.c CopyRight 2021 Remco Schellekens, see LICENSE for more details.

   This file contains all functions for displaying the layers on a Windows environment, using standard
   (non-hardware accelerated) GDI API.
   It will do so by using a framebuffer as source for a bitmap inside a fixed size window. Every time 
   framebuffer is changed, bitmap is replaced.
   This isn't the fastest method, however, it will come close to the usual "software framebuffer" use
   on non-hardware accelerated platforms. If you want speed, use additional SDL one (winSDLdisplay.c)

   every "...display.c" file should have these functions
   -sil_initDisplay        ; create initial display, called via initializing SIL
   -sil_updateDisplay      ; update display, will check all layers updates display accordingly
   -sil_destroyDisplay     ; remove display, called via destroying SIL
   -sil_getEventDisplay    ; Wait or get first event ( mouse / keys or closing window )
   -sil_getTypefromDisplay ; will return the "native" color type of the display
   -sil_setTimerDisplay    ; will set a repeating timer to interrupt the wait loop 
   -sil_stopTimerDisplay   ; stops the repeating timer
   -sil_setCursor          ; sets the mouse cursor (in windowed environments) 
   -sil_getMouse           ; retrieves last known position of mouse cursor and button


*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/kd.h>
#include "sil.h"
#include "log.h"



typedef struct _GDISP {
  SILFB *fb;
  SILEVENT se;
  struct timeval lasttimer;
  struct timeval tval;
  int fbfd;
  BYTE *fbp;
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;
  long int screensize;
  int fevent;
  UINT lastx;
  UINT lasty;
  BYTE button;
} GDISP;

static GDISP gv;

BYTE sil_getMouse(int *x,int *y) {
  if (x) *x=gv.lastx;
  if (y) *y=gv.lasty;
  return gv.button;
}

/*****************************************************************************

  retrieve color type from Display (SILTYPE... , see framebuffer.c for info)
  Used as "default" when no type is given when creating framebuffer
  Usually, it is the same type as the primary framebuffer the displayfuntions
  are using

 *****************************************************************************/

UINT sil_getTypefromDisplay() {
  if (NULL==gv.fb) {
    log_warn("trying to get display color type from non-initialized display");
    return 0;
  }
  return gv.fb->type;
}


/*****************************************************************************
  
  Initialize Display (called by initSIL) only for GDI, we need to have the "hIntance" 
  of WinMain call (SDL can use its own, using flag DSL_MAIN_HANDLED)
  other options are width,height and title of window

 *****************************************************************************/
UINT sil_initDisplay(void *hI, UINT width, UINT height, char *title) {
  BYTE type=0;
  int fd=0;
  char name[256]="unknown";



  gv.fbfd=open("/dev/fb0",O_RDWR);
  if (-1==gv.fbfd) {
    log_info("ERR: Can't open framebuffer device");
    return SILERR_NOTINIT;
  }

  if (-1==ioctl(gv.fbfd, FBIOGET_FSCREENINFO, &gv.finfo)) {
    log_info("ERR: Can't get framebuffer device fixed info");
    return SILERR_NOTINIT;
  }
  if (-1==ioctl(gv.fbfd, FBIOGET_VSCREENINFO, &gv.vinfo)) {
    log_warn("ERR: Can't get framebuffer device variable info");
    return SILERR_NOTINIT;
  }
  gv.screensize = gv.finfo.smem_len;

  gv.fbp=(BYTE *)mmap(NULL,gv.screensize,PROT_READ|PROT_WRITE,MAP_SHARED,gv.fbfd,0);
  if (NULL==gv.fbp) {
    log_warn("ERR: Can't mmap framebuffer device ");
    return SILERR_NOTINIT;
  }

  switch (gv.vinfo.green.length) {
    case 3: 
      if ((2==gv.vinfo.blue.length)&&(3==gv.vinfo.red.length)) type=SILTYPE_332RGB;
      if ((3==gv.vinfo.blue.length)&&(2==gv.vinfo.red.length)) type=SILTYPE_332BGR;
      break;
    case 4: 
      if ((4==gv.vinfo.blue.length)&&(4==gv.vinfo.red.length)) {
        if (gv.vinfo.red.offset>gv.vinfo.blue.offset) {
          type=SILTYPE_444RGB;
        } else {
          type=SILTYPE_444BGR;
        }
      }
      break;
    case 5: 
      if ((5==gv.vinfo.blue.length)&&(5==gv.vinfo.red.length)) {
        if (gv.vinfo.red.offset>gv.vinfo.blue.offset) {
          type=SILTYPE_555RGB;
        } else {
          type=SILTYPE_555BGR;
        }
      } 
      break;
    case 6: 
      if ((5==gv.vinfo.blue.length)&&(5==gv.vinfo.red.length)) {
        if (gv.vinfo.red.offset>gv.vinfo.blue.offset) {
          type=SILTYPE_565BGR;
        } else {
          type=SILTYPE_565RGB;
        }
      }
      break;
    case 8: 
      if ((8==gv.vinfo.blue.length)&&(8==gv.vinfo.red.length)) {
        if (gv.vinfo.transp.length>0) {
          if (gv.vinfo.red.offset>gv.vinfo.blue.offset) {
            type=SILTYPE_ARGB;
          } else {
            type=SILTYPE_ABGR;
          }
        } else {
          if (gv.vinfo.red.offset>gv.vinfo.blue.offset) {
            type=SILTYPE_888RGB;
          } else {
            type=SILTYPE_888BGR;
          }
        }
      }
      break;
  }

  if (0==type) {
    log_warn("Can't find the right framebuffer type");
    return SILERR_NOTINIT;
  }

  log_info("Screeninfo: %dx%d, %dbpp , size=%d",gv.vinfo.xres,gv.vinfo.yres, 
    gv.vinfo.bits_per_pixel,gv.screensize);
  log_info("Screentype: %d",type);

  /* this framebuffer will be dedicated for the window */
  gv.fb=sil_initFB(gv.vinfo.xres, gv.vinfo.yres, type);
  if (NULL==gv.fb) {
    log_info("ERR: Can't create framebuffer for display");
    return SILERR_NOMEM;
  }

  /* stop cursor */
  fd =open("/dev/tty0",O_RDWR);
  if (!fd) {
    log_warn("Cant open tty0 to turn of cursor");
  }
  if(ioctl(fd,KDSETMODE,KD_GRAPHICS)) {
    log_warn("Can't set mode to graphics");
  }
  close(fd);

  /* open events for mouse/touchscreen */
  /* not sure /dev/input/event2 is location of touch events in every situation..*/
  /* it looks like it is for raspberry pi's..                                   */
  gv.fevent=open("/dev/input/event2",O_RDONLY);
  if (!gv.fevent) {
    log_warn("Cant open event2 to read touchscreen");
  } else {
    /* should be something like "rasberrypi-ts" or something, denoting a */
    /* touchscreen and not a mouse or keyboard..                         */
    ioctl(gv.fevent, EVIOCGNAME(sizeof(name)),name);
    log_info("Input device name is : %s",name);
  }

  /* init some global variables for later */
  gv.lastx=0;
  gv.lasty=0;
  gv.button=0;

  return SILERR_ALLOK;
}

/*****************************************************************************
  
  Update Display

 *****************************************************************************/
void sil_updateDisplay() {

  /* get all layerinformation into a single fb */
  LayersToFB(gv.fb);

  /* and just copy it */
  memcpy(gv.fbp,gv.fb->buf,gv.screensize);

}

/*****************************************************************************
  
  Destroy display information (called by destroy SIL, to cleanup everything )

 *****************************************************************************/
void sil_destroyDisplay() { 
  int fd;

  if (gv.fb->type) sil_destroyFB(gv.fb);
  if (gv.fevent) close(gv.fevent);
  fd =open("/dev/tty0",O_RDWR);
  if (fd) { 
    /* back to text mode */
    ioctl(fd,KDSETMODE,KD_TEXT);
    close(fd);
  }
}


/*****************************************************************************
  
  Get event from display
  depending on abilities to do so, setting "wait" on 1 will wait until event 
  comes (blocking) and setting to zero will only poll.
  However, on Windows platform we only use blocking wait 

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

 *****************************************************************************/
SILEVENT *sil_getEventDisplay() {
  int stop=0;
  fd_set rs;
  struct timeval *tp;
  struct timeval tt,tv;
  struct input_event ev;
  int rd=0;


  do {
    gv.se.type=SILDISP_NOTHING;
    gv.se.val=0;
    gv.se.code=0;
    gv.se.key=0;
    gv.se.modifiers=0;
    gv.se.layer=NULL;
    gv.se.x=gv.lastx;
    gv.se.y=gv.lasty;

    FD_ZERO(&rs);
    FD_SET(gv.fevent,&rs);

    if ((gv.tval.tv_sec>0)||(gv.tval.tv_usec>0)) {
      tt.tv_sec=gv.tval.tv_sec;
      tt.tv_usec=gv.tval.tv_usec;
      tp=&tt;
    } else {
      tp=NULL;
    }

    if (0==select(gv.fevent+1,&rs,NULL,NULL,tp)) {
      /* timer expired */
      gv.se.type=SILDISP_TIMER;
      gv.se.code=666;
      gettimeofday(&tv,NULL);
      gv.se.val=(tv.tv_sec-gv.lasttimer.tv_sec)*1000+(tv.tv_usec-gv.lasttimer.tv_usec)/1000;
      gettimeofday(&gv.lasttimer,NULL);
      stop=1;
    } else {
      /* we have event(s) read till SYN_REPORT */
      do {
        rd=read(gv.fevent,&ev,sizeof(ev));
        
        /* not garbage ? */
        if (rd<(int)sizeof(struct input_event)) {
          log_warn("Wrong size of event returned from touchscreen");
          gv.se.type=SILDISP_NOTHING;
          return &gv.se;
        }
        //log_info("Got event: type:%d code:%d value:%d",ev.type,ev.code,ev.value);

        /* for this we presume touchscreen, therefore one button */
        if ((EV_KEY==ev.type)&&(BTN_TOUCH==ev.code)) {
          if (0==ev.value) {
            gv.se.type=SILDISP_MOUSE_UP;
            gv.se.val=SIL_BTN_LEFT;
            gv.button=0;
          } else {
            gv.se.type=SILDISP_MOUSE_DOWN;
            gv.se.val=SIL_BTN_LEFT;
            gv.button=SIL_BTN_LEFT;
          }
          /* wait for other events for more information */
        }

        if (EV_ABS==ev.type) {
          if (ABS_X==ev.code) {
            gv.se.x=ev.value;
            gv.se.dx=ev.value-gv.lastx;
            gv.lastx=ev.value;
          } else {
            if (ABS_Y==ev.code) {
              gv.se.y=ev.value;
              gv.se.dy=ev.value-gv.lasty;
              gv.lasty=ev.value;
            }
          }
          if (SILDISP_NOTHING==gv.se.type) gv.se.type=SILDISP_MOUSE_MOVE;
        }

      } while (!((EV_SYN==ev.type)&&(SYN_REPORT==ev.code))); 
      if (SILDISP_NOTHING!=gv.se.type) stop=1;
    }
  } while (!stop);
  return &gv.se;
}


void sil_setTimerDisplay(UINT amount) {
  gettimeofday(&gv.lasttimer,NULL);
  gv.tval.tv_sec=amount/1000;
  gv.tval.tv_usec=(amount-gv.tval.tv_sec*1000)*1000;
}

void sil_stopTimerDisplay() {
  gv.tval.tv_sec=0;
  gv.tval.tv_usec=0;
}


void sil_setCursor(BYTE type) {
  /* not used */
}
