#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef SIL_W32
  #include <windows.h>
#endif
#include "log.h"
#include "sil.h"

#ifdef SIL_W32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#else
int main() {
#endif

  SILEVENT *se=NULL;
  SILLYR *background,*top;
  SILFONT *font;


  #ifdef SIL_W32
    sil_initSIL(1000,1000,"Testing SIL Filters",hInstance);
  #else
    sil_initSIL(1000,1000,"Testing SIL Filters",NULL);
  #endif
  sil_setLog(NULL,LOG_INFO|LOG_DEBUG|LOG_VERBOSE);


  font=sil_loadFont("bahnschrift.fnt");
  background=sil_addLayer(1000,1000,0,0,0);
  sil_paintLayer(background,30,60,120,255);
  top=sil_addLayer(1000,1000,0,0,0);


  for (int x=0;x<255;x++) {
    for (int y=0;y<50;y++) {
      sil_setForegroundColor(255,255,0,x);
      sil_drawPixel(top,10+x,y+10);
    }
  }
  
  for (int x=0;x<255;x++) {
    for (int y=0;y<50;y++) {
      sil_setForegroundColor(255,0,0,(255-x));
      sil_drawPixel(top,10+x,y+35);
    }
  }
  sil_drawTextLayer(top,font,"drawPixel",10,90,SILTXT_KEEPCOLOR);
  

  for (int x=0;x<255;x++) {
    for (int y=0;y<50;y++) {
      sil_setForegroundColor(255,255,0,x);
      sil_blendPixel(top,10+x,y+160);
    }
  }
  
  for (int x=0;x<255;x++) {
    for (int y=0;y<50;y++) {
      sil_setForegroundColor(255,0,0,(255-x));
      sil_blendPixel(top,10+x,y+185);
    }
  }
  sil_drawTextLayer(top,font,"blendPixel",10,240,SILTXT_KEEPCOLOR);



  sil_updateDisplay();

  do {
    se=sil_getEventDisplay();
    if (se->type==SILDISP_QUIT) break;
    sil_updateDisplay();
  } while (se!=NULL && se->key!=SILKY_ESC);


  sil_destroyFont(font);
  sil_destroySIL();


  return 0;
}
