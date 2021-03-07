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

/*****************************************************************************
  getters & setters for those colors

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

   Draw Line (not anti-aliased) from x1,y1 to x2,y2 with current color

 *****************************************************************************/

void sil_drawLine(SILLYR *layer, UINT x1, UINT y1, UINT x2, UINT y2) {
  UINT fromx,fromy,tox,toy;
  int dx,dy,p;
  int add=1;

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
      sil_putPixelLayer(layer, fromx++,toy, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
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
      sil_putPixelLayer(layer, fromx,fromy++, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
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
      sil_putPixelLayer(layer, fromx ,fromy, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
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
      sil_putPixelLayer(layer, fromx ,fromy, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
      if (p>=0) {
        fromx+=add;
        p+=2*(dx-dy);
      } else {
        p+=2*dx;
      }
      fromy++;
    }
  }
}

static int abs(int in) {
  if (in<0) return (-1*in);
  return in;
}

/*****************************************************************************

   Internal function , Draw single Line anti-aliased from x1,y1 to x2,y2 
   with current color

 *****************************************************************************/
static void drawSingleLineAA(SILLYR *layer, UINT fromx, UINT fromy, UINT tox, UINT toy) {
    int dx,dy;
    BYTE overx=0;
    UINT add=1;

    /* No AA needed for straight lines */
    if ((toy==fromy)||(tox==fromx)) {
      drawSingleLine(layer,fromx,fromy,tox,toy);
      return;
    }

    /* and now a Xiaolin Wu's algorithm */
    dx=tox-fromx;
    dy=toy-fromy;

    if (abs(dx)>abs(dy)) {
      overx=1;
      if (dy<0) {
        dy=absdy
        add=-1;
      }






    if (dy<0) {
      dy=-1*dy;
      add=-1;
    }
    fraction=0;
    if (dy<=dx) {
      step=(float)dy/(float)dx;
      while(fromx<tox) {
        alpha2=gcolor.fg_alpha*fraction;
        alpha1=gcolor.fg_alpha-alpha2;
        sil_blendPixelLayer(layer, fromx ,fromy, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, alpha1);
        sil_blendPixelLayer(layer, fromx ,fromy+add, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, alpha2);
        fromx++;
        fraction+=step;
        if (fraction>1) {
          fromy+=add;
          fraction-=1;
        }
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
      step=(float)dx/(float)dy;

      while(fromy<toy) {
        alpha2=255*fraction;
        alpha1=255-alpha2;
        alpha2=gcolor.fg_alpha*fraction;
        alpha1=gcolor.fg_alpha-alpha2;
        sil_blendPixelLayer(layer, fromx,fromy, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, alpha1);
        sil_blendPixelLayer(layer, fromx+add ,fromy, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, alpha2);
        fromy++;
        fraction+=step;
        if (fraction>1) {
          fromx+=add;
          fraction-=1;
        }
      }

    } 
    /* single line                                */
    /* quick hack: get rid of AA inside the lines */

    if ((lw>minus)&&(lw<plus-1)) {
      sil_drawLine(layer,x1,y1,x2,y2);
    }
  }  
  
}


/*****************************************************************************

   Draw Line anti-aliased from x1,y1 to x2,y2 with current color and thickness

 *****************************************************************************/

void sil_drawLineAA(SILLYR *layer, UINT ix1, UINT iy1, UINT ix2, UINT iy2) {
  UINT fromx,fromy,tox,toy;
  int dx,dy,p;
  int add=1;
  float fraction;
  float step;
  UINT alpha1,alpha2;
  UINT x1,y1,x2,y2;
  BYTE noaa=0;

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
  
  int gw=7;
  int minus=-gw/2;
  int plus=gw+minus;
  int t=0;
  int cnt;
  int lw=minus;

  for (lw=minus;lw<plus;lw++) {
    y1=iy1;
    y2=iy2;
    x1=ix1;
    x2=ix2;

    if ((x2-x1)*(x2-x1) > (y2-y1)*(y2-y1)) {
      y1+=lw; y2+=lw;
    } else {
      x1+=lw; x2+=lw;
    }

@@
}







/*****************************************************************************

   Draw Pixel at x,y with current color

 *****************************************************************************/

void sil_drawPixel(SILLYR *layer, UINT x, UINT y) {
  /* checks on validity of layer, fb and all is done in putPixelLayer function already */
  sil_putPixelLayer(layer, x, y, gcolor.fg_red, gcolor.fg_green, gcolor.fg_blue, gcolor.fg_alpha);
}
