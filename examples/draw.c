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

  BYTE onlyDrawLine=0;


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


  if (onlyDrawLine) {
    sil_setZoom(30);
  } else {
    sil_setZoom(8);
  }
  sil_setForegroundColor(0,255,0,255);
  sil_drawLine(top,1,50,30,40);
  sil_drawTextLayer(top,font,"single line no AA (x8)",10,420,SILTXT_KEEPCOLOR);

  if (0==onlyDrawLine) {

    sil_drawLineAA(top,1,50+20,30,40+20);
    sil_drawTextLayer(top,font,"single line with AA (x8)",10,580,SILTXT_KEEPCOLOR);

    sil_setDrawWidth(5);
    sil_drawLine(top,1+40,50,30+40,40);
    sil_drawTextLayer(top,font,"linewidth=5 no AA (x8)",10+300,420,SILTXT_KEEPCOLOR);

    sil_drawLineAA(top,1+40,50+20,30+40,40+20);
    sil_drawTextLayer(top,font,"linewidth=5 with AA (x8)",10+300,580,SILTXT_KEEPCOLOR);
    sil_setZoom(1);

    sil_setDrawWidth(1);
    sil_drawCircle(top,80,680,50);
    sil_drawTextLayer(top,font,"single circle",10,730,SILTXT_KEEPCOLOR);

    sil_setDrawWidth(5);
    sil_drawCircle(top,300+80,680,50);
    sil_drawTextLayer(top,font,"linewidth=5",300+10,730,SILTXT_KEEPCOLOR);


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

    sil_setDrawWidth(1);

    /* variation of line directions */


    /* green, over X increasing x and y */
    sil_setForegroundColor(0,255,0,255);
    sil_drawLine(top,30,800,30+80,800+50);
    sil_putPixelLayer(top,30,799,255,255,255,255);
    sil_putPixelLayer(top,30+80,800+49,0,0,0,255);

    /* light green, over X decreasing x and y */
    sil_setForegroundColor(150,255,150,255);
    sil_drawLine(top,30+90,800+50,40,800);
    sil_putPixelLayer(top,30+90,799+50,255,255,255,255);
    sil_putPixelLayer(top,40,799,0,0,0,255);

    /* red, over Y increasing x and y */
    sil_setForegroundColor(255,0,0,255);
    sil_drawLine(top,10+10,800,10+10+50,800+70);
    sil_putPixelLayer(top,10+10,799,255,255,255,255);
    sil_putPixelLayer(top,10+10+50,799+70,0,0,0,255);

    /* light red, over Y decreasing x and y */
    sil_setForegroundColor(255,150,150,255);
    sil_drawLine(top,10+50,800+70,10,800);
    sil_putPixelLayer(top,10+50,799+70,255,255,255,255);
    sil_putPixelLayer(top,10,799,0,0,0,255);


    /* blue, over X increasing x and decreasing y */
    sil_setForegroundColor(0,0,255,255);
    sil_drawLine(top,10,800+60,10+80,800+10);
    sil_putPixelLayer(top,10,799+60,255,255,255,255);
    sil_putPixelLayer(top,10+80,799+10,0,0,0,255);

    /* light blue, over X decreasing x and increasing y */
    sil_setForegroundColor(150,150,255,255);
    sil_drawLine(top,10+90,800+10,20,800+60);
    sil_putPixelLayer(top,10+90,799+10,255,255,255,255);
    sil_putPixelLayer(top,20,799+60,0,0,0,255);

    /* purple, over Y increasing x and decreasing y */
    sil_setForegroundColor(255,0,255,255);
    sil_drawLine(top,10,800+70,10+40,800+10);
    sil_putPixelLayer(top,10,799+70,255,255,255,255);
    sil_putPixelLayer(top,10+40,799+10,0,0,0,255);

    /* light purple, over Y decreasing x and increasing y */
    sil_setForegroundColor(255,150,255,255);
    sil_drawLine(top,10+40,790,10,790+60);
    sil_putPixelLayer(top,10+40,789,255,255,255,255);
    sil_putPixelLayer(top,10,789+60,0,0,0,255);

    /* purple horizontal and vertical lines */
    sil_setForegroundColor(255,0,255,255);
    sil_drawLine(top,10,45+900,100,45+900);
    sil_putPixelLayer(top,10,45+899,255,255,255,255);
    sil_putPixelLayer(top,100,45+899,0,0,0,255);

    sil_drawLine(top,55,900,55,90+900);
    sil_putPixelLayer(top,56,900,255,255,255,255);
    sil_putPixelLayer(top,56,90+900,0,0,0,255);
   
    /* yellow 45 degree lines */
    sil_setForegroundColor(0,255,255,255);
    sil_drawLine(top,10,900,100,990);
    sil_putPixelLayer(top,10,899,255,255,255,255);
    sil_putPixelLayer(top,100,989,0,0,0,255);


    sil_setForegroundColor(0,255,255,255);
    sil_drawLineAA(top,400,900,450,900);
    sil_drawLineAA(top,400,900,350,900);
    sil_drawLineAA(top,400,900,400,950);
    sil_drawLineAA(top,400,900,400,850);
     
   /*

          |
       II | I
      ----+----
      III | IV
          |
   */

    sil_setForegroundColor(255,0,0,255);
    log_info("Red, quadrant I - 2x1y");
    sil_drawLineAA(top,400+40,900-5,450+40,875-5);
    sil_drawLineAA(top,400+50,900-5,450+50,885-5);
    sil_setForegroundColor(200,0,0,255);
    log_info("Red, quadrant I - 1x1y");
    sil_drawLineAA(top,400+30,900-5,450+30,850-5);
    sil_setForegroundColor(150,0,0,255);
    log_info("Red, quadrant I - 1x2y");
    sil_drawLineAA(top,400+20,900-5,425+20,850-5);
    sil_drawLineAA(top,400+10,900-5,415+10,850-5);

    sil_setForegroundColor(0,255,0,255);
    log_info("Green, quadrant II - 2x1y");
    sil_drawLineAA(top,400-30,900-5,350-30,875-5);
    sil_setForegroundColor(0,200,0,255);
    log_info("Green, quadrant II - 1x1y");
    sil_drawLineAA(top,400-20,900-5,350-20,850-5);
    sil_setForegroundColor(0,150,0,255);
    log_info("Green, quadrant II - 1x2y");
    sil_drawLineAA(top,400-10,900-5,375-10,850-5);

    sil_setForegroundColor(255,255,0,255);
    log_info("yellow, quadrant III - 2x1y");
    sil_drawLineAA(top,400-30,900+5,350-30,925+5);
    sil_setForegroundColor(200,200,0,255);
    log_info("yellow, quadrant III - 1x1y");
    sil_drawLineAA(top,400-20,900+5,350-20,950+5);
    sil_setForegroundColor(150,150,0,255);
    log_info("yellow, quadrant III - 1x2y");
    sil_drawLineAA(top,400-10,900+5,375-10,950+5);

    sil_setForegroundColor(0,0,255,255);
    log_info("blue, quadrant IV - 2x1y");
    sil_drawLineAA(top,400+30,900+5,450+30,925+5);
    sil_setForegroundColor(0,0,200,255);
    log_info("blue, quadrant IV - 1x1y");
    sil_drawLineAA(top,400+20,900+5,450+20,950+5);
    sil_setForegroundColor(0,0,150,255);
    log_info("blue, quadrant IV - 1x2y");
    sil_drawLineAA(top,400+10,900+5,425+10,950+5);


  }

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
