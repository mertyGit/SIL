#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef SIL_W32
  #include <windows.h>
#endif
#include "sil.h"
#include "log.h"

#ifdef SIL_W32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#else
int main() {
#endif

  unsigned char r,g,b;
  SILEVENT *se=NULL;
  SILLYR *fonttest,*foreground,*background,*one,*two,*three,*test,*ontop,*both,*bothnoblend;
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

  printf("sil_applyFilterLayer darken on one\n");
  sil_applyFilterLayer(one,SILFLTR_DARKEN);

  printf("sil_PNGtoNewLayer two...\n");
  two=sil_PNGtoNewLayer("testpic3.png",550,550);
  if (NULL==two) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 5;
  }

  printf("sil_applyFilterLayer lighten on two\n");
  sil_applyFilterLayer(two,SILFLTR_LIGHTEN);

  printf("sil_PNGtoNewLayer three..\n");
  three=sil_PNGtoNewLayer("testpic4.png",600,600);
  if (NULL==three) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 6;
  }
  sil_applyFilterLayer(three,SILFLTR_BLUR);

  printf("sil_PNGtoNewLayer test..\n");
  test=sil_PNGtoNewLayer("testpic5.png",500,350);
  if (NULL==test) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 7;
  }

  printf("sil_setViewLayer test..\n");
  sil_setViewLayer(test,1,1,test->fb->width-1,test->fb->height-1);


  printf("sil_addLayer foreground...\n");
  foreground=sil_addLayer(500,500,40,40,0);
  if (NULL==foreground) {
    printf("%s\n",sil_err2Txt(sil_getErr()));
    return 8;
  }

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
  sil_setFlagsLayer(bothnoblend,SILFLAG_NOBLEND);
  sil_applyFilterLayer(bothnoblend,SILFLTR_BORDER);

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


  printf("sil_updateDisplay...\n");
  sil_updateDisplay();

  printf("sil_getEventDisplay...\n");
  do {
    se=sil_getEventDisplay(0);
    if (se->type==SILDISP_QUIT) break;
    if ((se!=NULL)&&(se->type!=SILDISP_NOTHING)) {
      printf("Got event type=%d,scancode=%d (%02x),key=%d,modifiers=%d,val=%d,x=%d,y=%d\n",se->type,se->code,se->code,se->key,se->modifiers,se->val,se->x,se->y);
      if (se->type==SILDISP_KEY_DOWN) {
        update=1;
        switch(se->key) {
          case SILKY_LEFT:
            move|=4;
            break;
          case SILKY_RIGHT:
            move|=1;
            break;
          case SILKY_DOWN:
            move|=2;
            break;
          case SILKY_UP:
            move|=8;
            break;
          case SILKY_1:
            if (sil_checkFlagsLayer(background,SILFLAG_INVISIBLE))
              sil_clearFlagsLayer(background,SILFLAG_INVISIBLE);
            else 
              sil_setFlagsLayer(background,SILFLAG_INVISIBLE);
            break;
          case SILKY_2:
            if (sil_checkFlagsLayer(foreground,SILFLAG_INVISIBLE))
              sil_clearFlagsLayer(foreground,SILFLAG_INVISIBLE);
            else 
              sil_setFlagsLayer(foreground,SILFLAG_INVISIBLE);
            break;
          case SILKY_3:
            if (sil_checkFlagsLayer(ontop,SILFLAG_INVISIBLE))
              sil_clearFlagsLayer(ontop,SILFLAG_INVISIBLE);
            else 
              sil_setFlagsLayer(ontop,SILFLAG_INVISIBLE);
            break;
          case SILKY_R:
              sil_applyFilterLayer(foreground,SILFLTR_REVERSECOLOR);
            break;
          case SILKY_X:
            sil_applyFilterLayer(foreground,SILFLTR_FLIPX);
            break;
          case SILKY_Y:
            sil_applyFilterLayer(foreground,SILFLTR_FLIPY);
            break;
          case SILKY_G:
            sil_applyFilterLayer(foreground,SILFLTR_GRAYSCALE);
            break;
        }
      } else {
        if (se->type==SILDISP_KEY_UP) {
        update=1;
          switch(se->key) {
            case SILKY_LEFT:
              move^=4;
              break;
            case SILKY_RIGHT:
              move^=1;
              break;
            case SILKY_DOWN:
              move^=2;
              break;
            case SILKY_UP:
              move^=8;
              break;
          }
        }
      }
    }
    if (move) {
      if (move&1) sil_moveLayer(foreground, 4, 0);
      if (move&2) sil_moveLayer(foreground, 0, 4);
      if (move&4) sil_moveLayer(foreground,-4, 0);
      if (move&8) sil_moveLayer(foreground, 0,-4);
      update=1;
    }
    if (update) sil_updateDisplay();
  } while (se!=NULL && se->key!=SILKY_ESC);


  printf("sil_destroyFont...\n");
  sil_destroyFont(font);

  printf("sil_destroySIL...\n");
  sil_destroySIL();

  printf("Bye now...\n");

  return 0;
}
