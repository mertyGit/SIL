/*

   layer.c CopyRight 2021 Remco Schellekens, see LICENSE for more details.

   This file contains all functions for creating and manipulation of layers

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lodepng.h"
#include "sil.h"
#include "log.h"


/* head & tail of linked list of layers */
static SILLYR *top=NULL;
static SILLYR *bottom=NULL;

/* unique identifiers for layers (not used at the moment) */
static UINT idcount=1;

/*****************************************************************************
  Create a layer and it it to linked list of layers on top
  In: display context
      width x height of layer, 
      x,y  position of layer relative to display (top left)
      type = RGB type (SILTYPE_...) 
  Out: pointer to created layer or NULL if error occured (if so ; check 
       sil_getErr for more details)

 *****************************************************************************/

SILLYR *sil_addLayer(UINT width, UINT height, UINT relx, UINT rely, BYTE type) {
  SILLYR *layer=NULL;
  UINT err=0;

  layer=calloc(1,sizeof(SILLYR));
  if (NULL==layer) {
    log_info("ERR: Can't allocate memory for addLayer");
    sil_setErr(SILERR_NOMEM);
    return NULL;
  }

  /* if no type is given - '0' - use type of framebuffer of display for best performance */
  /* and maximum colordepth                                                              */
  if (0==type) {
    type=sil_getTypefromDisplay();
  }

  /* create framebuffer for that layer */
  layer->fb=sil_initFB(width, height, type);
  if (NULL==layer->fb) {
    log_info("ERR: Can't create framebuffer for added layer");
    return NULL;
  }

  /* add layer to double linked list of layers */
  layer->next=NULL;
  if (top) {
    top->next=layer;
    layer->previous=top;
  } else {
    bottom=layer;
    layer->previous=NULL;
  }
  top=layer;

  /* set the other parameters */
  layer->view.minx=0;
  layer->view.miny=0;
  layer->view.maxx=width;
  layer->view.maxy=height;
  layer->relx=relx;
  layer->rely=rely;
  layer->alpha=1;
  layer->flags=0;
  layer->id=idcount++;
  layer->texture=NULL;
  layer->init=1;

  sil_setErr(SILERR_ALLOK);
  return layer;
}

/*****************************************************************************
  Move layer to new position, relative to current position

  In: x and y amount (can be negative) 
      you can place a layer outside dimensions of display

 *****************************************************************************/

void sil_moveLayer(SILLYR *layer,int x,int y) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("moving a layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
  layer->relx+=x;
  layer->rely+=y;
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************
  Place layer on given position

  In: x,y coordinates (top left of display = 0,0)
      you can place a layer outside dimensions of display

 *****************************************************************************/
void sil_placeLayer(SILLYR *layer, UINT x,UINT y) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("placing a layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
  layer->relx=x;
  layer->rely=y;
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************
  Draw a pixel on given location inside a layer

  In: layer context, x,y position
      RGBA color values

 *****************************************************************************/

void sil_putPixelLayer(SILLYR *layer, UINT x, UINT y, BYTE red, BYTE green, BYTE blue, BYTE alpha) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("putPixel on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
  /* don't draw if outside of dimensions of framebuffer */
  if ((x >= layer->fb->width)||(y >= layer->fb->height)) return;
  sil_putPixelFB(layer->fb, x,y,red,green,blue,alpha);
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************
  Draw a pixel on given location inside a layer, but uses alpha value to 
  blend pixel with pixel already on that position

  In: layer context, x,y position
      RGBA color values

 *****************************************************************************/

void sil_blendPixelLayer(SILLYR *layer, UINT x, UINT y, BYTE red, BYTE green, BYTE blue, BYTE alpha) {
  BYTE mixred,mixgreen,mixblue,mixalpha;
  float af,negaf;

  if (0==alpha) return; /* nothing to do */
  if (alpha<255) { 
    /* only calculate when its less then 100% opaque */
    sil_getPixelLayer(layer,x,y,&mixred,&mixgreen,&mixblue,&mixalpha);
    af=((float)alpha)/255;
    negaf=1-af;
    red=red*af+negaf*mixred;
    green=green*af+negaf*mixgreen;
    blue=blue*af+negaf*mixblue;
  }
  sil_putPixelLayer(layer,x,y,red,green,blue,255);
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************
  Draw a pixel on given location inside a layer, but uses alpha value to 
  blend pixel with pixel already on that position

  In: layer context, x,y position
      RGBA color values

 *****************************************************************************/
void sil_getPixelLayer(SILLYR *layer, UINT x, UINT y, BYTE *red, BYTE *green, BYTE *blue, BYTE *alpha) {
  if ((layer) && (layer->init)) {
    /* just return transparant black when outside of fb dimensions */
    if ((x >= layer->fb->width)||(y >= layer->fb->height)) {
      *red=0;
      *green=0;
      *blue=0;
      *alpha=0;
    } else {
      sil_getPixelFB(layer->fb,x,y,red,green,blue,alpha);
    }
  }
  sil_setErr(SILERR_ALLOK);
}

void sil_setFlagsLayer(SILLYR *layer,BYTE flags) {
  layer->flags|=flags;
  sil_setErr(SILERR_ALLOK);
}

void sil_clearFlagsLayer(SILLYR *layer,BYTE flags) {
  layer->flags^=flags;
  sil_setErr(SILERR_ALLOK);
}

UINT sil_checkFlagsLayer(SILLYR *layer,BYTE flags) {
  sil_setErr(SILERR_ALLOK);
  if (layer->flags&flags) return 1;
  return 0;
}

SILLYR *sil_getBottomLayer() {
  sil_setErr(SILERR_ALLOK);
  return bottom;
}

SILLYR *sil_getTopLayer() {
  sil_setErr(SILERR_ALLOK);
  return top;
}

void sil_destroyLayer(SILLYR *layer) {
  if ((layer)&&(layer->init)) {
    sil_destroyFB(layer->fb);
  }
  sil_setErr(SILERR_ALLOK);
}

void sil_setAlphaLayer(SILLYR *layer, float alpha) {
  if (alpha>1) alpha=1;
  if (alpha<=0) alpha=0;
  layer->alpha=alpha;
  layer->flags|=SILFLAG_ALPHACHANGED;
  sil_setErr(SILERR_ALLOK);
}

void sil_setViewLayer(SILLYR *layer,UINT minx,UINT miny,UINT maxx,UINT maxy) {
  if (minx>layer->fb->width) minx=layer->fb->width-1;
  if (maxx>layer->fb->width) maxx=layer->fb->width-1;
  if (miny>layer->fb->height) miny=layer->fb->height-1;
  if (maxy>layer->fb->height) maxy=layer->fb->height-1;
  layer->view.minx=minx;
  layer->view.miny=miny;
  layer->view.maxx=maxx;
  layer->view.maxy=maxy;
  sil_setErr(SILERR_ALLOK);
}

void sil_resetViewLayer(SILLYR *layer) {
  layer->view.minx=0;
  layer->view.miny=0;
  layer->view.maxx=layer->fb->width;
  layer->view.maxy=layer->fb->height;
  sil_setErr(SILERR_ALLOK);
}

UINT sil_resizeLayer(SILLYR *layer, UINT minx,UINT miny,UINT maxx,UINT maxy) {
  SILFB *tmpfb;
  BYTE red,green,blue,alpha;
  UINT err=0;

  if (!layer->init) return SILERR_NOTINIT;

  /* no use to create 'negative' sizes... */
  if ((maxx<=minx)||(maxy<=miny)) return SILERR_WRONGFORMAT;

  /* create temporary framebuffer to copy from old one into */
  tmpfb=sil_initFB(maxx-minx,maxy-miny,layer->fb->type);
  if (NULL==tmpfb) {
    log_info("ERR: Can't create temporary framebuffer for resizing");
    return sil_getErr();
  }


  /* copy selected part */
  for (int x=minx;x<maxx;x++) {
    for (int y=miny;y<maxy;y++) {
        sil_getPixelFB(layer->fb,x,y,&red,&green,&blue,&alpha);
        sil_putPixelFB(tmpfb,x-minx,y-miny,red,green,blue,alpha);
    }
  }
  /* throw away old framebuffer */
  free(layer->fb->buf);

  /* and copy info from temp framebuffer in it */
  layer->fb->buf=tmpfb->buf;
  layer->fb->width=tmpfb->width;
  layer->fb->height=tmpfb->height;
  layer->fb->type=tmpfb->type;
  layer->fb->size=tmpfb->size;
  layer->view.minx=0;
  layer->view.miny=0;
  layer->view.maxx=tmpfb->width;
  layer->view.maxy=tmpfb->height;
  return 0;
}
