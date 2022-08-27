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
 UINT init;
 UINT (*timer)(SILEVENT *);
 UINT amount;
 UINT dragged;
} GSIL;
static GSIL gv;


/* Function: sil_initSIL

  initialize SIL context for all other calls. It will be used to contain program
  related stuff like initializing logging and global variables

  Parameters:

   width           - width of window to create (or screen display).
   height          - height of window to create (or screen display).
   title           - Title to use for window (only for windowed environments).
   hInstance       - Only for Windows/GDI; pass Windows instance handle from WinMain.
                     Set to NULL for non-Windows and/or SDL environments

  Returns:

   Error code or SILERR_ALLOK (0) if all went ok


*/
UINT sil_initSIL(UINT width, UINT height, char *title, void *hInstance) {
  UINT ret=SILERR_ALLOK;
  UINT err=0;

  /* initialize global variables */
  gv.ActiveLayer=NULL;
  gv.quit=0;


  gv.init=1;
  err=log_init(NULL,LOG_INFO|LOG_VERBOSE); 
  if (err) {
    log_fatal("Can't initialize logging");
  }
  if (SILERR_ALLOK!=sil_initDisplay(hInstance,width,height,title)) {
    log_fatal("Can't initialize display");
  }

  /* initialize other globals */
  sil_initDraw();
  return ret;
}

/* Function: sil_initLog

  Initialize logging parameters 

  Parameters:

    logname - name of logfile to write to. If NULL, stdout will be used
    flags   - kind of logging we want to capture, can be combined
              LOG_INFO | LOG_DEBUG | LOG_VERBOSE

  Returns:
    SILLERR_ALLOK (0) when done, Error on any other code

  Remarks:
    Only call this *after* calling <sil_initSIL>

    If not called at start, "LOG_INFO" & LOG_VERBOSE information will be logged to 
    stdout. Prevent logging by setting logname to NULL and flags to 0 

*/
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

/* Function: sil_quitLoop
   signal to stop <sil_mainLoop>. Usually to instruct SIL to clean up and quit the program.

 */
void sil_quitLoop() {
  gv.quit=1;
}

static void checkMove(SILEVENT *se) {
  SILLYR *al=NULL;
  SILLYR *tmp=NULL;
  BYTE buttons;

  al=gv.ActiveLayer;

  if ((al)&&(al->drag)) {
    /* are we dragging something ?                   */
    /* just get latest mousebutton status to be sure */
    buttons=sil_getMouse(NULL,NULL);
    if (buttons&SIL_BTN_LEFT) {
      /* yup we are still dragging */
      /* calculate new position    */
      se->x=al->relx+se->dx;
      se->y=al->rely+se->dy;
  
      gv.dragged=1;
  
      /* send out to drag handler */
      se->type=SILDISP_MOUSE_DRAG;
      se->layer=al;
      if (al->drag(se)) {
        /* if returns > 0, it will process proposed move */
        al->relx=se->x;
        al->rely=se->y;
        sil_updateDisplay();
      }
      return;
    } 
  }

  /* can we click it ?  */
  tmp=sil_findHighestClick(se->x,se->y);
  if (tmp) {
    if (tmp->pointer) {
      tmp->pointer(se);
    } else {
      sil_setCursor(SILCUR_HAND);
    }
  }


  /* no drag, but something to send hover events to ? */
  se->layer=sil_findHighestHover(se->x,se->y);
  if (NULL==se->layer) {
    /* nope, so just reset cursor if it wasn't clickable either */
    if (NULL==tmp) sil_setCursor(SILCUR_ARROW);
    /* if there was an a active layer, tell it goodbye */ 
    if ((al)&&(al->hover)) {
      se->layer=al;
      se->type=SILDISP_MOUSE_LEFT;
      if (!sil_checkFlags(al,SILFLAG_INVISIBLE)) {
        if (al->hover(se)) sil_updateDisplay();
      }
    }
    gv.ActiveLayer=NULL;
    return;
  }
  
  /* still above active layer ? */
  if ((al)&&(al->hover)) {
    if (al==se->layer) {
      /* just sent MOVE event */
      se->type=SILDISP_MOUSE_MOVE;
      if (al->hover(se)) sil_updateDisplay();
      return;
    } else {
      /* nope, send active layer leaving notice */
      tmp=se->layer;
      se->layer=al;
      se->type=SILDISP_MOUSE_LEFT;
      if (!sil_checkFlags(al,SILFLAG_INVISIBLE)) {
        if (al->hover(se)) sil_updateDisplay();
      }
      se->layer=tmp;
    }
  }

  /* so..at this point it is not active layer */
  /* but it has hover, so must be entering    */
  se->type=SILDISP_MOUSE_ENTER;
  if (se->layer->hover(se)) sil_updateDisplay();
  gv.ActiveLayer=se->layer;
  if (se->layer->pointer) {
    se->layer->pointer(se);
  } else {
    sil_setCursor(SILCUR_HAND);
  }
   
}

/* Function: sil_mainLoop
   Wait for events from timer, mouse and/or keyboard to handle. 
   this loop will continiously process incoming display events and redeligate them to the 
   appropriate program or layer handlers

  Events:
   - *timer* event *SILLDISP_TIMER* will be send to handler set via <sil_setTimerHandler> globally 
   for the whole program
   - *mouse button* events, like *SILDISP_MOUSE_UP* *SILDISP_MOUSE_DOWN* *SILDISP_MOUSEWHEEL* 
   will be send to the highest visible layer right under mouse cursor that has a registered 
   a mousehandler via <sil_setClickHandler> 
   - *mouse move* events, If mousepointers is above a visible layer with hoverhandler set, 
   *SILDISP_MOUSE_ENTER* will be send to hoverhandler that has been set via <sil_setHoverHandler >.
   Any mousemovement later, it will send *SILDISP_MOUSE_MOVE* events.
   If mouse leaves the layer, it will send a *SILDISP_MOUSE_LEFT*. 
   - *mouse drag* event, is a combination of *SILDISP_MOUSE_DOWN* and *SILDISP_MOUSE_MOVE*, that 
   will be intepreted as a *SILDISP_MOUSE_DRAG* if the highest visible layer under mouse cursor 
   also has a draghandler set via <sil_setDragHandler>
   - *keypress* event, SILDISP_KEY_UP or SILDISP_KEY_DOWN send to the highest layer that has a 
   keyhandler registered - via <sil_setKeyHandler> for every or specific keys or combinations. 

  Remarks: 
  - Mainloop is blocking and waiting for event after event until window is closed or <sil_quitLoop> 
    event is called. If any other time-critical task has to be handled, you might use timer for this,
    or use a separate thread allongside (outside scope of SIL, very platform dependend).
  - Mouse handler looks for *visible* pixels right under mouse cursor. Therefore, a layer under 
    another layer - and that do have both click/hover/drag handlers, can be selected to send event to 
    if the layer above has transparant pixel at that point. If this is unwanted, you can set the 
    "*SILFLAG_MOUSEALLPIX*" via <sil_setFlags>, this will make the whole -with or without transparant 
    pixels - layer (rectangular form) selectable. 
  - Mouse handlers walk from highest layer to lowest finding the first layer with the appropiate 
    handler set. Sometimes this is unwanted, like when using a layer as a "pop-up window", and you 
    want to prevent anly events send to layers under this "window". You can prevent this by setting 
    flag *SILFLAG_MOUSESHIELD* via <sil_setFlags>, and unset it after "window" layer isn't needed 
    anymore.
  - Key events can be send to any layer, visible or not and independend of mouse position, that 
    registers the keyevent with that particular event. However it might be more usefull to have 
    one layer with a handler handling (and maybe dispatching) *all* keyevents as a kind of 
    catch-all, and only registers particulair key-combo's for other layers, like function-keys 
    bounded to handler of layer that is used as menubutton or some sort.


 */
void sil_mainLoop() {
  SILEVENT *se;
  SILLYR *al=NULL;

  do {
    se=sil_getEventDisplay();
    if (NULL==se) break; /* should not happen */
    al=gv.ActiveLayer;

    switch (se->type) {

      case SILDISP_TIMER:
        gv.amount=0;
        if (gv.timer) 
          if (gv.timer(se)) sil_updateDisplay();
        break;

      case SILDISP_MOUSE_UP:
        if (al) {
          if (al->click) {
            se->layer=al;
            if (al->click(se)) {
              sil_updateDisplay();
            }
          }
        }
        gv.dragged=0;
        se->layer=sil_findHighestClick(se->x,se->y);
        if (se->layer) {
          gv.ActiveLayer=se->layer;
          if (se->layer->pointer) {
            se->layer->pointer(se);
          } else {
            sil_setCursor(SILCUR_HAND);
          }
          if (se->layer->click) {
            if (se->layer->click(se)) sil_updateDisplay();
          }
        } else {
          /* no layer found, check if hover is enabled */
          se->layer=sil_findHighestHover(se->x,se->y);
          if (se->layer) {
            if (se->layer->pointer) {
              se->layer->pointer(se);
            } else {
              sil_setCursor(SILCUR_HAND);
            }
          } else {
            sil_setCursor(SILCUR_ARROW);
            gv.ActiveLayer=NULL;
          }
        }
        break;

      case SILDISP_MOUSE_DOWN:
        se->layer=sil_findHighestClick(se->x,se->y);
        if (se->layer) {
          gv.ActiveLayer=se->layer;
          if (se->layer->pointer) {
            se->layer->pointer(se);
          } else {
            sil_setCursor(SILCUR_HAND);
          }
          if (se->layer->click) {
            if (se->layer->click(se)) sil_updateDisplay();
          }
        } else {
          /* no layer found, just revert to standard mousepointer */
          sil_setCursor(SILCUR_ARROW);
          gv.ActiveLayer=NULL;
        }
        break;

      case SILDISP_MOUSEWHEEL:
        se->layer=sil_findHighestClick(se->x,se->y);
        if (se->layer) {
          gv.ActiveLayer=se->layer;
          if (se->layer->pointer) {
            se->layer->pointer(se);
          } else {
            sil_setCursor(SILCUR_HAND);
          }
          if (se->layer->click) {
            if (se->layer->click(se)) sil_updateDisplay();
          }
        } else {
          sil_setCursor(SILCUR_ARROW);
          gv.ActiveLayer=NULL;
        }
        break;

      case SILDISP_MOUSE_MOVE:
        checkMove(se);
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
  } while ((0==gv.quit)&&(SILDISP_QUIT!=se->type));
}


/* 
  Function: sil_err2Txt
  Optional function to convert any internal SIL error code to a more human understandable 
  form. 

Paramers: 
  errorcode - SIL error code

Returns: 
  string that explains errorcode

*/
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

/*
Function: sil_setTimerHandler
  set timer handler 

Parameters: 
  timer - own handler function that accepts a pointer to *SILEVENT* and returns 
  1 if display has to be updated or not (0). 

Remarks:
  - Only one timer can be set per SIL program. Therefore, setting a Timerhandler
  will overwrite any existing -and running !- timers.
  - Don't forget to use <sil_setTimeval> to set time-out of timer
  - Timers are not auto-repeating, if you want to, call <sil_setTimeval> at 
  end of your timerhandler.
  - You can always cancel any running timers by calling <sil_setTimeval> with 0

*/
void sil_setTimerHandler(UINT (*timer)(SILEVENT *)) {
  if (!(gv.init)) return;
  if ((gv.timer)&&(NULL==timer)) {
    /* removing handler, so remove timer */
    sil_stopTimerDisplay();
  }
  gv.timer=timer;
}

/*
Function: sil_setTimeval
  set timer to given amount of seconds

Parameters:
  UINT amount - delay in seconds until timer handler is called. use '0' to 
                cancel any running timer

Remarks:
  - will cancel and overwrite any previous -running- timers
  - timer isn't repeating, so do set it again after handling timer event if you want to
  
*/
void sil_setTimeval(UINT amount) {
  if (!(gv.init)) return;
  gv.amount=amount;
  sil_setTimerDisplay(amount);
  if (0==amount) sil_stopTimerDisplay();
}

/*
Function: sil_setTimeval
  If you have trouble remembering things, retrieve the amount of seconds you did set your timer to

Returns:
  UINT - amount of seconds

*/
UINT sil_getTimeval() {
  if (!(gv.init)) return 0;
  return gv.amount;
}


/*
Function: sil_destroySIL
  Close & cleanup SIL

*/
void sil_destroySIL() {
  sil_destroyDisplay();
  gv.init=0;
}

