/* 

   drawing.c CopyRight 2021 Remco Schellekens, see LICENSE for more details.

   This file contains all functions for handling of drawing on layers (and
   indirectly, framebuffers of those layers )

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include"lodepng.h"
#include"sil.h"
#include"log.h"

/*****************************************************************************
  Program wide foreground & background colors 
  drawing text or lines will use these colors

 *****************************************************************************/


typedef struct _GCOLOR {
  BYTE fg_red;
  BYTE fg_green;
  BYTE fg_blue;
  BYTE fg_alpha;
  BYTE bg_red;
  BYTE bg_green;
  BYTE bg_blue;
  BYTE bg_alpha;
} GCOLOR;

static GCOLOR gcolor={255,255,255,255,0,0,0,0};

typedef struct _GDRAW {
  UINT width;
} GDRAW;

static GDRAW gdraw={1};


static int absint(int in) {
  if (in<0) return (-1*in);
  return in;
}

static void swapcoords(UINT *fromx, UINT *fromy, UINT *tox, UINT *toy) {
  UINT tmp;

  tmp=*fromx;
  *fromx=*tox;
  *tox=tmp;

  tmp=*fromy;
  *fromy=*toy;
  *toy=tmp;
}

static int max(int a,int b) {
  if (a>b) return a;
  return b;
}

/* primitive function to get square root (without math.h)*/
/* found on the internet :) */

static float squareroot(int number)
{
    int start = 0, end = number;
    int mid;

    float ans;

    // To find integral part of square
    // root of number
    while (start <= end) {

        // Find mid
        mid = (start + end) / 2;

        // If number is perfect square
        // then break
        if (mid * mid == number) {
            ans = mid;
            break;
        }

        // Increment start if integral
        // part lies on right side
        // of the mid
        if (mid * mid < number) {
            start = mid + 1;
            ans = mid;
        }

        // Decrement end if integral part
        // lies on the left side of the mid
        else {
            end = mid - 1;
        }
    }

    // To find the fractional part
    // of square root upto 5 decimal
    float increment = 0.1;
    for (int i = 0; i < 5; i++) {
        while (ans * ans <= number) {
            ans += increment;
        }

        // Loop terminates,
        // when ans * ans > number
        ans = ans - increment;
        increment = increment / 10;
    }
    return ans;
}

void floodfill(SILLYR *layer, UINT x,UINT y) {
  BYTE red,green,blue,alpha;

  if ((x>=layer->fb->width)||(y>=layer->fb->height)) return;
  sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha);
  if ((red!=gcolor.fg_red)||(green!=gcolor.fg_green)||(blue!=gcolor.fg_blue)||(alpha!=gcolor.fg_alpha)) {
    sil_drawPixel(layer,x,y);
    floodfill(layer,x+1,y);
    if (x>0) floodfill(layer,x-1,y);
    floodfill(layer,x,y+1);
    if (y>0) floodfill(layer,x,y-1);
  } 
}


/*****************************************************************************
  getters & setters for global colors settings

 *****************************************************************************/

void sil_setBackgroundColor(BYTE red,BYTE green, BYTE blue, BYTE alpha) {
  gcolor.bg_red=red;
  gcolor.bg_green=green;
  gcolor.bg_blue=blue;
  gcolor.bg_alpha=alpha;
}

void sil_setForegroundColor(BYTE red,BYTE green, BYTE blue, BYTE alpha) {
  gcolor.fg_red=red;
  gcolor.fg_green=green;
  gcolor.fg_blue=blue;
  gcolor.fg_alpha=alpha;
}

void sil_getBackgroundColor(BYTE *red, BYTE *green, BYTE *blue, BYTE *alpha) {
  *red=gcolor.bg_red;
  *green=gcolor.bg_green;
  *blue=gcolor.bg_blue;
  *alpha=gcolor.bg_alpha;
}

void sil_getForegroundColor(BYTE *red, BYTE *green, BYTE *blue, BYTE *alpha) {
  *red=gcolor.fg_red;
  *green=gcolor.fg_green;
  *blue=gcolor.fg_blue;
  *alpha=gcolor.fg_alpha;
}

/*****************************************************************************
  getters & setters for global drawing settings

 *****************************************************************************/

void sil_setDrawWidth(UINT width) {
  gdraw.width=width;
}

UINT sil_getDrawWidth() {
  return gdraw.width;
}


/*****************************************************************************
  load a PNG on location relx, rely into existing layer.
  Will check"SILFLAG_NOBLEND" to see if it just has to overwrite (including
  alpha) or blend with existing pixels

  In: Context of layer, filename , x and y position (relative to left upper
      corner (0,0) of layer
  Out: Possible errorcode

 *****************************************************************************/

UINT sil_PNGintoLayer(SILLYR *layer,char * filename,UINT relx,UINT rely) {
  BYTE *image =NULL;
  UINT err=0;
  UINT pos=0;
  UINT width=0;
  UINT height=0;
  UINT maxwidth=0;
  UINT maxheight=0;
  BYTE red,green,blue,alpha;
  float af,negaf;
  BYTE mixred,mixgreen,mixblue,mixalpha;

#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("Trying to load PNG into non-existing layer");
    sil_setErr(SILERR_WRONGFORMAT);
    return SILERR_WRONGFORMAT;
  }
  if (NULL==layer->fb) {
    log_warn("Trying to load PNG into layer with no framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return SILERR_WRONGFORMAT;
  }
  if (0==layer->fb->size) {
    log_warn("Trying to load PNG into layer with uninitialized framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return SILERR_WRONGFORMAT;
  }
  if ((relx>layer->fb->width)||(rely>layer->fb->height)) {
    log_warn("Trying to load PNG into layer outside of dimensions");
    sil_setErr(SILERR_WRONGFORMAT);
    return SILERR_WRONGFORMAT;
  }

#endif

    /* load image in temporary framebuffer with alhpa */
    err=lodepng_decode32_file(&image,&width,&height,filename);

  if (err) {
    switch (err) {
      case 28:
      case 29:
        /* wrong file presented as .png */
        log_warn("'%s' is Not an .png file (%d)",filename,err);
        err=SILERR_WRONGFORMAT;
        break;
      case 78:
        /* common error, wrong filename */
        log_warn("Can't open '%s' (%d)",filename,err);
        err=SILERR_CANTOPENFILE;
        break;
      default:
        /* something wrong with decoding png */
        log_warn("Can't decode PNG file '%s' (%d)",filename,err);
        err=SILERR_CANTDECODEPNG;
        break;
    }
    sil_setErr(err);
    if (image) free(image);
    return err;
  }

  /* copy all pixels to own framebuffer in layer */
  if ((layer->fb->width)<width) {
    maxwidth=layer->fb->width;
  } else {
    maxwidth=width;
  }
  if ((layer->fb->height)<height) {
    maxheight=layer->fb->height;
  } else {
    maxheight=height;
  }
  for (int x=0; x<maxwidth; x++) {
    for (int y=0; y<maxheight; y++) {
      pos=4*(x+y*width);
      red  =image[pos++];
      green=image[pos++];
      blue =image[pos++];
      alpha=image[pos];
      if ((x+relx<(layer->fb->width))&&(y+rely<(layer->fb->height))) {
        if (layer->flags&SILFLAG_NOBLEND) {
          sil_putPixelLayer(layer,x+relx,y+rely,red,green,blue,alpha);
        } else {
          sil_blendPixelLayer(layer,x+relx,y+rely,red,green,blue,alpha);
        }
      }
    }
  }
  //log_mark("ENDING");

  /* remove temporary framebuffer */
  if (image) free(image);

  sil_setErr(SILERR_ALLOK);
  return SILERR_ALLOK;
}

/*****************************************************************************
  paintLayer with given color

  In: Context of layer, color

 *****************************************************************************/

void sil_paintLayer(SILLYR *layer, BYTE red, BYTE green, BYTE blue, BYTE alpha) {

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("painting a layer that isn't initialized or has unitialized framebuffer");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif

  for (int x=0;x<layer->fb->width;x++) {
    for (int y=0;y<layer->fb->height;y++) {
      sil_putPixelLayer(layer,x,y,red,green,blue,alpha); 
    }
  }
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************
  drawtext with foreground color onto a layer at position relx,rely

  In: Context of layer, font to use, text to print
      x,y position relative to 0,0 of layer (top-left point) 
      flags: 
        SILTXT_NOKERNING  = Don't apply correction to kerning of characters 
        SILTXT_MONOSPACE  = Every character is as wide as widest char in font
        SILTXT_KEEPCOLOR  = Ignores foreground color and uses color of font
        SILTXT_BLENDLAYER = Blend text pixels with existing pixels 


 *****************************************************************************/


void sil_drawTextLayer(SILLYR *layer, SILFONT *font, char *text, UINT relx, UINT rely, BYTE flags) {
  int cursor=0;
  UINT cnt=0;
  char tch,prevtch;
  BYTE red,green,blue,alpha;
  SILFCHAR *chardef;
  int kerning=0;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("drawing text on a layer that isn't initialized or has unitialized framebuffer");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
  if ((NULL==font)||(NULL==font->image)) {
    log_warn("drawing text on a layer using a nonintialized font");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif

  tch=text[0];
  prevtch=0;
  while((tch)&&(cursor+relx<(layer->fb->width))) {
    chardef=sil_getCharFont(font,tch);
    if (0==chardef) {
      log_warn("can't find character code (%d) in font to draw, ignoring char.",tch);
      continue;
    }
    if (!(flags&SILTXT_NOKERNING) && !(flags&SILTXT_MONOSPACE)) {
      kerning=sil_getKerningFont(font,prevtch,tch);
      cursor+=kerning;
    }
#ifndef SIL_LIVEDANGEROUS
    if (((cursor+relx+chardef->width)>=layer->fb->width)||((rely+chardef->yoffset+chardef->height)>=layer->fb->height)) {
      log_verbose("drawing text outside layer area, skipping char");
      continue;
    }
#endif
    for (int x=0;x<chardef->width;x++) {
      for (int y=0;y<chardef->height;y++) {
        sil_getPixelFont(font,x+chardef->x,y+chardef->y,&red,&green,&blue,&alpha);
        alpha=alpha*(font->alpha);
        if ((alpha>0) && (red+green+blue>0)) {
          if (!(flags&SILTXT_KEEPCOLOR)) {
            red=((float)red/255)*gcolor.fg_red;
            green=((float)green/255)*gcolor.fg_green;
            blue=((float)blue/255)*gcolor.fg_blue;
            alpha=((float)alpha/255)*gcolor.fg_alpha;
          }
          if (flags&SILTXT_BLENDLAYER) {
            sil_blendPixelLayer(layer,cursor+x+relx,y+rely+chardef->yoffset,red,green,blue,alpha);
          } else {
            sil_putPixelLayer(layer,cursor+x+relx,y+rely+chardef->yoffset,red,green,blue,alpha);
          }
        }
      }
    }
    if (flags&SILTXT_MONOSPACE) {
      cursor+=font->mspace;
    } else {
      cursor+=chardef->xadvance;
    }
    prevtch=tch;
    tch=text[++cnt];
  }
  sil_setErr(SILERR_ALLOK);
}


/*****************************************************************************

   dump screen to given .png file with given width and height at position x,y

 *****************************************************************************/

UINT sil_saveDisplay(char *filename,UINT width, UINT height, UINT wx, UINT wy) {
  SILFB *fb;
  SILLYR *layer;
  BYTE red,green,blue,alpha;
  BYTE mixred,mixgreen,mixblue,mixalpha;
  float af;
  float negaf;
  UINT err=0;

  
  /* first create framebuffer to hold information */
  fb=sil_initFB(width,height,SILTYPE_888BGR);
  if (NULL==fb) {
    log_warn("Can't initialize framebuffer in order to dump to png file");
    return SILERR_NOTINIT;
  }

  /* merge all layers to single fb - within window of given paramaters  */
  layer=sil_getBottom();
  while (layer) {
    if (!(layer->flags&SILFLAG_INVISIBLE)) {
      for (int y=layer->view.miny; y<(layer->view.miny+layer->view.height); y++) {
        for (int x=layer->view.minx; x<(layer->view.minx+layer->view.width); x++) {
          int absx=x+layer->relx-layer->view.minx;
          int absy=y+layer->rely-layer->view.miny;
          int rx=x;
          int ry=y;

          /* draw if it is not within window */
          if ((absx<wx)||(absy<wy)||(absx>wx+width)||(absy>wy+height)) continue;

          /* adjust wx,wy to 0,0 of framebuffer */
          absx-=wx;
          absy-=wy;

          sil_getPixelLayer(layer,rx,ry,&red,&green,&blue,&alpha);
          if (0==alpha) continue; /* nothing to do if completely transparant */
          alpha=alpha*layer->alpha;
          if (255==alpha) {
            sil_putPixelFB(fb,absx,absy,red,green,blue,255);
          } else {
            sil_getPixelFB(fb,absx,absy,&mixred,&mixgreen,&mixblue,&mixalpha);
            af=((float)alpha)/255;
            negaf=1-af;
            red=red*af+negaf*mixred;
            green=green*af+negaf*mixgreen;
            blue=blue*af+negaf*mixblue;
            sil_putPixelFB(fb,absx,absy,red,green,blue,255);
          }
        }
      }
    }
    layer=layer->next;
  }

  /* write to file */
  err=lodepng_encode24_file(filename, fb->buf, width, height);
  if (err) {
    switch (err) {
      case 79:
        /* common error, wrong filename, no rights */
        log_warn("Can't open '%s' for writing (%d)",filename,err);
        err=SILERR_CANTOPENFILE;
        break;
      default:
        /* something wrong with encoding png */
        log_warn("Can't encode PNG file '%s' (%d)",filename,err);
        err=SILERR_CANTDECODEPNG;
        break;
    }
    return err;
  }

  sil_setErr(SILERR_ALLOK);
  return SILERR_ALLOK;
}

/*****************************************************************************

 ----------------------------------------------------------------------
 Based from code "thickLine.cpp" from Armin Joachimsmeyer under LGPL
 ----------------------------------------------------------------------

 internal Draw Single Line (not anti-aliased) from x1,y1 to x2,y2 
 with current color and possibility to overlap

 Sample line : (0=SILLO_NONE, +=SILLO_MAJOR, -=SILLO_MINOR)

     00+
      -0000+
          -0000+
              -00

 *****************************************************************************/

void drawSingleLine(SILLYR *layer,UINT x1, UINT y1, UINT x2, UINT y2, BYTE overlap) {
  int tDeltaX, tDeltaY, tDeltaXTimes2, tDeltaYTimes2, tError, tStepX, tStepY;
  int tmp;

  /* if it is an horizontal or vertical line, just use rectangle function */
  if (x1 == x2) {
    if (y2<y1) swapcoords(&x1,&y1,&x2,&y2);
    while(y1<=y2) {
      sil_putPixelLayer(layer, x1,y1++, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
    }
    return;
  }

  if (y1 == y2) {
    if (x2<x1) swapcoords(&x1,&y1,&x2,&y2);
    while(x1<=x2) {
      sil_putPixelLayer(layer, x1++,y1, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
    } 
    return;
  }

  /* bresenham algorithm */

  /* Direction */

  tDeltaX=absint(x2-x1);
  tDeltaY=absint(y2-y1);
  tStepX=x1<x2 ? 1 : -1;
  tStepY=y1<y2 ? 1 : -1;
  tDeltaXTimes2 = tDeltaX*2;
  tDeltaYTimes2 = tDeltaY*2;

  sil_drawPixel(layer,x1,y1);

  if (tDeltaX > tDeltaY) {
    /* stepping over X axis */
    tError = tDeltaYTimes2 - tDeltaX;
    while (x1 != x2) {
      x1 += tStepX;
      if (tError >= 0) {
        if (overlap & SILLO_MAJOR) sil_drawPixel(layer,x1,y1);
        y1 += tStepY;
        if (overlap & SILLO_MINOR) sil_drawPixel(layer,x1-tStepX,y1);
        tError -= tDeltaXTimes2;
      }
      tError += tDeltaYTimes2;
      sil_drawPixel(layer,x1,y1);
    }
  } else {
    /* stepping over y axis */
    tError = tDeltaXTimes2 - tDeltaY;
    while (y1 != y2) {
      y1 += tStepY;
      if (tError >= 0) {
        if (overlap & SILLO_MAJOR) sil_drawPixel(layer,x1,y1);
        x1 += tStepX;
        if (overlap & SILLO_MINOR) sil_drawPixel(layer,x1,y1-tStepY);
        tError -= tDeltaYTimes2;
      }
      tError += tDeltaXTimes2;
      sil_drawPixel(layer,x1,y1);
    }
  }
  sil_drawPixel(layer,x2,y2);
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************

  Draw Line (not anti-aliased) from x1,y1 to x2,y2 
  with current color and thickness

 *****************************************************************************/

void sil_drawLine(SILLYR *layer, UINT x1, UINT y1, UINT x2, UINT y2) {
  int i,tDeltaX,tDeltaY,tDeltaXTimes2, tDeltaYTimes2, tError, tStepX, tStepY;
  int tDrawStartAdjustCount;
  BYTE tSwap=1;
  BYTE tOverlap=0;


#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("Trying to draw non-existing layer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
  if (NULL==layer->fb) {
    log_warn("Trying to draw on layer with no framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
  if (0==layer->fb->size) {
    log_warn("Drawing on layer without initialized framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
#endif

  /* if it has one single line, use that function */
  if (gdraw.width<2) {
    drawSingleLine(layer,x1,y1,x2,y2,SILLO_NONE);
    return;
  }
  tDeltaY=absint(x2-x1);
  tDeltaX=absint(y2-y1);
  tSwap=1;
  if (x1>x2) {
    tStepX=-1;
    tSwap=0;
  } else {
    tStepX=1;
  }
  if (y1>y2) {
    tStepY=-1;
    tSwap=(0==tSwap)?1:0;
  } else {
    tStepY=1;
  }
  tDeltaXTimes2=tDeltaX*2;
  tDeltaYTimes2=tDeltaY*2;
  tDrawStartAdjustCount=gdraw.width/2;

  if (tDeltaX >= tDeltaY) {
    /* step over X */

    if (tSwap) {
      tDrawStartAdjustCount = (gdraw.width-1)-tDrawStartAdjustCount;
      tStepY=-tStepY;
    } else {
      tStepX=-tStepX;
    }

    tError=tDeltaYTimes2 - tDeltaX;
    for (i = tDrawStartAdjustCount; i > 0; i--) {
      x1 -= tStepX;
      x2 -= tStepX;
      if (tError >= 0) {
        y1 -= tStepY;
        y2 -= tStepY;
        tError -= tDeltaXTimes2;
      }
      tError += tDeltaYTimes2;
    }
    /* start line */
    drawSingleLine(layer,x1,y1,x2,y2,SILLO_NONE);

    /* draw gdraw.width number of lines */
    tError = tDeltaYTimes2 - tDeltaX;
    for (i = gdraw.width; i > 1; i--) {
      x1 += tStepX;
      x2 += tStepX;
      tOverlap = SILLO_NONE;
      if (tError >= 0) {
        y1 += tStepY;
        y2 += tStepY;
        tError -= tDeltaXTimes2;
        tOverlap = SILLO_MAJOR|SILLO_MINOR;
      }
      tError += tDeltaYTimes2;
      drawSingleLine(layer,x1, y1, x2, y2, tOverlap);
    }
  } else {
  /* step over Y */

    if (tSwap) {
      tStepX = -tStepX;
    } else {
      tDrawStartAdjustCount = (gdraw.width-1) - tDrawStartAdjustCount;
      tStepY = -tStepY;
    }

    tError = tDeltaXTimes2 - tDeltaY;
    for (i = tDrawStartAdjustCount; i > 0; i--) {
      y1 -= tStepY;
      y2 -= tStepY;
      if (tError >= 0) {
        x1 -= tStepX;
        x2 -= tStepX;
        tError -= tDeltaYTimes2;
      }
      tError += tDeltaXTimes2;
    }

    drawSingleLine(layer,x1, y1, x2, y2, SILLO_NONE);

    tError = tDeltaXTimes2 - tDeltaY;
    for (i = gdraw.width; i > 1; i--) {
      y1 += tStepY;
      y2 += tStepY;
      tOverlap = SILLO_NONE;
      if (tError >= 0) {
        x1 += tStepX;
        x2 += tStepX;
        tError -= tDeltaYTimes2;
        tOverlap = SILLO_MAJOR|SILLO_MINOR;
      }
      tError += tDeltaXTimes2;
      drawSingleLine(layer,x1, y1, x2, y2, tOverlap);
    }
  }

  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************

  Same as Draw Single Line, but with 90 grade angle and 
  stops when line has lenght "len" 

 *****************************************************************************/

static void drawLineLen(SILLYR *layer, UINT x1, UINT y1, UINT x2, UINT y2, UINT len) {
  int dx,sx,dy,sy,err,e2;
  UINT cx,cy;
  UINT px,py;

  /* just handle straight lines first */
  if (y1==y2) {
    if (x2<x1) swapcoords(&x1,&y1,&x2,&y2);
    while(len--) {
      sil_putPixelLayer(layer, x1++,y1, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
    }
    return;
  }
  if (x1==x2) {
    if (y2<y1) swapcoords(&x1,&y1,&x2,&y2);
    while(len--) {
      sil_putPixelLayer(layer, x1,y1++, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
    }
    return;
  }
  
  
  /* bresenham algorithm */

  dx=absint(x2-x1);
  sx=x1<x2 ? 1 : -1;
  dy=absint(y2-y1);
  sy=y1<y2 ? 1 : -1;
  err=(dx>dy ? dx : -dy)/2;
  cx=0;
  cy=0;
  px=x1;
  py=y1;

  do {
    sil_drawPixel(layer,px-(cy*sy),py+(cx*sx));
    sil_drawPixel(layer,px+(cy*sy),py-(cx*sx));
    sil_drawPixel(layer,x2-(cy*sy),y2+(cx*sx));
    sil_drawPixel(layer,x2+(cy*sy),y2-(cx*sx));
    e2 = err;
    if (e2>-dx) { 
      err-=dy; 
      x1+=sx; 
      cx++;
    }
    if (e2<dx) { 
      err+=dx; 
      y1+=sy; 
      cy++;
    }
  } while (len*len>=((cx*cx)+(cy*cy)));
  sil_setErr(SILERR_ALLOK);

}

/*****************************************************************************

  Draw Single Line (not anti-aliased) from x1,y1 to x2,y2 
   with current color and thickness

 *****************************************************************************/


void sil_drawLine2(SILLYR *layer, UINT x1, UINT y1, UINT x2, UINT y2) {
  int plus,minus;
  UINT addy=0;
  UINT addx=0;
  int dx,dy;

#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("Trying to draw non-existing layer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
  if (NULL==layer->fb) {
    log_warn("Trying to draw on layer with no framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
  if (0==layer->fb->size) {
    log_warn("Drawing on layer without initialized framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
#endif
  
  if (1==gdraw.width) {
    drawSingleLine(layer,x1,y1,x2,y2,SILLO_NONE);
    return;
  }
  
  minus=(int)gdraw.width/2;
  plus=gdraw.width-minus;

/*
  if (absint(ix2-ix1) > absint(iy2-iy1)) {
    addy=1;
    addx=0;
  } else {
    addx=1;
    addy=0;
  }
  for (int lw=minus;lw<plus;lw++) {
    drawSingleLine(layer,ix1+addx*lw,iy1+addy*lw,ix2+addx*lw,iy2+addy*lw);
  }
*/
  dx=x2-x1;
  dy=y2-y1;
  drawSingleLine(layer,x1,y1,x2,y2,SILLO_NONE);
  drawLineLen(layer,x1,y1,x2,y2,plus);
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************

   Internal function , Draw single Line anti-aliased from x1,y1 to x2,y2 
   with current color

 *****************************************************************************/
static void drawSingleLineAA(SILLYR *layer, UINT fromx, UINT fromy, UINT tox, UINT toy) {
  int dx,dy;
  UINT alpha1,alpha2;
  BYTE overx=0;
  UINT addx=0;
  UINT addy=0;
  UINT point=0;
  UINT end=0;
  float fraction=0;
  float step=0;

  /* No AA needed for straight lines */
  if ((toy==fromy)||(tox==fromx)) {
    drawSingleLine(layer,fromx,fromy,tox,toy,SILLO_NONE);
    return;
  }

  /* and now a Xiaolin Wu's algorithm */

  //log_info("Drawing: %d,%d -> %d,%d",fromx,fromy,tox,toy);
  if (absint(tox-fromx)>absint(toy-fromy)) {
    /* we progress over the X axis */
    overx=1;
    addx=0;
    addy=1;
    /* draw always from left to right */
    if (tox<fromx) swapcoords(&tox,&toy,&fromx,&fromy);
    dx=tox-fromx;
    dy=toy-fromy;
    if (dy<0) addy=-1;
    step=(float)absint(dy)/(float)absint(dx);
    point=fromx;
    end=tox;
  } else {
    /* we progress over the Y axis */
    overx=0;
    addx=1;
    addy=0;
    /* draw always from top to bottom */
    if (toy<fromy) swapcoords(&tox,&toy,&fromx,&fromy);
    dx=tox-fromx;
    dy=toy-fromy;
    if (dx<0) addx=-1;
    step=(float)absint(dx)/(float)absint(dy);
    point=fromy;
    end=toy;
  }

    //log_info("OverX: %d, addx: %d, addy: %d, point: %d, end: %d",overx,addx,addy,point,end);
  while(point++<=end) {
    alpha2=gcolor.fg_alpha*fraction;
    alpha1=gcolor.fg_alpha-alpha2;
    sil_blendPixelLayer(layer, fromx ,fromy, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, alpha1);
    sil_blendPixelLayer(layer, fromx+addx ,fromy+addy, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, alpha2);

    fraction+=step;
    if (fraction>1) {
      fraction-=1;
      fromy+=addy;
      fromx+=addx;
    }
    if (1==overx) {
      fromx++;
    } else {
      fromy++;
    }
  }
  sil_setErr(SILERR_ALLOK);
}


/*****************************************************************************

   Draw Line anti-aliased from x1,y1 to x2,y2 with current color and thickness

 *****************************************************************************/

void sil_drawLineAA(SILLYR *layer, UINT ix1, UINT iy1, UINT ix2, UINT iy2) {
  UINT fromx,fromy,tox,toy;
  UINT addy=0;
  UINT addx=0;
  int plus,minus;
  float d=0;

#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("Trying to draw non-existing layer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
  if (NULL==layer->fb) {
    log_warn("Trying to draw on layer with no framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
  if (0==layer->fb->size) {
    log_warn("Drawing on layer without initialized framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
#endif


  /* straight lines don't need aliasing */
  if ((ix1==ix2)||(iy1==iy2)) {
    sil_drawLine(layer,ix1,iy1,ix2,iy2);
    return;
  }

  if (1==gdraw.width) {
    drawSingleLineAA(layer,ix1,iy1,ix2,iy2);
    return;
  }

  minus=-(int)gdraw.width/2;
  plus=gdraw.width+minus-1;
  if (absint(ix2-ix1) > absint(iy2-iy1)) {
    addy=1;
  } else {
    addx=1;
  }

  /* draw AA outerborders first */
  drawSingleLineAA(layer,ix1+addx*minus,iy1+addy*minus,ix2+addx*minus,iy2+addy*minus);
  drawSingleLineAA(layer,ix1+addx*plus,iy1+addy*plus,ix2+addx*plus,iy2+addy*plus);

  /* fill in with non-AA on top of it */
  for (int lw=minus;lw<plus;lw++) {
    drawSingleLine(layer,ix1+addx*lw,iy1+addy*lw,ix2+addx*lw,iy2+addy*lw,SILLO_NONE);
  }
  sil_setErr(SILERR_ALLOK);
}
  

/* draw functions for BIG pixels, to check out errors in algorithm by sight */


static void drawBigSingleLine(SILLYR *layer, UINT x1, UINT y1, UINT x2, UINT y2) {
  UINT fromx,fromy,tox,toy;
  int dx,dy,p;
  int add=1;

  /* make sure lines always go from left to right */
  if (x1>x2) {
    tox=x1;
    toy=y1;
    fromx=x2;
    fromy=y2;
  } else {
    tox=x2;
    toy=y2;
    fromx=x1;
    fromy=y1;
  }
   

  /* just handle straight lines first */
  if (toy==fromy) {
    while(fromx!=tox) {
      sil_putBigPixelLayer(layer, fromx++,toy, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
    }
    return;
  }
  if (tox==fromx) {
    if (y1>y2) { 
      fromy=y2;
      toy=y1;
    } else {
      fromy=y1;
      toy=y2;
    } 
    while(toy!=fromy) {
      sil_putBigPixelLayer(layer, fromx,fromy++, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
    }
    return;
  }
  
  /* and now a Bresenham's algorithm */
  dx=tox-fromx;
  dy=toy-fromy;
  if (dy<0) {
    dy=-1*dy;
    add=-1;
  }
  if (dy<=dx) {
    p=2*dy-dx;
    while(fromx<tox) {
      sil_putBigPixelLayer(layer, fromx ,fromy, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
      if (p>=0) {
        fromy+=add;
        p+=2*(dy-dx);
      } else {
        p+=2*dy;
      }
      fromx++;
    }
  } else {
    /* to steep, draw with looping y instead of x */
    /* makesure to draw from top till bottom      */
    if (y1>y2) {
      tox=x1;
      toy=y1;
      fromx=x2;
      fromy=y2;
    } else {
      tox=x2;
      toy=y2;
      fromx=x1;
      fromy=y1;
    }
    dx=tox-fromx;
    dy=toy-fromy;
    add=1;
    if (dx<0) {
      dx=-1*dx;
      add=-1;
    }

    while(fromy<toy) {
      sil_putBigPixelLayer(layer, fromx ,fromy, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
      if (p>=0) {
        fromx+=add;
        p+=2*(dx-dy);
      } else {
        p+=2*dx;
      }
      fromy++;
    }
  }
  sil_setErr(SILERR_ALLOK);
}


void sil_drawBigLine(SILLYR *layer, UINT ix1, UINT iy1, UINT ix2, UINT iy2) {
  int plus,minus;
  UINT addy=0;
  UINT addx=0;

#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("Trying to draw non-existing layer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
  if (NULL==layer->fb) {
    log_warn("Trying to draw on layer with no framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
  if (0==layer->fb->size) {
    log_warn("Drawing on layer without initialized framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
#endif
  
  if (1==gdraw.width) {
    drawBigSingleLine(layer,ix1,iy1,ix2,iy2);
    return;
  }

  minus=-(int)gdraw.width/2;
  plus=gdraw.width+minus;
  if (absint(ix2-ix1) > absint(iy2-iy1)) {
    addy=1;
  } else {
    addx=1;
  }
  for (int lw=minus;lw<plus;lw++) {
    drawBigSingleLine(layer,ix1+addx*lw,iy1+addy*lw,ix2+addx*lw,iy2+addy*lw);
  }
  sil_setErr(SILERR_ALLOK);
}


static void drawBigSingleLineAA(SILLYR *layer, UINT fromx, UINT fromy, UINT tox, UINT toy) {
  int dx,dy;
  UINT alpha1,alpha2;
  BYTE overx=0;
  UINT addx=0;
  UINT addy=0;
  UINT point=0;
  UINT end=0;
  float fraction=0;
  float step=0;

  /* No AA needed for straight lines */
  if ((toy==fromy)||(tox==fromx)) {
    drawBigSingleLine(layer,fromx,fromy,tox,toy);
    return;
  }

  /* and now a Xiaolin Wu's algorithm */

  //log_info("Drawing: %d,%d -> %d,%d",fromx,fromy,tox,toy);
  if (absint(tox-fromx)>absint(toy-fromy)) {
    /* we progress over the X axis */
    overx=1;
    addx=0;
    addy=1;
    /* draw always from left to right */
    if (tox<fromx) swapcoords(&tox,&toy,&fromx,&fromy);
    dx=tox-fromx;
    dy=toy-fromy;
    if (dy<0) addy=-1;
    step=(float)absint(dy)/(float)absint(dx);
    point=fromx;
    end=tox;
  } else {
    /* we progress over the Y axis */
    overx=0;
    addx=1;
    addy=0;
    /* draw always from top to bottom */
    if (toy<fromy) swapcoords(&tox,&toy,&fromx,&fromy);
    dx=tox-fromx;
    dy=toy-fromy;
    if (dx<0) addx=-1;
    step=(float)absint(dx)/(float)absint(dy);
    point=fromy;
    end=toy;
  }

    //log_info("OverX: %d, addx: %d, addy: %d, point: %d, end: %d",overx,addx,addy,point,end);
  while(point++<end) {
    alpha2=gcolor.fg_alpha*fraction;
    alpha1=gcolor.fg_alpha-alpha2;
    sil_blendBigPixelLayer(layer, fromx ,fromy, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, alpha1);
    sil_blendBigPixelLayer(layer, fromx+addx ,fromy+addy, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, alpha2);

    fraction+=step;
    if (fraction>1) {
      fraction-=1;
      fromy+=addy;
      fromx+=addx;
    }
    if (1==overx) {
      fromx++;
    } else {
      fromy++;
    }
  }

  sil_setErr(SILERR_ALLOK);
}

void sil_drawBigLineAA(SILLYR *layer, UINT ix1, UINT iy1, UINT ix2, UINT iy2) {
  UINT fromx,fromy,tox,toy;
  int plus,minus;
  UINT addy=0;
  UINT addx=0;

#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("Trying to draw non-existing layer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
  if (NULL==layer->fb) {
    log_warn("Trying to draw on layer with no framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
  if (0==layer->fb->size) {
    log_warn("Drawing on layer without initialized framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
#endif


  /* straight lines don't need aliasing */
  if ((ix1==ix2)||(iy1==iy2)) {
    sil_drawLine(layer,ix1,iy1,ix2,iy2);
    return;
  }
  
  if (1==gdraw.width) {
    drawBigSingleLineAA(layer,ix1,iy1,ix2,iy2);
    return;
  }

  minus=-(int)gdraw.width/2;
  plus=gdraw.width+minus-1;
  if (absint(ix2-ix1) > absint(iy2-iy1)) {
    addy=1;
  } else {
    addx=1;
  }

  /* draw AA outerborders first */
  drawBigSingleLineAA(layer,ix1+addx*minus,iy1+addy*minus,ix2+addx*minus,iy2+addy*minus);
  drawBigSingleLineAA(layer,ix1+addx*plus,iy1+addy*plus,ix2+addx*plus,iy2+addy*plus);

  /* fill in with non-AA on top of it */
  for (int lw=minus;lw<plus;lw++) {
    drawBigSingleLine(layer,ix1+addx*lw,iy1+addy*lw,ix2+addx*lw,iy2+addy*lw);
  }
  sil_setErr(SILERR_ALLOK);
}


/*****************************************************************************

   Draw Single Circle with xm,ym being middle point and r the radius  

 *****************************************************************************/

static void drawSingleCircle(SILLYR *layer, UINT xm, UINT ym, UINT r) {
  int x=r;
  int y=0;
  int px=x,py=y;
  int xch=1-2*r;
  int ych=1;
  int rerr=0;

  while(x>=y) {
    sil_drawPixel(layer,xm + x, ym + y);
    sil_drawPixel(layer,xm - x, ym + y);
    sil_drawPixel(layer,xm - x, ym - y);
    sil_drawPixel(layer,xm + x, ym - y);
    sil_drawPixel(layer,xm + y, ym + x);
    sil_drawPixel(layer,xm - y, ym + x);
    sil_drawPixel(layer,xm - y, ym - x);
    sil_drawPixel(layer,xm + y, ym - x);
    y++;
    rerr+=ych;
    ych+=2;
    if (2*rerr+xch > 0) {
      x--;
      rerr+=xch;
      xch+=2;
    }
  }
}



/*****************************************************************************

   Draw Circle with xm,ym being middle point and r the radius and using 
   drawing with 

 *****************************************************************************/
void sil_drawCircle(SILLYR *layer, UINT xm, UINT ym, UINT r) {
  int minus,plus;

#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("Trying to draw non-existing layer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
  if (NULL==layer->fb) {
    log_warn("Trying to draw on layer with no framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
  if (0==layer->fb->size) {
    log_warn("Drawing on layer without initialized framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
#endif

  if (1==gdraw.width) {
    drawSingleCircle(layer,xm,ym,r);
    return;
  }

  minus=-(int)gdraw.width/2;
  plus=gdraw.width+minus-1;
  drawSingleCircle(layer,xm,ym,r+minus);
  drawSingleCircle(layer,xm,ym,r+plus);
  floodfill(layer,xm+r-1,ym);
  sil_setErr(SILERR_ALLOK);
}


/*****************************************************************************

   Draw Rectangle at x,y using given width and height

 *****************************************************************************/

void sil_drawRectangle(SILLYR *layer, UINT x, UINT y, UINT width, UINT height) {

#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("Trying to draw non-existing layer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
  if (NULL==layer->fb) {
    log_warn("Trying to draw on layer with no framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
  if (0==layer->fb->size) {
    log_warn("Drawing on layer without initialized framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return ;
  }
#endif

  for (UINT yc=0;yc<height;yc++) {
    for (UINT xc=0;xc<width;xc++) {
      if (((x+xc)<layer->fb->width)&&((y+yc)<layer->fb->height)) {
        if ((xc<gdraw.width)||(xc>width-gdraw.width)||(yc<gdraw.width)||(yc>height-gdraw.width)) {
          /* border */
          sil_putPixelLayer(layer, x+xc, y+yc, gcolor.bg_red, gcolor.bg_green, gcolor.bg_blue, gcolor.bg_alpha);
        } else {
          sil_blendPixelLayer(layer, x+xc, y+yc, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
        }
      }
    }
  }
  sil_setErr(SILERR_ALLOK);
}





/*****************************************************************************

   Draw Pixel at x,y with current color

 *****************************************************************************/

void sil_drawPixel(SILLYR *layer, UINT x, UINT y) {
  /* checks on validity of layer, fb and all is done in putPixelLayer function already */
  sil_putPixelLayer(layer, x, y, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
}

/*****************************************************************************

   Draw Pixel at x,y with current color and blend with existing pixels

 *****************************************************************************/


void sil_blendPixel(SILLYR *layer, UINT x, UINT y) {
  /* checks on validity of layer, fb and all is done in blendPixelLayer function already */
  sil_blendPixelLayer(layer, x, y, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
}


