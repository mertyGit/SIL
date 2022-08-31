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
  void *hInstance=NULL;
#endif

  SILEVENT *se=NULL;
  SILLYR *background,*top;
  SILFONT *font;

  BYTE onlyDrawLine=0;


  sil_initSIL(1000,700,"Testing SIL drawing primitives",hInstance);
  sil_setLog(NULL,LOG_INFO|LOG_DEBUG|LOG_VERBOSE);


  font=sil_loadFont("architectsdaughter_thickoutline_24px.fnt");
  background=sil_addLayer(0,0,1000,1040,0);
  sil_paintLayer(background,30,60,120,255);
  top=sil_addLayer(0,0,1000,1040,0);


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
  sil_drawText(top,font,"drawPixel",10,90,SILTXT_KEEPCOLOR);
  

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
  sil_drawText(top,font,"blendPixel",10,240,SILTXT_KEEPCOLOR);


  sil_setForegroundColor(SILCOLOR_LIME,255);

  sil_setDrawWidth(1);
  sil_setBackgroundColor(SILCOLOR_BLACK,0);
  sil_drawCircle(top,10+650,370,40);
  sil_drawText(top,font,"single circle,\nno AA",10+610,370+50,SILTXT_KEEPCOLOR);

  sil_setDrawWidth(5);
  sil_drawCircle(top,10+850,370,40);
  sil_drawText(top,font,"linewidth=5\nno AA",10+810,370+50,SILTXT_KEEPCOLOR);

  sil_setDrawWidth(1);
  sil_setBackgroundColor(SILCOLOR_BLACK,0);
  sil_drawCircleAA(top,10+650,530,40);
  sil_drawText(top,font,"single circle,\nwith AA",10+610,530+50,SILTXT_KEEPCOLOR);

  sil_setDrawWidth(5);
  sil_drawCircleAA(top,10+850,530,40);
  sil_drawText(top,font,"linewidth=5\nwith AA",10+810,530+50,SILTXT_KEEPCOLOR);



  sil_setDrawWidth(3);
  sil_setForegroundColor(SILCOLOR_YELLOW,255);
  sil_setBackgroundColor(SILCOLOR_MAGENTA,255);
  sil_drawRectangle(top,300,150,80,80);

  sil_setBackgroundColor(SILCOLOR_LIME,255);
  sil_drawRectangle(top,350,100,100,60);

  sil_setBackgroundColor(SILCOLOR_MAGENTA,100);
  sil_drawRectangle(top,400,50,80,80);

  sil_setBackgroundColor(SILCOLOR_RED,0);
  sil_drawRectangle(top,400,150,80,80);

  sil_setDrawWidth(0);
  sil_setBackgroundColor(SILCOLOR_NAVY,255);
  sil_drawRectangle(top,300,50,80,80);

  sil_drawText(top,font,"drawRectangle",300+10,240,SILTXT_KEEPCOLOR);


  sil_setDrawWidth(3);
  sil_setForegroundColor(SILCOLOR_YELLOW,255);
  sil_setBackgroundColor(SILCOLOR_MAGENTA,255);
  sil_drawCircleAA(top,640,180,40);

  sil_setBackgroundColor(SILCOLOR_LIME,255);
  sil_drawCircleAA(top,690,120,60);

  sil_setBackgroundColor(SILCOLOR_MAGENTA,100);
  sil_drawCircleAA(top,750,100,40);

  sil_setBackgroundColor(SILCOLOR_RED,0);
  sil_drawCircleAA(top,750,180,40);

  sil_setDrawWidth(0);
  sil_setBackgroundColor(SILCOLOR_NAVY,255);
  sil_drawCircleAA(top,640,100,40);

  sil_drawText(top,font,"drawCircleAA",600+10,240,SILTXT_KEEPCOLOR);

  sil_setDrawWidth(3);
  sil_setForegroundColor(SILCOLOR_YELLOW,255);
  sil_setBackgroundColor(SILCOLOR_LIME,255);
  sil_drawCircle(top,840,180,40);

  sil_setBackgroundColor(SILCOLOR_LIME,0);
  sil_drawCircle(top,870,160,40);

  sil_setDrawWidth(0);
  sil_setBackgroundColor(SILCOLOR_NAVY,255);
  sil_drawCircle(top,840,120,40);
  sil_drawText(top,font,"drawCircle",800+10,240,SILTXT_KEEPCOLOR);

  sil_setDrawWidth(3);
  sil_setBackgroundColor(SILCOLOR_MAGENTA,100);
  sil_drawCircle(top,880,140,40);

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
