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


  sil_initSIL(1000,1040,"Testing SIL drawing primitives",hInstance);
  sil_setLog(NULL,LOG_INFO|LOG_DEBUG|LOG_VERBOSE);


  font=sil_loadFont("architectsdaughter_thickoutline_24px.fnt");
  background=sil_addLayer(1000,1040,0,0,0);
  sil_paintLayer(background,30,60,120,255);
  top=sil_addLayer(1000,1040,0,0,0);


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


  if (onlyDrawLine) {
    sil_setZoom(30);
  } else {
    sil_setZoom(8);
  }
  sil_setForegroundColor(SILCOLOR_LIME,255);
  sil_drawLine(top,1,50,30,40);
  sil_drawText(top,font,"single line no AA (x8)",10,420,SILTXT_KEEPCOLOR);


  sil_drawLineAA(top,1,50+20,30,40+20);
  sil_drawText(top,font,"single line with AA (x8)",10,580,SILTXT_KEEPCOLOR);

  sil_setDrawWidth(5);
  sil_drawLine(top,1+40,50,30+40,40);
  sil_drawText(top,font,"linewidth=5 no AA (x8)",10+300,420,SILTXT_KEEPCOLOR);

  sil_drawLineAA(top,1+40,50+20,30+40,40+20);
  sil_drawText(top,font,"linewidth=5 with AA (x8)",10+300,580,SILTXT_KEEPCOLOR);
  sil_setZoom(1);

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

  /* variation of line directions */


  sil_setDrawWidth(1);
  /* green, over X increasing x and y */
  sil_setForegroundColor(0,255,0,255);
  sil_drawLine(top,30,700,30+80,700+50);
  sil_putPixelLayer(top,30,699,255,255,255,255);
  sil_putPixelLayer(top,30+80,700+49,0,0,0,255);

  /* light green, over X decreasing x and y */
  sil_setForegroundColor(150,255,150,255);
  sil_drawLine(top,30+90,700+50,40,700);
  sil_putPixelLayer(top,30+90,699+50,255,255,255,255);
  sil_putPixelLayer(top,40,699,0,0,0,255);

  /* red, over Y increasing x and y */
  sil_setForegroundColor(255,0,0,255);
  sil_drawLine(top,10+10,700,10+10+50,700+70);
  sil_putPixelLayer(top,10+10,699,255,255,255,255);
  sil_putPixelLayer(top,10+10+50,699+70,0,0,0,255);

  /* light red, over Y decreasing x and y */
  sil_setForegroundColor(255,150,150,255);
  sil_drawLine(top,10+50,700+70,10,700);
  sil_putPixelLayer(top,10+50,699+70,255,255,255,255);
  sil_putPixelLayer(top,10,699,0,0,0,255);


  /* blue, over X increasing x and decreasing y */
  sil_setForegroundColor(0,0,255,255);
  sil_drawLine(top,10,700+60,10+80,700+10);
  sil_putPixelLayer(top,10,699+60,255,255,255,255);
  sil_putPixelLayer(top,10+80,699+10,0,0,0,255);

  /* light blue, over X decreasing x and increasing y */
  sil_setForegroundColor(150,150,255,255);
  sil_drawLine(top,10+90,700+10,20,700+60);
  sil_putPixelLayer(top,10+90,699+10,255,255,255,255);
  sil_putPixelLayer(top,20,699+60,0,0,0,255);

  /* purple, over Y increasing x and decreasing y */
  sil_setForegroundColor(255,0,255,255);
  sil_drawLine(top,10,700+70,10+40,700+10);
  sil_putPixelLayer(top,10,699+70,255,255,255,255);
  sil_putPixelLayer(top,10+40,699+10,0,0,0,255);

  /* light purple, over Y decreasing x and increasing y */
  sil_setForegroundColor(255,150,255,255);
  sil_drawLine(top,10+40,690,10,690+60);
  sil_putPixelLayer(top,10+40,689,255,255,255,255);
  sil_putPixelLayer(top,10,689+60,0,0,0,255);

  /* purple horizontal and vertical lines */
  sil_setForegroundColor(255,0,255,255);
  sil_drawLine(top,10,45+800,100,45+800);
  sil_putPixelLayer(top,10,45+799,255,255,255,255);
  sil_putPixelLayer(top,100,45+799,0,0,0,255);

  sil_drawLine(top,55,800,55,90+800);
  sil_putPixelLayer(top,56,800,255,255,255,255);
  sil_putPixelLayer(top,56,90+800,0,0,0,255);
 
  /* yellow 45 degree lines */
  sil_setForegroundColor(0,255,255,255);
  sil_drawLine(top,10,800,100,890);
  sil_putPixelLayer(top,10,799,255,255,255,255);
  sil_putPixelLayer(top,100,889,0,0,0,255);


  sil_setForegroundColor(0,255,255,255);
  sil_drawLineAA(top,400,800,450,800);
  sil_drawLineAA(top,400,800,350,800);
  sil_drawLineAA(top,400,800,400,850);
  sil_drawLineAA(top,400,800,400,750);
  sil_drawText(top,font,"Non-AA lines",10,900,SILTXT_KEEPCOLOR);
   
 /*

          |
       II | I
      ----+----
      III | IV
          |
 */

  sil_setForegroundColor(255,0,0,255);
  log_info("Red, quadrant I - 2x1y");
  sil_drawLineAA(top,400+40,800-5,450+40,775-5);
  sil_drawLineAA(top,400+50,800-5,450+50,785-5);
  sil_setForegroundColor(200,0,0,255);
  log_info("Red, quadrant I - 1x1y");
  sil_drawLineAA(top,400+30,800-5,450+30,750-5);
  sil_setForegroundColor(150,0,0,255);
  log_info("Red, quadrant I - 1x2y");
  sil_drawLineAA(top,400+20,800-5,425+20,750-5);
  sil_drawLineAA(top,400+10,800-5,415+10,750-5);

  sil_setForegroundColor(0,255,0,255);
  log_info("Green, quadrant II - 2x1y");
  sil_drawLineAA(top,400-30,800-5,350-30,775-5);
  sil_setForegroundColor(0,200,0,255);
  log_info("Green, quadrant II - 1x1y");
  sil_drawLineAA(top,400-20,800-5,350-20,750-5);
  sil_setForegroundColor(0,150,0,255);
  log_info("Green, quadrant II - 1x2y");
  sil_drawLineAA(top,400-10,800-5,375-10,750-5);

  sil_setForegroundColor(255,255,0,255);
  log_info("yellow, quadrant III - 2x1y");
  sil_drawLineAA(top,400-30,800+5,350-30,825+5);
  sil_setForegroundColor(200,200,0,255);
  log_info("yellow, quadrant III - 1x1y");
  sil_drawLineAA(top,400-20,800+5,350-20,850+5);
  sil_setForegroundColor(150,150,0,255);
  log_info("yellow, quadrant III - 1x2y");
  sil_drawLineAA(top,400-10,800+5,375-10,850+5);

  sil_setForegroundColor(0,0,255,255);
  log_info("blue, quadrant IV - 2x1y");
  sil_drawLineAA(top,400+30,800+5,450+30,825+5);
  sil_setForegroundColor(0,0,200,255);
  log_info("blue, quadrant IV - 1x1y");
  sil_drawLineAA(top,400+20,800+5,450+20,850+5);
  sil_setForegroundColor(0,0,150,255);
  log_info("blue, quadrant IV - 1x2y");
  sil_drawLineAA(top,400+10,800+5,425+10,850+5);

  sil_drawText(top,font,"AA lines",10+300,900,SILTXT_KEEPCOLOR);


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
