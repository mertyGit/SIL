#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef SIL_W32
  #include <windows.h>
#endif
#include "sil.h"
#include "log.h"


static SILLYR *fonttest,*drawing,*animation, *foreground,*background,*one,*two,*three,*test,*mirror, *ontop,*both,*bothnoblend,*rotatetest;
static UINT lw=1;
static SILGROUP *grouptest;

void drawlines() {
  sil_paintLayer(drawing,30,30,30,255);
  sil_borderFilter(drawing);

  sil_setForegroundColor(0,0,255,255);
  sil_drawLine(drawing,20,20,180,180);
  sil_drawLine(drawing,20,20,180,20);
  sil_setForegroundColor(255,255,255,255);
  sil_drawPixel(drawing,20,20);

  sil_setForegroundColor(0,255,0,255);
  sil_drawLine(drawing,180,20,20,180);
  sil_drawLine(drawing,180,20,180,180);
  sil_setForegroundColor(255,255,255,255);
  sil_drawPixel(drawing,180,20);

  sil_setForegroundColor(255,0,0,255);
  sil_drawLine(drawing,150,20,20,180);
  sil_drawLine(drawing,20,20,150,180);
  sil_setForegroundColor(255,255,255,255);
  sil_drawPixel(drawing,150,20);
  sil_drawPixel(drawing,20,20);

  sil_setForegroundColor(255,255,0,255);
  sil_drawLine(drawing,180,50,20,180);
  sil_drawLine(drawing,180,150,20,20);
  sil_setForegroundColor(255,255,255,255);
  sil_drawPixel(drawing,180,50);
  sil_drawPixel(drawing,180,150);

  sil_setForegroundColor(0,255,255,255);
  sil_drawLine(drawing,20,90,180,70);
  sil_drawLine(drawing,20,70,180,90);
  sil_setForegroundColor(255,255,255,255);
  sil_drawPixel(drawing,20,70);
  sil_drawPixel(drawing,20,90);

  sil_setForegroundColor(255,0,255,255);
  sil_drawLine(drawing,70,20,90,180);
  sil_drawLine(drawing,90,20,70,180);
  sil_setForegroundColor(255,255,255,255);
  sil_drawPixel(drawing,70,20);
  sil_drawPixel(drawing,90,20);

  sil_setForegroundColor(0,0,255,255);
  sil_drawLineAA(drawing,210+20,20,210+180,180);
  sil_drawLineAA(drawing,210+20,20,210+180,20);
  sil_setForegroundColor(255,255,255,255);
  sil_drawPixel(drawing,210+20,20);

  sil_setForegroundColor(0,255,0,255);
  sil_drawLineAA(drawing,210+180,20,210+20,180);
  sil_drawLineAA(drawing,210+180,20,210+180,180);
  sil_setForegroundColor(255,255,255,255);
  sil_drawPixel(drawing,210+180,20);

  sil_setForegroundColor(255,0,0,255);
  sil_drawLineAA(drawing,210+150,20,210+20,180);
  sil_drawLineAA(drawing,210+20,20,210+150,180);
  sil_setForegroundColor(255,255,255,255);
  sil_drawPixel(drawing,210+150,20);
  sil_drawPixel(drawing,210+20,20);

  sil_setForegroundColor(255,255,0,255);
  sil_drawLineAA(drawing,210+180,50,210+20,180);
  sil_drawLineAA(drawing,210+180,150,210+20,20);
  sil_setForegroundColor(255,255,255,255);
  sil_drawPixel(drawing,210+180,50);
  sil_drawPixel(drawing,210+180,150);
  
  sil_setForegroundColor(0,255,255,255);
  sil_drawLineAA(drawing,210+20,90,210+180,70);
  sil_drawLineAA(drawing,210+20,70,210+180,90);
  sil_setForegroundColor(255,255,255,255);
  sil_drawPixel(drawing,20,70);
  sil_drawPixel(drawing,20,90);

  sil_setForegroundColor(255,0,255,255);
  sil_drawLineAA(drawing,210+70,20,210+90,180);
  sil_drawLineAA(drawing,210+90,20,210+70,180);
  sil_setForegroundColor(255,255,255,255);
  sil_drawPixel(drawing,210+70,20);
  sil_drawPixel(drawing,210+90,20);

}

UINT keyhandler(SILEVENT *event) {
  SILLYR *lyr;
  switch(event->type) {
    case SILDISP_KEY_DOWN:
      switch(event->key) {
        case SILKY_ESC:
          sil_quitLoop();
          break;
        case SILKY_G:
          sil_showGroup(grouptest);
          return 1;
          break;
        case SILKY_H:
          sil_hideGroup(grouptest);
          return 1;
          break;
        case SILKY_UP:
          sil_setDrawWidth(++lw);
          drawlines();
          return 1;
          break;
        case SILKY_DOWN:
          if (lw>1) {
            sil_setDrawWidth(--lw);
            drawlines();
            return 1;
          }
          break;
      }
      break;
    case SILDISP_KEY_UP:
      if  (SILKY_PRINTSCREEN==event->key) {
        log_info("saving display to 'displaydump.png'");
        sil_saveDisplay("displaydump.png",1000,1000,0,0);
#ifdef SIL_W32
        log_info("saving complete screen to 'screendump.png'");
        lyr=sil_screenCapture();
        if (lyr) {
          sil_saveLayer(lyr,"screendump.png");
          sil_destroyLayer(lyr);
        }
#endif
        
      }
      break;
  }
  return 0;
}

UINT togglevisible(SILEVENT *event) {
  if (sil_checkFlags(event->layer,SILFLAG_INVISIBLE))
    sil_clearFlags(event->layer,SILFLAG_INVISIBLE);
  else 
    sil_setFlags(event->layer,SILFLAG_INVISIBLE);
  return 1;
}

UINT flipit(SILEVENT *event) {
  if ((event->type==SILDISP_MOUSE_ENTER)||(event->type==SILDISP_MOUSE_LEFT)) {
    sil_flipxFilter(event->layer);
    return 1;
  } 
  return 0;
}

UINT showme(SILEVENT *event) {
  if (event->type==SILDISP_MOUSE_LEFT) {
    sil_setView(event->layer,20,20,60,60);
    return 1;
  }
  if (event->type==SILDISP_MOUSE_ENTER) {
    sil_resetView(event->layer);
    return 1;
  }
  return 0;
}


UINT tick(SILEVENT *event) {
  sil_nextSprite(animation);
  return 1;
}

UINT drag(SILEVENT *event) {
  return 1;
}

UINT rotateit(SILEVENT *event) {
  if (event->type==SILDISP_MOUSE_DOWN) {
    switch(event->val) {
      case 1:
        sil_rotateColorFilter(event->layer);
        break;
      case 3:
        sil_reverseColorFilter(event->layer);
        break;
    }
    return 1;
  } 
  return 0;
}

UINT rotateme(SILEVENT *event) {
  if (event->type==SILDISP_MOUSE_DOWN) {
    if (SIL_BTN_LEFT==event->val) {
      sil_rotateLayer(rotatetest,-30);
      } else {
      sil_rotateLayer(rotatetest,30);
    }
  }
  return 1;
}

#ifdef SIL_W32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#else
int main() {
  void *hInstance=NULL;
#endif

  unsigned char r,g,b;
  SILEVENT *se=NULL;
  UINT err;
  BYTE move=0;
  BYTE update=0;
  SILFONT *font,*font2;


  printf("sil_init...\n");
  sil_initSIL(1000,1000,"Testing SIL DISPLAY III",hInstance);
  sil_setLog(NULL,LOG_INFO|LOG_DEBUG|LOG_VERBOSE);

  printf("sil_createGroup...\n");
  grouptest=sil_createGroup();
  if (NULL==grouptest) {
    return 1;
  }

  printf("sil_addLayer background...\n");
  background=sil_addLayer(0,0,1000,1000,SILTYPE_565RGB);
  if (NULL==background) {
    return 2;
  }


  printf("setting catch all for key events...\n");
  sil_setKeyHandler(background,0,0,0,keyhandler); 


  printf("sil_PNGintoLayer background...\n");
  err=sil_PNGintoLayer(background,"testimage.png",0,0);
  if (err) {
    return 3;
  }

  printf("loading dancing banana...\n");
  animation=sil_PNGtoNewLayer("dancingbanana.png",320,80);
  if (NULL==animation) {
    return 3;
  }
  sil_initSpriteSheet(animation,8,2);


  printf("sil_PNGtoNewLayer one...\n");
  one=sil_PNGtoNewLayer("testpic2.png",500,500);
  if (NULL==one) {
    return 4;
  }
  sil_setKeyHandler(one,SILKY_2,0,SILKT_SINGLE,togglevisible);
  sil_addLayerGroup(grouptest,one);
  

  printf("darken on one\n");
  sil_brightnessFilter(one,-50);

  printf("sil_PNGtoNewLayer two...\n");
  two=sil_PNGtoNewLayer("testpic3.png",550,550);
  if (NULL==two) {
    return 5;
  }
  sil_setKeyHandler(two,SILKY_3,0,SILKT_SINGLE,togglevisible);
  sil_addLayerGroup(grouptest,two);

  printf("lighten on two\n");
  sil_brightnessFilter(two,50);

  printf("sil_PNGtoNewLayer three..\n");
  three=sil_PNGtoNewLayer("testpic4.png",600,600);
  if (NULL==three) {
    return 6;
  }
  sil_blurFilter(three);
  sil_setKeyHandler(three,SILKY_4,0,SILKT_SINGLE,togglevisible);
  sil_addLayerGroup(grouptest,three);

  printf("sil_PNGtoNewLayer test..\n");
  test=sil_PNGtoNewLayer("testpic5.png",500,350);
  if (NULL==test) {
    return 7;
  }
  sil_setKeyHandler(test,SILKY_5,0,SILKT_SINGLE,togglevisible);
  sil_setHoverHandler(test,flipit);
  sil_setClickHandler(test,rotateit);
  sil_setFlags(test,SILFLAG_MOUSEALLPIX);
  sil_addLayerGroup(grouptest,test);

  printf("sil_setViewLayer test..\n");
  sil_setView(test,1,1,98,98);


  printf("sil_addLayer foreground...\n");
  foreground=sil_addLayer(40,40,500,500,SILTYPE_ARGB);
  if (NULL==foreground) {
    return 8;
  }
  sil_setKeyHandler(foreground,SILKY_1,0,SILKT_SINGLE,togglevisible);
  sil_setDragHandler(foreground,drag);

  printf("sil_PNGtoLayer foreground...\n");
  err=sil_PNGintoLayer(foreground,"testpic1.png",0,0);
  if (NULL==foreground) {
    return 9;
  }

  printf("crop AlphaFilter foreground...\n");
  sil_cropAlphaFilter(foreground);

  printf("set Alpha foreground...\n");
  sil_setAlphaLayer(foreground,0.9);


  printf("sil_addLayer ontop...\n");
  ontop=sil_addLayer(50,450,256,256,SILTYPE_ARGB);
  if (NULL==ontop) {
    return 10;
  }
  sil_setKeyHandler(ontop,SILKY_6,0,SILKT_SINGLE,togglevisible);

  printf("sil_putPixelLayer ontop...\n");
  for (int x=0;x<256;x++) {
    for (int y=0;y<256;y++) {
      g=(x*y)%255;
      b=x%255;
      r=y%255;
      sil_putPixelLayer(ontop,x,y,r,g,b,230);
    }
  }

  printf("sil_addLayer both...\n");
  both=sil_addLayer(550,50,200,200,SILTYPE_ARGB);
  if (NULL==both) {
    return 11;
  }
  sil_borderFilter(both);
  sil_setKeyHandler(both,SILKY_7,0,SILKT_SINGLE,togglevisible);

  printf("sil_PNGtoLayer both left ...\n");
  err=sil_PNGintoLayer(both,"testpic6.png",0,0);
  if (err) {
    printf("ERROR: %d\n",err);
    return 12;
  }
  printf("sil_PNGtoLayer both right ...\n");
  err=sil_PNGintoLayer(both,"testpic7.png",30,50);
  if (err) {
    printf("ERROR: %d\n",err);
    return 13;
  }

  printf("sil_addLayer bothnoblend...\n");
  bothnoblend=sil_addLayer(750,50,200,200,SILTYPE_ARGB);
  if (NULL==bothnoblend) {
    return 14;
  }
  sil_setFlags(bothnoblend,SILFLAG_NOBLEND);
  sil_borderFilter(bothnoblend);
  sil_setKeyHandler(bothnoblend,SILKY_9,0,SILKT_SINGLE,togglevisible);
  sil_addLayerGroup(grouptest,bothnoblend);

  printf("sil_PNGtoLayer bothnoblend left ...\n");
  err=sil_PNGintoLayer(bothnoblend,"testpic8.png",0,0);
  if (err) {
    printf("ERROR: %d\n",err);
    return 15;
  }
  printf("sil_PNGtoLayer bothnoblend right ...\n");
  err=sil_PNGintoLayer(bothnoblend,"testpic9.png",30,50);
  if (err) {
    printf("ERROR: %d\n",err);
    return 16;
  }

  printf("sil_PNGtoNewLayer 10..\n");
  test=sil_PNGtoNewLayer("testpic10.png",700,300);
  if (NULL==test) {
    return 17;
  }
  sil_setView(test,20,20,60,60);
  sil_setHoverHandler(test,showme);
  sil_addLayerGroup(grouptest,test);

  printf("mirroring layer 10");
  mirror=sil_addInstance(test,700,400);
  sil_resetView(mirror);




  printf("sil_PNGtoNewLayer 11..\n");
  test=sil_PNGtoNewLayer("testpic11.png",800,300);
  if (NULL==test) {
    return 17;
  }
  sil_setView(test,20,20,60,60);
  sil_setFlags(test,SILFLAG_VIEWPOSSTAY);
  sil_setHoverHandler(test,showme);
  sil_addLayerGroup(grouptest,test);


  printf("sil_rescale test width 12");
  test=sil_PNGtoNewLayer("testpic12.png",750,510);
  if (NULL==test) {
    return 17;
  }

  test=sil_PNGtoNewLayer("testpic12.png",650,510);
  if (NULL==test) {
    return 18;
  }
  sil_rescale(test,60,60);

  test=sil_PNGtoNewLayer("testpic12.png",860,510);
  if (NULL==test) {
    return 19;
  }
  sil_rescale(test,135,90);







  printf("sil_addLayer fonttest...\n");
  fonttest=sil_addLayer(50,710,500,200,SILTYPE_ARGB);
  if (NULL==fonttest) {
    return 20;
  }
  printf("painting fonttest ...\n");
  sil_paintLayer(fonttest,180,40,40,255);
  sil_borderFilter(fonttest);
  sil_setKeyHandler(fonttest,SILKY_0,0,SILKT_SINGLE,togglevisible);
  

  printf("sil_loadFont...\n");
  font=sil_loadFont("architectsdaughter_thickoutline_20px.fnt");
  if (NULL==font) {
    return 21;
  }
  font2=sil_loadFont("architectsdaughter_24px.fnt");
  if (NULL==font2) {
    return 22;
  }

  printf("sil_drawText with kerning and original color ...\n");
  sil_drawText(fonttest,font,"The quick brown fox jumps over the lazy dog",5,5,SILTXT_KEEPCOLOR);

  sil_setAlphaFont(font,0.5);
  printf("...and without kerning but with punchout...\n");
  sil_drawText(fonttest,font,"The quick brown fox jumps over the lazy dog",5,5+(font->base),SILTXT_NOKERNING|SILTXT_PUNCHOUT);

  printf("...and with alpha set, no blending ..\n");
  sil_drawText(fonttest,font,"The quick brown fox jumps over the lazy dog",5,5+2*(font->base),0);
  sil_setAlphaFont(font,1.0);

  printf("...and now with monospaced...\n");
  sil_drawText(fonttest,font,"The Monospace 123456790",5,5+3*(font->base),SILTXT_MONOSPACE);

  sil_setForegroundColor(0,0,255,128);
  printf("...and now with blue color...\n");
  sil_drawText(fonttest,font2,"Lets turn the color halfway to BLUE",5,5+4*(font2->base),0);
  sil_setForegroundColor(255,255,0,255);
  printf("...and now with yellow color & without Kerning...\n");
  sil_drawText(fonttest,font2,"Lets turn the color to YELLOW",5,5+5*(font2->base),0);
  sil_drawText(fonttest,font,"Lets turn the color to YELLOW",5,5+6*(font2->base),0);

  printf("sil_addLayer drawing...\n");

  printf("sil_addLayer drawing...\n");
  drawing=sil_addLayer(600,710,400,200,SILTYPE_ARGB);
  if (NULL==drawing) {
    return 23;
  }
  drawlines();

  printf("testing rotation with testpic13 ..\n");
  rotatetest=sil_addLayer(260,250,100,100,SILTYPE_ARGB);
  if (NULL==rotatetest) {
    return 24;
  }
  sil_PNGintoLayer(rotatetest,"testpic13.png",0,0);
  sil_setClickHandler(rotatetest,rotateme);




  printf("sil_setTimerHandler...\n");
  sil_setTimerHandler(tick);

  printf("sil_setTimeval...\n");
  sil_setTimeval(100); /* every 100 msec = every 1/10 second */


  printf("sil_updateDisplay...\n");
  sil_updateDisplay();

  printf("sil_mainLoop...\n");
  sil_mainLoop();

  printf("sil_destroyGroup...\n");
  sil_destroyGroup(grouptest);

  printf("sil_destroyFont...\n");
  sil_destroyFont(font);

  printf("sil_destroySIL...\n");
  sil_destroySIL();

  printf("Bye now...\n");

  return 0;
}
