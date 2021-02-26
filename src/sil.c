/* 

   sil.c CopyRight 2021 Remco Schellekens, see LICENSE for more details.

   This file contains all generic basic functions and helpers for SIL

*/

#include <stdio.h>
#include <stdlib.h>
#include "sil.h"
#include "log.h"


typedef struct _GSIL {
 SILLYR *ActiveLayer;
 BYTE quit;
 UINT lasterr;
 UINT init;
 UINT (*timer)(SILEVENT *);
 UINT amount;
} GSIL;
static GSIL gsil;


/*****************************************************************************
   initialize SIL context for library. It will be used to contain program 
   related stuff like logging parameters together

   In: 
       width,height   ; dimensions of window to create (or screen display)
       char *title    ; Title to use for window (in windowed environments)
       void *hInstance; Only for Windows/GDI; Windows instance handle from WinMain
       char *logname  ; Filename of the logs. When "NULL" stdout will be used
       flags          ; Flag(s) (use '|' to combine ) what to send to this log ; 
                        LOG_INFO    (Errors, Warnings), "ERR:" and "WARN:"
                        LOG_VERBOSE (other non-critical information), "INF:"
                        LOG_DEBUG   (debugging info) "DBG:"
                        no flags means no logging at all.

   Out: 0=OK, other=error

 *****************************************************************************/

UINT sil_initSIL(UINT width, UINT height, char *title, void *hInstance) {
  UINT ret=SILERR_ALLOK;
  UINT err=0;

  /* initialize global variables */
  gsil.ActiveLayer=NULL;
  gsil.quit=0;


  gsil.lasterr=0;
  gsil.init=1;
  err=log_init(NULL,0); 
  if (err) {
    log_fatal("Can't initialize logging");
  }
  if (SILERR_ALLOK!=sil_initDisplay(hInstance,width,height,title)) {
    log_fatal("Can't initialize display");
  }
  return ret;
}

UINT sil_setLog(char *logname, BYTE flags) {
  UINT ret=SILERR_ALLOK;
  UINT err=0;

  err=log_init(logname,flags); /* just redo initialization */
  if (err) {
    log_fatal("Can't set logging");
  }
  log_verbose("SIL Log options set");
  return ret;
}

void sil_quitLoop() {
  gsil.quit=1;
}

void sil_mainLoop() {
  SILLYR *tmp;
  SILEVENT *se;
  UINT destx,desty;

  do {
    se=sil_getEventDisplay(0);
    if (NULL==se) break; /* should not happen */
    switch (se->type) {
      case SILDISP_TIMER:
        gsil.amount=0;
        if (gsil.timer) 
          if (gsil.timer(se)) sil_updateDisplay();
        break;
      case SILDISP_MOUSE_UP:
          if (gsil.ActiveLayer) sil_clearFlags(gsil.ActiveLayer,SILFLAG_BUTTONDOWN);
      case SILDISP_MOUSE_DOWN:
      case SILDISP_MOUSEWHEEL:
        se->layer=sil_findHighestClick(se->x,se->y);
        if (se->layer) {
          sil_setCursor(SILCUR_HAND);
          gsil.ActiveLayer=se->layer;
          se->x-=se->layer->relx;
          se->y-=se->layer->rely;
          if (SILDISP_MOUSE_UP==se->type) sil_clearFlags(gsil.ActiveLayer,SILFLAG_BUTTONDOWN);
          if (SILDISP_MOUSE_DOWN==se->type) {
            sil_setFlags(gsil.ActiveLayer,SILFLAG_BUTTONDOWN);
            gsil.ActiveLayer->prevx=se->x;
            gsil.ActiveLayer->prevy=se->y;
          }
          if (se->layer->click) {
            if (se->layer->click(se)) sil_updateDisplay();
          }
        } else {
          sil_setCursor(SILCUR_ARROW);
        }
        break;

      case SILDISP_MOUSE_MOVE:
        /* Set mouse pointer if it has click or draggable */
        if ((gsil.ActiveLayer)&&(sil_checkFlags(gsil.ActiveLayer,SILFLAG_DRAGGABLE|SILFLAG_BUTTONDOWN))) {
          sil_setCursor(SILCUR_HAND);
          /* if it is draggable and mousebutton is down, just move it */
          se->x=(int)(se->x)-(int)gsil.ActiveLayer->prevx;
          se->y=(int)(se->y)-(int)gsil.ActiveLayer->prevy;
          if (gsil.ActiveLayer->drag) {
            se->type=SILDISP_MOUSE_DRAG;
            se->layer=gsil.ActiveLayer;
            if (gsil.ActiveLayer->drag(se)) {
              gsil.ActiveLayer->relx=se->x;
              gsil.ActiveLayer->rely=se->y;
              sil_updateDisplay();
            }
          } else {
            /* no draghandler defined, just drag it */
            gsil.ActiveLayer->relx=se->x;
            gsil.ActiveLayer->rely=se->y;
            sil_updateDisplay();
          }
          /*  */
        } else {
          if (sil_findHighestClick(se->x,se->y)) { 
            sil_setCursor(SILCUR_HAND);
          } else {
            sil_setCursor(SILCUR_ARROW);
          }
          se->layer=sil_findHighestHover(se->x,se->y);
          if ((gsil.ActiveLayer)&&(gsil.ActiveLayer!=se->layer)) {
            /* even if mouse button is still pressed, if mousepointer isn't above layer */
            /* it doesn't make sense to keep this flag up for the "old" layer           */
            sil_clearFlags(gsil.ActiveLayer,SILFLAG_BUTTONDOWN);
            if (gsil.ActiveLayer->hover) {
              tmp=se->layer;
              se->type=SILDISP_MOUSE_LEFT;
              se->layer=gsil.ActiveLayer;
              /* don't send LEFT event when old activelayer became invisible */
              if (!sil_checkFlags(gsil.ActiveLayer,SILFLAG_INVISIBLE)) {
                if (gsil.ActiveLayer->hover(se)) sil_updateDisplay();
              }
              se->layer=tmp;
              gsil.ActiveLayer=NULL;
            }
          }
          if (se->layer) {
            se->x-=se->layer->relx;
            se->y-=se->layer->rely;
            if (gsil.ActiveLayer!=se->layer) {
              se->type=SILDISP_MOUSE_ENTER;
              if (se->layer->hover(se)) sil_updateDisplay();
            }
            se->type=SILDISP_MOUSE_MOVE;
            gsil.ActiveLayer=se->layer;
            if (se->layer->hover(se)) sil_updateDisplay();
          }
        }
        break;

      case SILDISP_KEY_DOWN:
        se->layer=sil_findHighestKeyPress(se->key, se->modifiers);
        if ((se->layer)&&(!((se->layer->internal) & SILKT_ONLYUP))) {
          if ((se->layer->internal) & SILKT_SINGLE) {
            if (0==(se->layer->internal&SILFLAG_KEYEVENT)) {
              se->layer->internal|=SILFLAG_KEYEVENT;
              if (se->layer->keypress(se)) {
                sil_updateDisplay();
              }
            }
          } else {
            if (se->layer->keypress(se)) sil_updateDisplay();
          }
        }
        break;
      case SILDISP_KEY_UP:
        se->layer=sil_findHighestKeyPress(se->key, se->modifiers);
        if (se->layer) {
          se->layer->internal&=~SILFLAG_KEYEVENT;
          if (!((se->layer->internal) & SILKT_SINGLE)) {
            if (se->layer->keypress(se)) sil_updateDisplay();
          }
        }
        break;
    }
  } while ((0==gsil.quit)&&(SILDISP_QUIT!=se->type));
}


/*****************************************************************************
  Set errorcode, used internally to check on previous errors 
  or to translate error code to text into log
  In: SIL context + errorcode (UINT) => one of the "SILERR_..." 

 *****************************************************************************/

void sil_setErr(UINT errorcode) {
  if (gsil.init) {
    gsil.lasterr=errorcode;
  } else {
    /* cant send it to log, because no SIL, no init of log */
    printf("WARN: SIL not initialized (yet), can't set errorcode\n");
  }
}

/*****************************************************************************
  Get errorcode
  In: SIL context 
  Out: latest errorcode

 *****************************************************************************/

UINT sil_getErr() {
  if (gsil.init) {
    return gsil.lasterr;
  } else {
    /* cant send it to log, because no SIL, no init of log */
    printf("WARN: SIL not initialized (yet), can't set errorcode\n");
  }
  return 0;
}

/*****************************************************************************
  Translate errorcode to text that can be used in error messages
  In:  errorcode
  Out: pointer to text

 *****************************************************************************/
const char * sil_err2Txt(UINT errorcode) {
  char *ret="Unknown code";
  switch(errorcode) {
    case SILERR_ALLOK:
      ret="INFO: No error, Everything is ok";
      break;
    case SILERR_CANTOPENFILE:
      ret="ERR: Can't open file";
      break;
    case SILERR_NOMEM:
      ret="ERR: Can't allocate memory";
      break;
    case SILERR_CANTDECODEPNG:
      ret="ERR: Can't decode PNG";
      break;
    case SILERR_NOFILEFOUND:
      ret="ERR: Can't find given file";
      break;
    case SILERR_NOCHARS:
      ret="ERR: Can't find character definitions in fonts file";
      break;
    case SILERR_WRONGFORMAT:
      ret="ERR: Parsing error, wrong format used";
      break;
    case SILERR_NOTINIT:
      ret="ERR: Parameters or context not initialized (yet)";
      break;
    default:
      log_warn("unknown errorcode (%d) used",errorcode);
      break;
  }
  return ret;
}

/*****************************************************************************
  Functions to set timer handler and to set timer value (set to zero when timer
  expires !)

 *****************************************************************************/


void sil_setTimerHandler(UINT (*timer)(SILEVENT *)) {
  if (!(gsil.init)) return;
  if ((gsil.timer)&&(NULL==timer)) {
    /* removing handler, so remove timer */
    sil_stopTimerDisplay();
  }
  gsil.timer=timer;
}

void sil_setTimeval(UINT amount) {
  if (!(gsil.init)) return;
  gsil.amount=amount;
  sil_setTimerDisplay(amount);
  if (0==amount) sil_stopTimerDisplay();
}

UINT sil_getTimeval() {
  if (!(gsil.init)) return 0;
  return gsil.amount;
}


/*****************************************************************************
  Close & cleanup SIL

 *****************************************************************************/

void sil_destroySIL() {
  sil_destroyDisplay();
  gsil.init=0;
}

