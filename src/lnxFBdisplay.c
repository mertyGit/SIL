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
} GDISP;

static GDISP gdisp;

/*****************************************************************************

  retrieve color type from Display (SILTYPE... , see framebuffer.c for info)
  Used as "default" when no type is given when creating framebuffer
  Usually, it is the same type as the primary framebuffer the displayfuntions
  are using

 *****************************************************************************/

UINT sil_getTypefromDisplay() {
  if (NULL==gdisp.fb) {
    log_warn("trying to get display color type from non-initialized display");
    return 0;
  }
  return gdisp.fb->type;
}


/*****************************************************************************
  
  Initialize Display (called by initSIL) only for GDI, we need to have the "hIntance" 
  of WinMain call (SDL can use its own, using flag DSL_MAIN_HANDLED)
  other options are width,height and title of window

 *****************************************************************************/
UINT sil_initDisplay(void *hI, UINT width, UINT height, char *title) {
  unsigned char *image;
  unsigned error;
  BYTE type=0;
  FILE *fp;
  int fd=0;
  char name[256]="unknown";



  gdisp.fbfd=open("/dev/fb0",O_RDWR);
  if (-1==gdisp.fbfd) {
    log_info("ERR: Can't open framebuffer device");
    return SILERR_NOTINIT;
  }

  if (-1==ioctl(gdisp.fbfd, FBIOGET_FSCREENINFO, &gdisp.finfo)) {
    log_info("ERR: Can't get framebuffer device fixed info");
    return SILERR_NOTINIT;
  }
  if (-1==ioctl(gdisp.fbfd, FBIOGET_VSCREENINFO, &gdisp.vinfo)) {
    log_warn("ERR: Can't get framebuffer device variable info");
    return SILERR_NOTINIT;
  }
  gdisp.screensize = gdisp.finfo.smem_len;

  gdisp.fbp=(BYTE *)mmap(NULL,gdisp.screensize,PROT_READ|PROT_WRITE,MAP_SHARED,gdisp.fbfd,0);
  if (NULL==gdisp.fbp) {
    log_warn("ERR: Can't mmap framebuffer device ");
    return SILERR_NOTINIT;
  }

  switch (gdisp.vinfo.green.length) {
    case 3: if ((2==gdisp.vinfo.blue.length)&&(3==gdisp.vinfo.red.length))
              type=SILTYPE_332RGB;
            if ((3==gdisp.vinfo.blue.length)&&(2==gdisp.vinfo.red.length))
              type=SILTYPE_332BGR;
            break;
    case 4: if ((4==gdisp.vinfo.blue.length)&&(4==gdisp.vinfo.red.length)) {
              if (gdisp.vinfo.red.offset>gdisp.vinfo.blue.offset) {
                type=SILTYPE_444RGB;
              } else {
                type=SILTYPE_444BGR;
              }
            }
            break;
    case 5: if ((5==gdisp.vinfo.blue.length)&&(5==gdisp.vinfo.red.length)) {
              if (gdisp.vinfo.red.offset>gdisp.vinfo.blue.offset) {
                type=SILTYPE_555RGB;
              } else {
                type=SILTYPE_555BGR;
              }
            } 
    case 6: if ((5==gdisp.vinfo.blue.length)&&(5==gdisp.vinfo.red.length)) {
              if (gdisp.vinfo.red.offset>gdisp.vinfo.blue.offset) {
                type=SILTYPE_565BGR;
              } else {
                type=SILTYPE_565RGB;
              }
            }
            break;
    case 8: if ((8==gdisp.vinfo.blue.length)&&(8==gdisp.vinfo.red.length)) {
              if (gdisp.vinfo.transp.length>0) {
                if (gdisp.vinfo.red.offset>gdisp.vinfo.blue.offset) {
                  type=SILTYPE_ARGB;
                } else {
                  type=SILTYPE_ABGR;
                }
              } else {
                if (gdisp.vinfo.red.offset>gdisp.vinfo.blue.offset) {
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

  log_info("Screeninfo: %dx%d, %dbpp , size=%d",gdisp.vinfo.xres,gdisp.vinfo.yres, 
    gdisp.vinfo.bits_per_pixel,gdisp.screensize);
  log_info("Screentype: %d",type);

  /* this framebuffer will be dedicated for the window */
  gdisp.fb=sil_initFB(gdisp.vinfo.xres, gdisp.vinfo.yres, type);
  if (NULL==gdisp.fb) {
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
  gdisp.fevent=open("/dev/input/event2",O_RDONLY);
  if (!gdisp.fevent) {
    log_warn("Cant open event2 to read touchscreen");
  } else {
    /* should be something like "rasberrypi-ts" or something, denoting a */
    /* touchscreen and not a mouse or keyboard..                         */
    ioctl(gdisp.fevent, EVIOCGNAME(sizeof(name)),name);
    log_info("Input device name is : %s",name);
  }

  /* init some global variables for later */
  gdisp.lastx=0;
  gdisp.lasty=0;

  return SILERR_ALLOK;
}

/*****************************************************************************
  
  Update Display

 *****************************************************************************/
void sil_updateDisplay() {

  /* get all layerinformation into a single fb */
  LayersToFB(gdisp.fb);

  /* and just copy it */
  memcpy(gdisp.fbp,gdisp.fb->buf,gdisp.screensize);

}

/*****************************************************************************
  
  Destroy display information (called by destroy SIL, to cleanup everything )

 *****************************************************************************/
void sil_destroyDisplay() { 
  int fd;

  if (gdisp.fb->type) sil_destroyFB(gdisp.fb);
  if (gdisp.fevent) close(gdisp.fevent);
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
    gdisp.se.type=SILDISP_NOTHING;
    gdisp.se.val=0;
    gdisp.se.code=0;
    gdisp.se.key=0;
    gdisp.se.modifiers=0;
    gdisp.se.layer=NULL;
    gdisp.se.x=gdisp.lastx;
    gdisp.se.y=gdisp.lasty;

    FD_ZERO(&rs);
    FD_SET(gdisp.fevent,&rs);

    if ((gdisp.tval.tv_sec>0)||(gdisp.tval.tv_usec>0)) {
      tt.tv_sec=gdisp.tval.tv_sec;
      tt.tv_usec=gdisp.tval.tv_usec;
      tp=&tt;
    } else {
      tp=NULL;
    }

    if (0==select(gdisp.fevent+1,&rs,NULL,NULL,tp)) {
      /* timer expired */
      gdisp.se.type=SILDISP_TIMER;
      gdisp.se.code=666;
      gettimeofday(&tv,NULL);
      gdisp.se.val=(tv.tv_sec-gdisp.lasttimer.tv_sec)*1000+(tv.tv_usec-gdisp.lasttimer.tv_usec)/1000;
      gettimeofday(&gdisp.lasttimer,NULL);
      stop=1;
    } else {
      /* we have event(s) read till SYN_REPORT */
      do {
        rd=read(gdisp.fevent,&ev,sizeof(ev));
        
        /* not garbage ? */
        if (rd<(int)sizeof(struct input_event)) {
          log_warn("Wrong size of event returned from touchscreen");
          gdisp.se.type=SILDISP_NOTHING;
          return &gdisp.se;
        }
        //log_info("Got event: type:%d code:%d value:%d",ev.type,ev.code,ev.value);


        if ((EV_KEY==ev.type)&&(BTN_TOUCH==ev.code)) {
          if (0==ev.value) {
            gdisp.se.type=SILDISP_MOUSE_UP;
            gdisp.se.val=1;
          } else {
            gdisp.se.type=SILDISP_MOUSE_DOWN;
            gdisp.se.val=1;
          }
          /* wait for other events for more information */
        }

        if (EV_ABS==ev.type) {
          if (ABS_X==ev.code) {
            gdisp.se.x=ev.value;
            gdisp.lastx=ev.value;
          } else {
            if (ABS_Y==ev.code) {
              gdisp.se.y=ev.value;
              gdisp.lasty=ev.value;
            }
          }
          if (SILDISP_NOTHING==gdisp.se.type) gdisp.se.type=SILDISP_MOUSE_MOVE;
        }

      } while (!((EV_SYN==ev.type)&&(SYN_REPORT==ev.code))); 
      if (SILDISP_NOTHING!=gdisp.se.type) stop=1;
    }
  } while (!stop);
  return &gdisp.se;
}


void sil_setTimerDisplay(UINT amount) {
  gettimeofday(&gdisp.lasttimer,NULL);
  gdisp.tval.tv_sec=amount/1000;
  gdisp.tval.tv_usec=(amount-gdisp.tval.tv_sec*1000)*1000;
}

void sil_stopTimerDisplay() {
  gdisp.tval.tv_sec=0;
  gdisp.tval.tv_usec=0;
}


void sil_setCursor(BYTE type) {
  /* not used */
}
