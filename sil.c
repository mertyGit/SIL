/* 

   sil.c CopyRight 2021 Remco Schellekens, see LICENSE for more details.

   This file contains all generic basic functions and helpers for SIL

*/

#include <stdio.h>
#include <stdlib.h>
#include "sil.h"
#include "log.h"

static SILCONTEXT st;



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

UINT sil_initSIL(UINT width, UINT height, char *title, void *hInstance,char *logname, BYTE flags) {
  UINT ret=0;
  UINT err=0;

  st.lasterr=0;
  st.init=1;
  err=log_init(logname,flags);
  if (err) {
    log_fatal("Can't initialize logging");
  }
  if (SILERR_ALLOK!=sil_initDisplay(hInstance,width,height,title)) {
    log_fatal("Can't initialize display");
  }


  log_info("INF: Initialized SIL, logging & display");
  return ret;
}

/*****************************************************************************
  Set errorcode, used internally to check on previous errors 
  or to translate error code to text into log
  In: SIL context + errorcode (UINT) => one of the "SILERR_..." 

 *****************************************************************************/

void sil_setErr(UINT errorcode) {
  if (st.init) {
    st.lasterr=errorcode;
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

UINT sil_getErr(SILCONTEXT *sil) {
  if (st.init) {
    return st.lasterr;
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
  Close & cleanup SIL

 *****************************************************************************/

void sil_destroySIL() {
  sil_destroyDisplay();
  st.init=0;
}
