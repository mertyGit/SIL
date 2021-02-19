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

  /* load image in temporary framebuffer */
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
