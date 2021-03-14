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
    sil_initSIL(600,1000,"Testing SIL drawing primitives",hInstance);
  #else
    sil_initSIL(600,1000,"Testing SIL drawing primitives",NULL);
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


  sil_setForegroundColor(0,255,0,255);
  sil_drawBigLine(top,1,50,30,40);
  sil_drawTextLayer(top,font,"single line no AA (x8)",10,420,SILTXT_KEEPCOLOR);

  sil_drawBigLineAA(top,1,50+20,30,40+20);
  sil_drawTextLayer(top,font,"single line with AA (x8)",10,580,SILTXT_KEEPCOLOR);

  sil_setDrawWidth(5);
  sil_drawBigLine(top,1+40,50,30+40,40);
  sil_drawTextLayer(top,font,"linewidth=5 no AA (x8)",10+300,420,SILTXT_KEEPCOLOR);

  sil_drawBigLineAA(top,1+40,50+20,30+40,40+20);
  sil_drawTextLayer(top,font,"linewidth=5 with AA (x8)",10+300,580,SILTXT_KEEPCOLOR);

  sil_setDrawWidth(1);
  sil_drawCircle(top,80,680,50);
  sil_drawTextLayer(top,font,"single circle, no AA",10,730,SILTXT_KEEPCOLOR);

  sil_setDrawWidth(5);
  sil_drawCircle(top,300+80,680,50);
  sil_drawTextLayer(top,font,"linewidth=5, no AA",300+10,730,SILTXT_KEEPCOLOR);

  sil_setDrawWidth(1);
  sil_drawCircleAA(top,80,680+150,50);
  sil_drawTextLayer(top,font,"single circle, with AA",10,730+150,SILTXT_KEEPCOLOR);

  sil_setDrawWidth(5);
  sil_drawCircleAA(top,300+80,680+150,50);
  sil_drawTextLayer(top,font,"linewidth=5, with AA",300+10,730+150,SILTXT_KEEPCOLOR);

  sil_setDrawWidth(3);
  sil_setForegroundColor(255,0,255,255);
  sil_setBackgroundColor(255,255,0,255);
  sil_drawRectangle(top,300,150,80,80);

  sil_setForegroundColor(0,255,0,255);
  sil_drawRectangle(top,350,100,100,60);

  sil_setForegroundColor(255,0,255,100);
  sil_drawRectangle(top,400,50,80,80);

  sil_setForegroundColor(255,0,0,0);
  sil_drawRectangle(top,400,150,80,80);
  sil_drawTextLayer(top,font,"drawRectangle",300+10,240,SILTXT_KEEPCOLOR);


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
