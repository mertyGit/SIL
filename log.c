/*

   log.c Copyright (c) 2021 Remco Schellekens, see LICENSE for more details
   This file implements a stand-alone logging functionality which can be 
   used for multiple c projects

*/
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "log.h"




/*****************************************************************************
   Global var struct, used to minimize the use of context/descriptors/..     
 *****************************************************************************/

/*  

   level flag (1=on, 0=off)                                                  
                                                                             
    Debug:   Stuff the programmer wants to know, in case of problems         
    Verbose: Tracing information the user wants to know, in case of problems 
    Info:    General information about status, for operational reasons       
                                                                             
   Setting Debug turns on Verbose                                            
   Info is default on, could be turned of for "silent mode"                 

*/

typedef struct {
  unsigned char flags;
  int didInit;
  FILE * fp;
  int boxcnt;
} LOG_VARS;

static LOG_VARS logVars;

/*****************************************************************************
   (re-)set logging mode                                                     
   in: flags (LOG_INFO | LOG_DEBUG | LOG_VERBOSE )                          
   out:LOG_OK on success, LOG_ERR on error                                 
 *****************************************************************************/

int log_set_f(unsigned char flags) {
  /* set all flags on if debug and skip unknown flags */
  if (flags&LOG_DEBUG) 
    logVars.flags=LOG_INFO|LOG_DEBUG|LOG_VERBOSE;
  else
    logVars.flags=flags&(LOG_INFO|LOG_DEBUG|LOG_VERBOSE);
  return LOG_OK;
}

/*****************************************************************************
   check if flags are set or not
   in: flags (LOG_INFO | LOG_DEBUG | LOG_VERBOSE )                          
   out:LOG_OK if set, LOG_NOT if not
 *****************************************************************************/

int log_has_f(unsigned char flags) {
  if (logVars.flags&flags) return LOG_OK;
  return LOG_ERR;
}



/*****************************************************************************
   Initialize logging module                                                 
   in: file name of log + flags (LOG_INFO | LOG_DEBUG | LOG_VERBOSE)        
       if filename is NULL, w'll use stdout                                  
   out:LOG_OK on success, LOG_ERR on error                                
 *****************************************************************************/
int log_init(char *fname, unsigned char flags) {
  logVars.flags=LOG_INFO;
  logVars.fp=stdout;
  logVars.boxcnt=0;
  if (log_set_f(flags)!=LOG_OK) return LOG_ERR;
  if (fname!=NULL) {
    logVars.fp=fopen(fname,"a+");
    if (NULL==logVars.fp) {
      logVars.fp=stdout;
      log_warn("logfile cannot be opened, using stdout\n");
    }
  }
  logVars.didInit=LOG_INITDONE;
  return LOG_OK;
}

/*****************************************************************************
   internal function, used by all log types to print stuff in respect with   
   information and line width                                               
 *****************************************************************************/
static void log_write(char *msg,char *prefix,char *postfix) {
  char buffer[LOG_MAX_LINE+1]; 
  struct timeval tv;
  time_t ltime;

  int begin=0;
  int len=0;
  int w;
  int cnt;

  len=strnlen(msg,LOG_MAX_LINE);
  w=LOG_LINE_WIDTH-strlen(prefix)-strlen(postfix)-logVars.boxcnt*2;
  if (w<1) w=LOG_LINE_WIDTH;

  while (begin<len) {
    /* print time header */
    memset(buffer,0,sizeof(buffer));
    gettimeofday(&tv,NULL);
    ltime=tv.tv_sec; /* using ltime prevents localtime / time_t differences between environments */
    strftime(buffer,LOG_MAX_LINE,"%Y%m%d %H:%M:%S",localtime(&ltime));
    fprintf(logVars.fp,"%s%06ld| ",buffer,tv.tv_usec);
    for (cnt=0;cnt<logVars.boxcnt;cnt++) fprintf(logVars.fp,"|");
    fprintf(logVars.fp,"%s",prefix);
    for (cnt=0;((cnt<w)&&(begin<len));begin++) {
      /* skip: non-printable chars and line-ends */
      if ((!iscntrl(msg[begin]))&&isprint(msg[begin])) {
        fprintf(logVars.fp,"%c",msg[begin]);
        cnt++;
      }
    }
    for (;cnt<w;cnt++) fprintf(logVars.fp," "); /* fill-up line */
    for (cnt=0;cnt<logVars.boxcnt;cnt++) fprintf(logVars.fp,"|");
    fprintf(logVars.fp,"%s",postfix);
    fprintf(logVars.fp,"\n");
  }
  fflush(logVars.fp);
}

/*****************************************************************************
   Fatal; most cases memory problems, so dont create extra problems
   and we are not sure what can be done or closed; just stop program

 *****************************************************************************/
void log_fatal(const char *msg) {
  printf("FATAL: %s\n",msg);
  exit(1);
}

/*****************************************************************************
   log information request                                                   
   in: printf like format                                                   
 *****************************************************************************/
int log_info(const char *msg, ...) {
  char buffer[LOG_MAX_LINE+1];
  va_list ap;

  if (logVars.didInit!=LOG_INITDONE) return LOG_ERR;

  if (0==(logVars.flags&LOG_INFO)) return LOG_ERR;

  memset(buffer,0,sizeof(buffer));
  va_start(ap,msg);
  vsnprintf(buffer,LOG_MAX_LINE,msg,ap);
  va_end(ap);
  log_write(buffer,"","");

  return LOG_OK;
}


/*****************************************************************************
   log error message, this wil be same as info, except "ERR:" prefix        
   in: printf like format                                                  
 *****************************************************************************/
int log_err(const char *msg, ...) {
  char buffer[LOG_MAX_LINE+1];
  va_list ap;

  if (logVars.didInit!=LOG_INITDONE) return LOG_ERR;

  if (0==(logVars.flags&LOG_INFO)) return LOG_ERR;

  memset(buffer,0,sizeof(buffer));
  va_start(ap,msg);
  vsnprintf(buffer,LOG_MAX_LINE,msg,ap);
  va_end(ap);
  log_write(buffer,"ERR: ","");

  return LOG_OK;
}


/*****************************************************************************
   log warning message, this wil be same as verbose, except "WRN:" prefix   
   in: printf like format                                                  
 *****************************************************************************/
int log_warn(const char *msg, ...) {
  char buffer[LOG_MAX_LINE+1];
  va_list ap;

  if (logVars.didInit!=LOG_INITDONE) return LOG_ERR;

  if (0==(logVars.flags&LOG_VERBOSE)) return LOG_ERR;

  memset(buffer,0,sizeof(buffer));
  va_start(ap,msg);
  vsnprintf(buffer,LOG_MAX_LINE,msg,ap);
  va_end(ap);
  log_write(buffer,"WARN: ","");

  return LOG_OK;
}

/*****************************************************************************
   log verbose request    
   in: printf like format
 *****************************************************************************/
int log_verbose(const char *msg, ...) {
  char buffer[LOG_MAX_LINE+1];
  va_list ap;

  if (logVars.didInit!=LOG_INITDONE) return LOG_ERR;

  if (0==(logVars.flags&LOG_VERBOSE)) return LOG_ERR;

  memset(buffer,0,sizeof(buffer));
  va_start(ap,msg);
  vsnprintf(buffer,LOG_MAX_LINE,msg,ap);
  va_end(ap);
  log_write(buffer,"","");

  return LOG_OK;
}

/*****************************************************************************
   log debug request                                                         
   in: printf like format                                                   
                                                                           
   The "log_mark" function is actually macro that calls this function     
   to fill in file and line                                              
 *****************************************************************************/
int log_debug(const char *msg, ...) {
  char buffer[LOG_MAX_LINE+1];
  va_list ap;

  if (logVars.didInit!=LOG_INITDONE) return LOG_ERR;

  if (0==(logVars.flags&LOG_DEBUG)) return LOG_ERR;

  memset(buffer,0,sizeof(buffer));
  va_start(ap,msg);
  vsnprintf(buffer,LOG_MAX_LINE,msg,ap);
  va_end(ap);
  log_write(buffer,"DBG:  ","");

  return LOG_OK;
}


/*****************************************************************************
   Create nice divider '------' in logfile 
   out:LOG_OK on success, LOG_ERR on error
 *****************************************************************************/
int log_div() {
  int cnt;
  char buffer[LOG_MAX_LINE+1];

  memset(buffer,0,sizeof(buffer));
  if (logVars.didInit!=LOG_INITDONE) return LOG_ERR;

  if (0==(logVars.flags&LOG_INFO)) return LOG_ERR;

  for (cnt=0;cnt<LOG_LINE_WIDTH-logVars.boxcnt*2;cnt++) buffer[cnt]='-';
  buffer[LOG_LINE_WIDTH]='\0';
  log_write(buffer,"","");

  return LOG_OK;
}

/*****************************************************************************
   More ASCII art, until 'log_box_off' everything will be printed inside an 
   box, to group elements. Result looks like this:                         
                                                                          
   +------------+                                                        
   | line       |                                                       
   | other line |                                                      
   +------------+                                                     
                                                                     
 *****************************************************************************/
int log_box_on() {
  if (logVars.didInit!=LOG_INITDONE) return LOG_ERR;
  int cnt;
  char buffer[LOG_MAX_LINE+1];

  buffer[0]='+';
  for (cnt=1;cnt<LOG_LINE_WIDTH-logVars.boxcnt*2-1;cnt++) buffer[cnt]='-';
  buffer[cnt]='+';
  buffer[cnt+1]='\0';
  log_write(buffer,"","");
  logVars.boxcnt++;

  return LOG_OK;
}

int log_box_off() {
  int cnt;
  char buffer[LOG_MAX_LINE+1];

  if (logVars.didInit!=LOG_INITDONE) return LOG_ERR;

  if (logVars.boxcnt>0) logVars.boxcnt--;
  buffer[0]='+';
  for (cnt=1;cnt<LOG_LINE_WIDTH-logVars.boxcnt*2-1;cnt++) buffer[cnt]='-';
  buffer[cnt]='+';
  buffer[cnt+1]='\0';
  log_write(buffer,"","");

  return LOG_OK;
}

/*****************************************************************************
   Dump memory at addr and len bytes as hexdump in log                      
   debug has to be set to do this                                          
 *****************************************************************************/
int log_dump(void *addr,size_t ilen) {
    size_t i;
    size_t len;
  int cnt=0;
  int chopped=0;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;
    char line[LOG_MAX_LINE+1];



  if (logVars.didInit!=LOG_INITDONE) return LOG_ERR;
  if (0==(logVars.flags&LOG_DEBUG)) return LOG_ERR;
  if (ilen<1) return LOG_ERR;

  len=ilen;
  if (len>LOG_MAX_ROW*16) {
    len=LOG_MAX_ROW*16;
    chopped=1;
  }

    memset(line,0,sizeof(line));
  strcpy(line,"       ");
  for (cnt=0; cnt<16; cnt++) {
    sprintf(&line[strlen(line)],"  %01x",cnt);
  }
  sprintf(&line[strlen(line)],"  ");
  for (cnt=0; cnt<16; cnt++) {
    sprintf(&line[strlen(line)],"%01x",cnt);
  }
  log_write(line,"","");
  log_write("        -----------------------------------------------"\
            "  ----------------", "","");

  /* top part */

    memset(line,0,sizeof(line));
  memset(buff,0,sizeof(buff));
    for (i=0; i<len; i++) {
        if ((i % 16) == 0) {
            if (i != 0) {
                sprintf(&line[strlen(line)],"  %s", buff);
                log_write(line,"","");
            }
            sprintf(line,"  %05lu", i);
        }
        sprintf(&line[strlen(line)]," %02x", pc[i]);
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    while ((i % 16) != 0) {
        sprintf(&line[strlen(line)],"   ");
        i++;
    }
    sprintf(&line[strlen(line)],"  %s", buff);
  log_write(line,"","");

  if (chopped) {
    /* end part (if lines has to be skipped) */
    log_write("   ","","");
    log_write("  ..skipped..","","");
    len=ilen;
    i=len-len%16;
    if (0==i) i=16;
    memset(line,0,sizeof(line));
    memset(buff,0,sizeof(buff));
    for (; i<len; i++) {
      if ((i % 16) == 0) {
        if (i != 0) {
          sprintf(&line[strlen(line)],"  %s", buff);
          log_write(line,"","");
        }
        sprintf(line,"  %05lu", i);
      }
      sprintf(&line[strlen(line)]," %02x", pc[i]);
      if ((pc[i] < 0x20) || (pc[i] > 0x7e))
        buff[i % 16] = '.';
      else
        buff[i % 16] = pc[i];
      buff[(i % 16) + 1] = '\0';
    }
    while ((i % 16) != 0) {
      sprintf(&line[strlen(line)],"   ");
      i++;
    }
    sprintf(&line[strlen(line)],"  %s", buff);
    log_write(line,"","");
  }



  return LOG_OK;
}


/*****************************************************************************
   close logging module. Closes log file, cleans up stuff if needed>         
   out:LOG_OK on success, LOG_ERR on error                                  
 *****************************************************************************/
int log_close() {
  /* Don't call us if there isn't a log to close */
  if (logVars.didInit!=LOG_INITDONE) return LOG_ERR;
  if ((logVars.fp!=NULL)&&(logVars.fp!=stdout)) {
    fclose(logVars.fp);
  }

  return LOG_OK;
}
