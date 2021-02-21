#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef SIL_W32
  #include <windows.h>
#endif
#include "sil.h"
#include "log.h"

static SILLYR *fonttest,*foreground,*background,*one,*two,*three,*test,*ontop,*both,*bothnoblend;

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

UINT togglevisible(SILEVENT *event) {
  if (sil_checkFlags(event->layer,SILFLAG_INVISIBLE))
    sil_clearFlags(event->layer,SILFLAG_INVISIBLE);
  else 
    sil_setFlags(event->layer,SILFLAG_INVISIBLE);
  return 1;
}

UINT flipit(SILEVENT *event) {
  if ((event->type==SILDISP_MOUSE_ENTER)||(event->type==SILDISP_MOUSE_LEFT)) {
    sil_applyFilterLayer(event->layer,SILFLTR_FLIPX);
    return 1;
  } 
  return 0;
}

UINT tick(SILEVENT *event) {
  //log_verbose("GOT tick:%d",event->val);
  return 0;
}

UINT drag(SILEVENT *event) {
  return 1;
}

UINT rotateit(SILEVENT *event) {
  if (event->type==SILDISP_MOUSE_DOWN) {
    switch(event->val) {
      case 1:
        sil_applyFilterLayer(event->layer,SILFLTR_ROTATECOLOR);
        break;
      case 3:
        sil_applyFilterLayer(event->layer,SILFLTR_REVERSECOLOR);
        break;
    }
    return 1;
  } 
  return 0;
}

#ifdef SIL_W32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#else
int main() {
#endif

  unsigned char r,g,b;
  SILEVENT *se=NULL;
  UINT err;
  BYTE move=0;
  BYTE update=0;
  SILFONT *font;


  printf("sil_init...\n");
  #ifdef SIL_W32
    sil_initSIL(1000,1000,"Testing SIL DISPLAY III",hInstance);
  #else
    sil_initSIL(1000,1000,"Testing SIL DISPLAY III",NULL);
  #endif
  sil_setLog(NULL,LOG_INFO|LOG_DEBUG|LOG_VERBOSE);


  printf("sil_addLayer background...\n");
  background=sil_addLayer(1000,1000,0,0,0);
  if (NULL==background) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 2;
  }

  printf("setting catch all for key events...\n");
  sil_setKeyHandler(background,0,0,0,keyhandler); 


  printf("sil_PNGintoLayer background...\n");
  err=sil_PNGintoLayer(background,"testimage.png",0,0);
  if (err) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 3;
  }

  printf("sil_PNGtoNewLayer one...\n");
  one=sil_PNGtoNewLayer("testpic2.png",500,500);
  if (NULL==one) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 4;
  }
  sil_setKeyHandler(one,SILKY_2,0,SILKT_SINGLE,togglevisible);

  printf("sil_applyFilterLayer darken on one\n");
  sil_applyFilterLayer(one,SILFLTR_DARKEN);

  printf("sil_PNGtoNewLayer two...\n");
  two=sil_PNGtoNewLayer("testpic3.png",550,550);
  if (NULL==two) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 5;
  }
  sil_setKeyHandler(two,SILKY_3,0,SILKT_SINGLE,togglevisible);

  printf("sil_applyFilterLayer lighten on two\n");
  sil_applyFilterLayer(two,SILFLTR_LIGHTEN);

  printf("sil_PNGtoNewLayer three..\n");
  three=sil_PNGtoNewLayer("testpic4.png",600,600);
  if (NULL==three) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 6;
  }
  sil_applyFilterLayer(three,SILFLTR_BLUR);
  sil_setKeyHandler(three,SILKY_4,0,SILKT_SINGLE,togglevisible);

  printf("sil_PNGtoNewLayer test..\n");
  test=sil_PNGtoNewLayer("testpic5.png",500,350);
  if (NULL==test) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 7;
  }
  sil_setKeyHandler(test,SILKY_5,0,SILKT_SINGLE,togglevisible);
  sil_setHoverHandler(test,flipit);
  sil_setClickHandler(test,rotateit);

  printf("sil_setViewLayer test..\n");
  sil_setView(test,1,1,test->fb->width-1,test->fb->height-1);


  printf("sil_addLayer foreground...\n");
  foreground=sil_addLayer(500,500,40,40,0);
  if (NULL==foreground) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 8;
  }
  sil_setKeyHandler(foreground,SILKY_1,0,SILKT_SINGLE,togglevisible);
  sil_setDragHandler(foreground,drag);

  printf("sil_PNGtoLayer foreground...\n");
  err=sil_PNGintoLayer(foreground,"testpic1.png",0,0);
  if (NULL==foreground) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 9;
  }

  printf("sil_applyFilter AUTOCROPALPHA foreground...\n");
  sil_applyFilterLayer(foreground,SILFLTR_AUTOCROPALPHA);

  printf("sil_setAlphaLayer foreground...\n");
  sil_setAlphaLayer(foreground,0.9);


  printf("sil_addLayer ontop...\n");
  ontop=sil_addLayer(256,256,50,450,0);
  if (NULL==ontop) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
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
  both=sil_addLayer(200,200,550,50,0);
  if (NULL==both) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 11;
  }
  sil_applyFilterLayer(both,SILFLTR_BORDER);
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
  bothnoblend=sil_addLayer(200,200,750,50,0);
  if (NULL==bothnoblend) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 14;
  }
  sil_setFlags(bothnoblend,SILFLAG_NOBLEND);
  sil_applyFilterLayer(bothnoblend,SILFLTR_BORDER);
  sil_setKeyHandler(bothnoblend,SILKY_9,0,SILKT_SINGLE,togglevisible);

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

  printf("sil_addLayer fonttest...\n");
  fonttest=sil_addLayer(500,200,50,710,0);
  if (NULL==fonttest) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 17;
  }
  printf("painting fonttest ...\n");
  sil_paintLayer(fonttest,180,40,40,255);
  sil_applyFilterLayer(fonttest,SILFLTR_BORDER);
  sil_setKeyHandler(fonttest,SILKY_0,0,SILKT_SINGLE,togglevisible);
  

  printf("sil_loadFont...\n");
  font=sil_loadFont("bahnschrift.fnt");
  if (NULL==font) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 18;
  }

  printf("sil_drawTextLayer with kerning and original color ...\n");
  sil_drawTextLayer(fonttest,font,"The quick brown fox jumps over the lazy dog",5,5,SILTXT_KEEPCOLOR);

  sil_setAlphaFont(font,0.5);
  printf("...and without kerning but with blending ...\n");
  sil_drawTextLayer(fonttest,font,"The quick brown fox jumps over the lazy dog",5,5+(font->base),SILTXT_NOKERNING|SILTXT_BLENDLAYER);

  printf("...and with alpha set, no blending ..\n");
  sil_drawTextLayer(fonttest,font,"The quick brown fox jumps over the lazy dog",5,5+2*(font->base),0);
  sil_setAlphaFont(font,1.0);

  printf("...and now with monospaced...\n");
  sil_drawTextLayer(fonttest,font,"The Monospace 123456790",5,5+3*(font->base),SILTXT_MONOSPACE);

  sil_setForegroundColor(0,0,255,150);
  printf("...and now with blue color...\n");
  sil_drawTextLayer(fonttest,font,"Lets turn BLUE",5,5+4*(font->base),0);

  printf("sil_setTimerHandler...\n");
  sil_setTimerHandler(tick);

  printf("sil_setTimeval...\n");
  sil_setTimeval(1000); /* every 1000 msec = every second */


  printf("sil_updateDisplay...\n");
  sil_updateDisplay();

  printf("sil_mainLoop...\n");
  sil_mainLoop();


  printf("sil_destroyFont...\n");
  sil_destroyFont(font);

  printf("sil_destroySIL...\n");
  sil_destroySIL();

  printf("Bye now...\n");

  return 0;
}
