/*

   filter.c CopyRight 2021 Remco Schellekens, see LICENSE for more details.

   This file contains all functions for applying filters to layers
   All filters are permanent and not reversable, in some cases a temporary
   framebuffer has to be created, so might costs temporary memory

*/

#include <stdlib.h>
#include <stdio.h>
#include "sil.h"
#include "log.h"


static UINT precheck(SILLYR *layer) {
  if ((NULL==layer)||(0==layer->init)) {
    log_warn("Trying to apply filter against non-existing or non-initialized layer");
    sil_setErr(SILERR_NOTINIT);
    return SILERR_NOTINIT;
  }
  if (NULL==layer->fb) {
    log_warn("Trying to apply filter against layer with no framebuffer");
    sil_setErr(SILERR_NOTINIT);
    return SILERR_NOTINIT;
  }
  if (0==layer->fb->size) {
    log_warn("Trying to apply filter against layer with zero size framebuffer");
    sil_setErr(SILERR_WRONGFORMAT);
    return SILERR_WRONGFORMAT;
  }
  return SILERR_ALLOK;
}

UINT sil_cropAlphaFilter(SILLYR *layer) {
  UINT err=SILERR_ALLOK;
  UINT minx,maxx,miny,maxy;
  BYTE red,green,blue,alpha;

#ifndef SIL_LIVEDANGEROUS
  err=precheck(layer);
  if (SILERR_ALLOK!=err) return err;
#endif
  minx=layer->fb->width-1;
  miny=layer->fb->height-1;
  maxx=0;
  maxy=0;
  for (int x=0;x<layer->fb->width;x++) {
    for (int y=0;y<layer->fb->height;y++) {
      sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha); 
      if (alpha) {
        if (x<minx) minx=x;
        if (y<miny) miny=y;
        if (maxx<x) maxx=x;
        if (maxy<y) maxy=y;
      } 
    }
  }
  err=sil_resizeLayer(layer,minx,miny,maxx-minx+1,maxy-miny+1);

  sil_setErr(err);
  return err;
}

UINT sil_cropFirstpixelFilter(SILLYR *layer) {
  UINT err=SILERR_ALLOK;
  UINT minx,maxx,miny,maxy;
  BYTE red,green,blue,alpha;
  BYTE red2,green2,blue2,alpha2;

#ifndef SIL_LIVEDANGEROUS
  err=precheck(layer);
  if (SILERR_ALLOK!=err) return err;
#endif

  minx=layer->fb->width-1;
  miny=layer->fb->height-1;
  maxx=0;
  maxy=0;
  sil_getPixelLayer(layer,0,0,&red2,&green2,&blue2,&alpha2); 
  for (int x=0;x<layer->fb->width;x++) {
    for (int y=0;y<layer->fb->height;y++) {
      sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha); 
      if ((red2!=red)||(green2!=green)||(blue2!=blue)) {
        if (x<minx) minx=x;
        if (y<miny) miny=y;
        if (maxx<x) maxx=x;
        if (maxy<y) maxy=y;
      } 
    }
  }
  err=sil_resizeLayer(layer,minx,miny,maxx-minx+1,maxy-miny+1);

  sil_setErr(err);
  return err;
}


UINT sil_brightnessFilter(SILLYR *layer, int amount) {
  UINT err=SILERR_ALLOK;
  BYTE red,green,blue,alpha;


#ifndef SIL_LIVEDANGEROUS
  err=precheck(layer);
  if (SILERR_ALLOK!=err) return err;
#endif

  if (amount<-255) amount=-255;
  if (amount>255) amount=255;

  for (int x=0;x<layer->fb->width;x++) {
    for (int y=0;y<layer->fb->height;y++) {
      sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha); 
      if (amount>0) {
        if (red+amount<=255) 
          red+=amount;
        else
          red=255;
        if (blue+amount<=255) 
          blue+=amount;
        else
          blue=255;
        if (green+amount<=255) 
          green+=amount;
        else
          green=255;
      } else {
        if (red+amount>=0) 
          red+=amount;
        else
          red=0;
        if (green+amount>=0) 
          green+=amount;
        else
          green=0;
        if (blue+amount>=0) 
          blue+=amount;
        else
          blue=0;
      }
      sil_putPixelLayer(layer,x,y,red,green,blue,alpha); 
    }
  }

  sil_setErr(err);
  return err;
}

UINT sil_blurFilter(SILLYR *layer ) {
  UINT err=SILERR_ALLOK;
  SILFB *dest;
  UINT cnt;
  BYTE red,green,blue,alpha;
  UINT dred,dgreen,dblue,dalpha;

#ifndef SIL_LIVEDANGEROUS
  err=precheck(layer);
  if (SILERR_ALLOK!=err) return err;
#endif

  /* for this, we need to create a seperate FB temporary */

  dest=sil_initFB(layer->fb->width,layer->fb->height,layer->fb->type);
  if (NULL==dest) {
    log_info("ERR: Cant create framebuffer for blur filter");
    return sil_getErr();
  }
  for (int x=0;x<layer->fb->width;x++) {
    for (int y=0;y<layer->fb->height;y++) {
      cnt=0;
      /* mid */
      sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha);
      dred=red;
      dgreen=green;
      dblue=blue;
      dalpha=alpha;
      cnt=1;
      /* top */
      if (y-1>0) {
        sil_getPixelLayer(layer,x,y-1,&red,&green,&blue,&alpha);
        dred+=red;
        dgreen+=green;
        dblue+=blue;
        dalpha+=alpha;
        cnt++;
        /* topright */
        if (x+1<layer->fb->width) {
          sil_getPixelLayer(layer,x+1,y-1,&red,&green,&blue,&alpha);
          dred+=red;
          dgreen+=green;
          dblue+=blue;
          dalpha+=alpha;
          cnt++;
        }
      }
      /* right */
      if (x+1<layer->fb->width) {
        sil_getPixelLayer(layer,x+1,y,&red,&green,&blue,&alpha);
        dred+=red;
        dgreen+=green;
        dblue+=blue;
        dalpha+=alpha;
        cnt++;
        /* bottomright */
        if (y+1<layer->fb->height) {
          sil_getPixelLayer(layer,x+1,y+1,&red,&green,&blue,&alpha);
          dred+=red;
          dgreen+=green;
          dblue+=blue;
          dalpha+=alpha;
          cnt++;
        }
      }
      /* bottom */
      if (y+1<layer->fb->height) {
        sil_getPixelLayer(layer,x,y+1,&red,&green,&blue,&alpha);
        dred+=red;
        dgreen+=green;
        dblue+=blue;
        dalpha+=alpha;
        cnt++;
        /* bottomleft */
        if (x-1>0) {
          sil_getPixelLayer(layer,x-1,y+1,&red,&green,&blue,&alpha);
          dred+=red;
          dgreen+=green;
          dblue+=blue;
          dalpha+=alpha;
          cnt++;
        }
      }
      /* left */
      if (x-1>0) {
        sil_getPixelLayer(layer,x-1,y,&red,&green,&blue,&alpha);
        dred+=red;
        dgreen+=green;
        dblue+=blue;
        dalpha+=alpha;
        cnt++;
        /* topleft */
        if (y-1>0) {
          sil_getPixelLayer(layer,x-1,y-1,&red,&green,&blue,&alpha);
          dred+=red;
          dgreen+=green;
          dblue+=blue;
          dalpha+=alpha;
          cnt++;
        }
      }
      red=dred/cnt;
      green=dgreen/cnt;
      blue=dblue/cnt;
      alpha=dalpha/cnt;
      sil_putPixelFB(dest,x,y,red,green,blue,alpha); 
    }
  }
  /* swap framebuffers and remove the old one */
  if (layer->fb->buf) free(layer->fb->buf);
  layer->fb->buf=dest->buf;
  
  sil_setErr(err);
  return err;
}

UINT sil_borderFilter(SILLYR *layer ) {
  UINT err=SILERR_ALLOK;

#ifndef SIL_LIVEDANGEROUS
  err=precheck(layer);
  if (SILERR_ALLOK!=err) return err;
#endif

  for (int x=0;x<layer->fb->width;x++) {
    sil_putPixelLayer(layer,x,0,255*(x%2),255*(x%2),255*(x%2),255);
    sil_putPixelLayer(layer,x,layer->fb->height-1,255*(x%2),255*(x%2),255*(x%2),255);
  }
  for (int y=0;y<layer->fb->height;y++) {
    sil_putPixelLayer(layer,0,y,255*(y%2),255*(y%2),255*(y%2),255);
    sil_putPixelLayer(layer,layer->fb->width-1,y,255*(y%2),255*(y%2),255*(y%2),255);
  }

  sil_setErr(err);
  return err;
}

UINT sil_alphaFirstpixelFilter(SILLYR *layer ) {
  UINT err=SILERR_ALLOK;
  BYTE red,green,blue,alpha;
  BYTE red2,green2,blue2,alpha2;


#ifndef SIL_LIVEDANGEROUS
  err=precheck(layer);
  if (SILERR_ALLOK!=err) return err;
#endif

  sil_getPixelLayer(layer,0,0,&red,&green,&blue,&alpha); 
  for (int x=0;x<layer->fb->width;x++) {
    for (int y=0;y<layer->fb->height;y++) {
      sil_getPixelLayer(layer,x,y,&red2,&green2,&blue2,&alpha2); 
      if ((green==green2)&&(red==red2)&&(blue==blue2)) {
        sil_putPixelLayer(layer,x,y,red,green,blue,0); 
      }
    }
  }
  sil_setErr(err);
  return err;
}

UINT sil_flipxFilter(SILLYR *layer ) {
  UINT err=SILERR_ALLOK;
  BYTE red,green,blue,alpha;
  BYTE red2,green2,blue2,alpha2;

#ifndef SIL_LIVEDANGEROUS
  err=precheck(layer);
  if (SILERR_ALLOK!=err) return err;
#endif

  for (int x=0;x<layer->fb->width;x++) {
    for (int y=0;y<(layer->fb->height)/2;y++) {
      sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha); 
      sil_getPixelLayer(layer,x,(layer->fb->height)-y-1,&red2,&green2,&blue2,&alpha2); 
      sil_putPixelLayer(layer,x,y,red2,green2,blue2,alpha2); 
      sil_putPixelLayer(layer,x,(layer->fb->height)-y-1,red,green,blue,alpha); 
    }
  }

  sil_setErr(err);
  return err;
}

UINT sil_flipyFilter(SILLYR *layer ) {
  UINT err=SILERR_ALLOK;
  BYTE red,green,blue,alpha;
  BYTE red2,green2,blue2,alpha2;

#ifndef SIL_LIVEDANGEROUS
  err=precheck(layer);
  if (SILERR_ALLOK!=err) return err;
#endif

  for (int x=0;x<(layer->fb->width)/2;x++) {
    for (int y=0;y<layer->fb->height;y++) {
      sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha); 
      sil_getPixelLayer(layer,(layer->fb->width)-x-1,y,&red2,&green2,&blue2,&alpha2); 
      sil_putPixelLayer(layer,x,y,red2,green2,blue2,alpha2); 
      sil_putPixelLayer(layer,(layer->fb->width)-x-1,y,red,green,blue,alpha); 
    }
  }
  sil_setErr(err);
  return err;
}

UINT sil_rotateColorFilter(SILLYR *layer ) {
  UINT err=SILERR_ALLOK;
  BYTE red,green,blue,alpha;

#ifndef SIL_LIVEDANGEROUS
  err=precheck(layer);
  if (SILERR_ALLOK!=err) return err;
#endif

  for (int x=0;x<layer->fb->width;x++) {
    for (int y=0;y<layer->fb->height;y++) {
      sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha); 
      sil_putPixelLayer(layer,x,y,green,blue,red,alpha); 
    }
  }

  sil_setErr(err);
  return err;
}

UINT sil_reverseColorFilter(SILLYR *layer ) {
  UINT err=SILERR_ALLOK;
  BYTE red,green,blue,alpha;

#ifndef SIL_LIVEDANGEROUS
  err=precheck(layer);
  if (SILERR_ALLOK!=err) return err;
#endif

  for (int x=0;x<layer->fb->width;x++) {
    for (int y=0;y<layer->fb->height;y++) {
      sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha); 
      sil_putPixelLayer(layer,x,y,255-red,255-green,255-blue,alpha); 
    }
  }

  sil_setErr(err);
  return err;
}

UINT sil_grayFilter(SILLYR *layer ) {
  UINT err=SILERR_ALLOK;
  BYTE red,green,blue,alpha;

#ifndef SIL_LIVEDANGEROUS
  err=precheck(layer);
  if (SILERR_ALLOK!=err) return err;
#endif

  for (int x=0;x<layer->fb->width;x++) {
    for (int y=0;y<layer->fb->height;y++) {
      sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha); 
      red=0.21*(float)red+0.71*(float)green+0.07*(float)blue;
      green=red;
      blue=red;
      sil_putPixelLayer(layer,x,y,red,green,blue,alpha); 
    }
  }

  sil_setErr(err);
  return err;
}

/*****************************************************************************

 *****************************************************************************/
