#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef SIL_W32
  #include <windows.h>
#endif
#include "log.h"
#include "sil.h"

static SILLYR *eight,*eleven,*source,*copy,*instance;

UINT totop(SILEVENT *event) {
  if (event->type==SILDISP_MOUSE_DOWN) {
    sil_toTop(event->layer);
    return 1;
  }
  return 0;
}

UINT tobottom(SILEVENT *event) {
  if (event->type==SILDISP_MOUSE_DOWN) {
    sil_toBottom(event->layer);
    return 1;
  }
  return 0;
}

UINT reverse(SILEVENT *event) {
  if (event->type==SILDISP_MOUSE_DOWN) {
    sil_reverseColorFilter(event->layer);
    return 1;
  }
  return 0;
}

UINT toswap(SILEVENT *event) {
  if (event->type==SILDISP_MOUSE_DOWN) {
    sil_swap(event->layer,eight);
    return 1;
  }
  return 0;
}

UINT toabovebelow(SILEVENT *event) {
  if (event->type==SILDISP_MOUSE_DOWN) {
    if (event->val==1) 
      sil_toAbove(event->layer,eleven);
    else
      sil_toBelow(event->layer,eleven);
    return 1;
  }
  return 0;
}

UINT keyhandler(SILEVENT *event) {
    switch(event->type) {
    case SILDISP_KEY_DOWN:
      switch(event->key) {
        case SILKY_ESC:
          sil_quitLoop();
          break;

      }
      break;
  }
  return 0;
}


#ifdef SIL_W32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#else
int main() {
  void *hInstance=NULL;
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
  sil_initSIL(1000,500,"Testing SIL Order",hInstance);
  sil_setLog(NULL,LOG_INFO|LOG_DEBUG|LOG_VERBOSE);


  font=sil_loadFont("architectsdaughter_thickoutline_20px.fnt");

  background=sil_addLayer(0,0,1000,1000,0);
  sil_setKeyHandler(background,0,0,0,keyhandler);


  lyr=sil_PNGtoNewLayer("testpic1.png",x,y);
  sil_setClickHandler(lyr,totop);
  sil_drawText(background,font,"Click to move to top",x,y+150,0);
  lyr=sil_PNGtoNewLayer("testpic2.png",x+50,y+50);
  sil_setClickHandler(lyr,totop);
  lyr=sil_PNGtoNewLayer("testpic3.png",x+80,y);
  sil_setClickHandler(lyr,totop);

  x+=300;

  lyr=sil_PNGtoNewLayer("testpic4.png",x,y);
  sil_setClickHandler(lyr,tobottom);
  sil_drawText(background,font,"Click to move to bottom",x,y+150,0);
  lyr=sil_PNGtoNewLayer("testpic5.png",x+50,y+50);
  sil_setClickHandler(lyr,tobottom);
  lyr=sil_PNGtoNewLayer("testpic6.png",x+80,y);
  sil_setClickHandler(lyr,tobottom);

  x+=300;

  lyr=sil_PNGtoNewLayer("testpic7.png",x,y);
  sil_setClickHandler(lyr,toswap);
  eight=sil_PNGtoNewLayer("testpic8.png",x+50,y+50);
  sil_setClickHandler(eight,toswap);
  lyr=sil_PNGtoNewLayer("testpic9.png",x+80,y);
  sil_setClickHandler(lyr,toswap);
  sil_drawText(background,font,"Click to swap with 8",x,y+150,0);

  /* --- */

  y+=250;
  x=50;

  lyr=sil_PNGtoNewLayer("testpic10.png",x,y);
  sil_setClickHandler(lyr,toabovebelow);
  sil_drawText(background,font,"r click->under, l click->above 11",x,y+150,0);
  eleven=sil_PNGtoNewLayer("testpic11.png",x+50,y+50);
  sil_setClickHandler(eleven,toabovebelow);
  lyr=sil_PNGtoNewLayer("testpic12.png",x+80,y);
  sil_setClickHandler(lyr,toabovebelow);

  x+=450;

  source=sil_PNGtoNewLayer("testpic1.png",x,y);
  sil_setClickHandler(source,reverse);
  sil_drawText(background,font,"  Source    Copy    Instance",x,y+100,0);
  sil_drawText(background,font,"click to reverse color",x,y+150,0);
  copy=sil_addCopy(source,x+110,y);
  sil_setClickHandler(copy,reverse);
  instance=sil_addInstance(source,x+220,y);
  sil_setClickHandler(instance,reverse);


  sil_updateDisplay();

  sil_mainLoop();

  sil_destroyFont(font);
  sil_destroySIL();


  return 0;
}
