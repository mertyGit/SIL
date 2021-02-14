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
    sil_initSIL(1000,1000,"Testing SIL Filters",hInstance,NULL,LOG_INFO|LOG_DEBUG|LOG_VERBOSE);
  #else
    sil_initSIL(1000,1000,"Testing SIL Filters",NULL,NULL,LOG_INFO|LOG_DEBUG|LOG_VERBOSE);
  #endif


  font=sil_loadFont("bahnschrift.fnt");

  background=sil_addLayer(1000,1000,0,0,0);

  lyr=sil_PNGtoNewLayer("testpic1.png",x,y);
  sil_drawTextLayer(background,font,"no filter",x,y+100,0);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic2.png",x,y);
  sil_drawTextLayer(background,font,"grayscale",x,y+100,0);
  sil_applyFilterLayer(lyr,SILFLTR_GRAYSCALE);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic3.png",x,y);
  sil_drawTextLayer(background,font,"flip-x",x,y+100,0);
  sil_applyFilterLayer(lyr,SILFLTR_FLIPX);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic4.png",x,y);
  sil_drawTextLayer(background,font,"flip-y",x,y+100,0);
  sil_applyFilterLayer(lyr,SILFLTR_FLIPY);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic5.png",x,y);
  sil_drawTextLayer(background,font,"reverse color",x,y+100,0);
  sil_applyFilterLayer(lyr,SILFLTR_REVERSECOLOR);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic6.png",x,y);
  sil_drawTextLayer(background,font,"blur",x,y+100,0);
  sil_applyFilterLayer(lyr,SILFLTR_BLUR);

  /* --- */

  y+=150;
  x=50;

  lyr=sil_PNGtoNewLayer("testpic7.png",x,y);
  sil_drawTextLayer(background,font,"ligthen",x,y+100,0);
  sil_applyFilterLayer(lyr,SILFLTR_LIGHTEN);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic8.png",x,y);
  sil_drawTextLayer(background,font,"ligthen x 2",x,y+100,0);
  sil_applyFilterLayer(lyr,SILFLTR_LIGHTEN);
  sil_applyFilterLayer(lyr,SILFLTR_LIGHTEN);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic9.png",x,y);
  sil_drawTextLayer(background,font,"darken",x,y+100,0);
  sil_applyFilterLayer(lyr,SILFLTR_DARKEN);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic10.png",x,y);
  sil_drawTextLayer(background,font,"darken x 2",x,y+100,0);
  sil_applyFilterLayer(lyr,SILFLTR_DARKEN);
  sil_applyFilterLayer(lyr,SILFLTR_DARKEN);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic11.png",x,y);
  sil_drawTextLayer(background,font,"rotate color",x,y+100,0);
  sil_applyFilterLayer(lyr,SILFLTR_ROTATECOLOR);
  x+=150;

  lyr=sil_PNGtoNewLayer("testpic12.png",x,y);
  sil_drawTextLayer(background,font,"rotate color x 2",x,y+100,0);
  sil_applyFilterLayer(lyr,SILFLTR_ROTATECOLOR);
  sil_applyFilterLayer(lyr,SILFLTR_ROTATECOLOR);
  x+=150;

  /* --- */

  y+=150;
  x=50;
  lyr=sil_PNGtoNewLayer("star.png",x,y);
  sil_drawTextLayer(background,font,"border",x,y+100,0);
  sil_applyFilterLayer(lyr,SILFLTR_BORDER);
  x+=150;

  lyr=sil_PNGtoNewLayer("star.png",x,y);
  sil_drawTextLayer(background,font,"crop alpha",x,y+100,0);
  sil_applyFilterLayer(lyr,SILFLTR_AUTOCROPALPHA);
  x+=150;

  lyr=sil_PNGtoNewLayer("star.png",x,y);
  sil_drawTextLayer(background,font,"+border",x,y+100,0);
  sil_applyFilterLayer(lyr,SILFLTR_AUTOCROPALPHA);
  sil_applyFilterLayer(lyr,SILFLTR_BORDER);
  x+=150;

  lyr=sil_PNGtoNewLayer("starwhite.png",x,y);
  sil_drawTextLayer(background,font,"no crop",x,y+100,0);
  x+=150;

  lyr=sil_PNGtoNewLayer("starwhite.png",x,y);
  sil_applyFilterLayer(lyr,SILFLTR_AUTOCROPFIRST);
  sil_drawTextLayer(background,font,"crop first",x,y+100,0);
  x+=150;




  sil_updateDisplay();

  do {
    se=sil_getEventDisplay(1);
    if (se->type==SILDISP_QUIT) break;
    sil_updateDisplay();
  } while (se!=NULL && se->key!=SILKY_ESC);


  sil_destroyFont(font);
  sil_destroySIL();


  return 0;
}
