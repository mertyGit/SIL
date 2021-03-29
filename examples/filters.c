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

  unsigned char r,g,b;
  SILEVENT *se=NULL;
  SILLYR *background,*lyr;
  UINT err;
  BYTE move=0;
  BYTE update=0;
  SILFONT *font;
  UINT x=50;
  UINT y=50;


  printf("sil_init...\n");
  #ifdef SIL_W32
    sil_initSIL(1000,1000,"Testing SIL Filters",hInstance);
  #else
    sil_initSIL(1000,1000,"Testing SIL Filters",NULL);
  #endif
  sil_setLog(NULL,LOG_INFO|LOG_DEBUG|LOG_VERBOSE);


  //font=sil_loadFont("architectsdaughter_thickoutline_20px.fnt");
  font=sil_loadFont("sourcesanspro_16px.fnt");

  background=sil_addLayer(1000,1000,0,0,0);

  lyr=sil_PNGtoNewLayer("testpic1.png",x,y);
  sil_drawText(background,font,"no filter",x,y+100,0);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic2.png",x,y);
  sil_drawText(background,font,"grayscale",x,y+100,0);
  sil_grayFilter(lyr);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic3.png",x,y);
  sil_drawText(background,font,"flip-x",x,y+100,0);
  sil_flipxFilter(lyr);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic4.png",x,y);
  sil_drawText(background,font,"flip-y",x,y+100,0);
  sil_flipyFilter(lyr);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic5.png",x,y);
  sil_drawText(background,font,"reverse color",x,y+100,0);
  sil_reverseColorFilter(lyr);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic6.png",x,y);
  sil_drawText(background,font,"blur",x,y+100,0);
  sil_blurFilter(lyr);

  /* --- */

  y+=150;
  x=50;

  lyr=sil_PNGtoNewLayer("testpic7.png",x,y);
  sil_drawText(background,font,"bright. +50",x,y+100,0);
  sil_brightnessFilter(lyr,50);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic8.png",x,y);
  sil_drawText(background,font,"bright. +100",x,y+100,0);
  sil_brightnessFilter(lyr,100);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic9.png",x,y);
  sil_drawText(background,font,"bright -50",x,y+100,0);
  sil_brightnessFilter(lyr,-50);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic10.png",x,y);
  sil_drawText(background,font,"bright -100",x,y+100,0);
  sil_brightnessFilter(lyr,-100);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic11.png",x,y);
  sil_drawText(background,font,"rotate color",x,y+100,0);
  sil_rotateColorFilter(lyr);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic12.png",x,y);
  sil_drawText(background,font,"rotate color x 2",x,y+100,0);
  sil_rotateColorFilter(lyr);
  sil_rotateColorFilter(lyr);
  x+=150;

  /* --- */

  y+=150;
  x=50;
  lyr=sil_PNGtoNewLayer("star.png",x,y);
  sil_drawText(background,font,"border",x,y+100,0);
  sil_borderFilter(lyr);
  x+=150;

  lyr=sil_PNGtoNewLayer("star.png",x,y);
  sil_drawText(background,font,"crop alpha",x,y+100,0);
  sil_cropAlphaFilter(lyr);
  x+=150;

  lyr=sil_PNGtoNewLayer("star.png",x,y);
  sil_drawText(background,font,"+border",x,y+100,0);
  sil_cropAlphaFilter(lyr);
  sil_borderFilter(lyr);
  x+=150;

  lyr=sil_PNGtoNewLayer("starwhite.png",x,y);
  sil_drawText(background,font,"no crop",x,y+100,0);
  x+=150;

  lyr=sil_PNGtoNewLayer("starwhite.png",x,y);
  sil_drawText(background,font,"crop first pix",x,y+100,0);
  sil_cropFirstpixelFilter(lyr);
  x+=150;

  lyr=sil_PNGtoNewLayer("starwhite.png",x,y);
  sil_drawText(background,font,"alpha first pix",x,y+100,0);
  sil_alphaFirstpixelFilter(lyr);
  x+=150;



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
