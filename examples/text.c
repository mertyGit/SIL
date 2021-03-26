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
  SILFONT *ad24,*ad28,*ado24,*ado28;
  SILFONT *ssp16;
  UINT x,y;
  UINT len,height;



  #ifdef SIL_W32
    sil_initSIL(900,400,"Testing SIL font and texthandling",hInstance);
  #else
    sil_initSIL(900,400,"Testing SIL font and texthandling",NULL);
  #endif
  sil_setLog(NULL,LOG_INFO|LOG_DEBUG|LOG_VERBOSE);


  ad24=sil_loadFont("architectsdaughter_24px.fnt");
  ado24=sil_loadFont("architectsdaughter_thickoutline_24px.fnt");
  ad28=sil_loadFont("architectsdaughter_28px.fnt");
  ado28=sil_loadFont("architectsdaughter_thickoutline_28px.fnt");
  ssp16=sil_loadFont("sourcesanspro_16px.fnt");

  background=sil_addLayer(900,400,0,0,0);
  sil_paintLayer(background,SILCOLOR_LIGHT_STEEL_BLUE,255);
  top=sil_addLayer(900,400,0,0,0);

  sil_setForegroundColor(SILCOLOR_YELLOW,255);
  y=10;
  sil_drawText(top,ado28,"Ordinary tekst with outlines in font",10,y,0);

  y+=sil_getHeightFont(ado28);

  sil_drawText(top,ado28,"Ordinary text with original color SILTXT_KEEPCOLOR",10,y,SILTXT_KEEPCOLOR);

  y+=sil_getHeightFont(ado28);

  sil_setForegroundColor(SILCOLOR_BLACK,255);
  sil_drawText(top,ssp16,"Smaller text to test kerning LIKE \"WATER\"",10,y,0);

  y+=sil_getHeightFont(ssp16);

  sil_drawText(top,ssp16,"Smaller text to test kerning LIKE \"WATER\" (SILTXT_NOKERNING)",10,y,SILTXT_NOKERNING);
  sil_setForegroundColor(SILCOLOR_GREEN,255);

  y+=sil_getHeightFont(ssp16);

  sil_drawText(top,ssp16,"Smaller text to test MONOSPACING (SILTXT_MONOSPACE)",10,y,SILTXT_MONOSPACE);

  y+=sil_getHeightFont(ssp16);

  sil_drawText(top,ssp16,"12345678901234567890..............................|",10,y,SILTXT_MONOSPACE);

  y+=sil_getHeightFont(ssp16);

  sil_setBackgroundColor(SILCOLOR_PURPLE,255);
  sil_setDrawWidth(0);
  sil_drawRectangle(top,5,y,600,sil_getHeightFont(ad28)+5);
  sil_setBackgroundColor(SILCOLOR_YELLOW,255);
  sil_drawRectangle(background,5,y,600,sil_getHeightFont(ad28)/2);
  sil_drawText(top,ad28,"Punching out with SILTXT_PUNCHOUT to show background layer",10,y,SILTXT_PUNCHOUT);

  y+=sil_getHeightFont(ad28);

  sil_setForegroundColor(SILCOLOR_MAROON,255);
  sil_drawText(top,ado24,"Lets use maroon as color",10,y,0);

  y+=sil_getHeightFont(ado24);

  sil_setForegroundColor(SILCOLOR_MAROON,64);
  sil_drawText(top,ado24,"Lets use maroon as color, but with alpha=64",10,y,0);

  y+=sil_getHeightFont(ado24);

  sil_drawText(top,ado24,"Lets use maroon as color, but with alpha=64 and SILTXT_KEEPBLACK",10,y,SILTXT_KEEPBLACK);

  y+=sil_getHeightFont(ado24);

  sil_setForegroundColor(SILCOLOR_YELLOW,255);
  sil_drawText(top,ado28,"Testing height and lenght calculation of text",10,y,0);
  sil_setBackgroundColor(SILCOLOR_BLACK,0);

  len=sil_getTextWidth(ado28,"Testing height and lenght calculation of text",0);
  height=sil_getHeightFont(ado28);

  sil_setDrawWidth(3);
  sil_setForegroundColor(SILCOLOR_GREEN,120);
  sil_drawRectangle(top,10,y,len,height);

  y+=sil_getHeightFont(ado28);


  sil_setForegroundColor(SILCOLOR_CHOCOLATE,255);
  sil_drawText(top,ad28,"Testing height and lenght calculation of text",10,y,0);

  len=sil_getTextWidth(ad28,"Testing height and lenght calculation of text",0);
  height=sil_getHeightFont(ad28);

  sil_setForegroundColor(SILCOLOR_DARK_GREEN,120);
  sil_drawRectangle(top,10,y,len,height);

  y+=sil_getHeightFont(ad28);


  sil_updateDisplay();

  do {
    se=sil_getEventDisplay();
    if (se->type==SILDISP_QUIT) break;
    sil_updateDisplay();
  } while (se!=NULL && se->key!=SILKY_ESC);


  sil_destroyFont(ad24);
  sil_destroyFont(ado24);
  sil_destroyFont(ad28);
  sil_destroyFont(ssp16);
  sil_destroySIL();


  return 0;
}
