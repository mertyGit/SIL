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

typedef struct _GLYR {
  /* head & tail of linked list of layers */
  SILLYR *top;
  SILLYR *bottom;
  UINT idcount;  /* unique identifiers for layers (not used at the moment) */
} GLYR;

static GLYR glyr={NULL,NULL,0}; /* holds all global variables used only within layers.c */


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
  if (glyr.top) {
    glyr.top->next=layer;
    layer->previous=glyr.top;
  } else {
    glyr.bottom=layer;
    layer->previous=NULL;
  }
  glyr.top=layer;

  /* set the other parameters to default */
  layer->view.minx=0;
  layer->view.miny=0;
  layer->view.maxx=width;
  layer->view.maxy=height;
  layer->relx=relx;
  layer->rely=rely;
  layer->alpha=1;
  layer->flags=0;
  layer->internal=0;
  layer->id=glyr.idcount++;
  layer->texture=NULL;
  layer->init=1;

  layer->hover=NULL;
  layer->click=NULL;
  layer->keypress=NULL;
  layer->drag=NULL;
  layer->key=0;
  layer->modifiers=0;
  layer->prevx=0;
  layer->prevy=0;
  sil_setErr(SILERR_ALLOK);
  return layer;
}

void sil_setHoverHandler(SILLYR *layer, UINT (*hover)(SILEVENT *)) {
#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("setting handler on layer that isn't initialized");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
  layer->hover=hover;
}

void sil_setClickHandler(SILLYR *layer, UINT (*click)(SILEVENT *)) {
#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("setting handler on layer that isn't initialized");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
  layer->click=click;
}

void sil_setKeyHandler(SILLYR *layer, UINT key, BYTE modifiers, BYTE flags, UINT (*keypress)(SILEVENT *)) {
#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("setting handler on layer that isn't initialized");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
  layer->internal&=~(SILKT_ONLYUP|SILKT_SINGLE);
  layer->internal|=(flags&(SILKT_ONLYUP|SILKT_SINGLE));
  layer->keypress=keypress;
  layer->key=key;
  layer->modifiers=modifiers;
}

void sil_setDragHandler(SILLYR *layer, UINT (*drag)(SILEVENT *)) {
#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("setting handler on layer that isn't initialized");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
  layer->drag=drag;
  sil_setFlags(layer,SILFLAG_DRAGGABLE);
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

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("blendPixelLayer on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
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

/*****************************************************************************
  Manipulate flags for layers
  SILFLAG_INVISIBLE Don't draw layer
  SILFLAG_NOBLEND   Do not blend with existing colors


 *****************************************************************************/

void sil_setFlags(SILLYR *layer,BYTE flags) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("setFlags on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
  layer->flags|=flags;
  sil_setErr(SILERR_ALLOK);
}

void sil_clearFlags(SILLYR *layer,BYTE flags) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("clearFlags on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
  layer->flags&=~flags;
  sil_setErr(SILERR_ALLOK);
}

UINT sil_checkFlags(SILLYR *layer,BYTE flags) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("checkFlags on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return SILERR_NOTINIT;
  }
#endif
  sil_setErr(SILERR_ALLOK);
  if ((layer->flags&flags)==flags) return 1;
  return 0;
}


/*****************************************************************************
  Get bottom layer
 *****************************************************************************/

SILLYR *sil_getBottom() {
  sil_setErr(SILERR_ALLOK);
  return glyr.bottom;
}

/*****************************************************************************
  Get top most layer 
 *****************************************************************************/

SILLYR *sil_getTop() {
  sil_setErr(SILERR_ALLOK);
  return glyr.top;
}

/*****************************************************************************
  Remove layer
 *****************************************************************************/
void sil_destroyLayer(SILLYR *layer) {
  if ((layer)&&(layer->init)) {
    sil_destroyFB(layer->fb);
    layer->init=0;
    if (layer==glyr.bottom) {
      glyr.bottom=layer->next;
    } else {
      layer->previous->next=layer->next;
    }
    free(layer);
  } else {
    log_warn("removing non-existing or non-initialized layer");
  }
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************
  Set alpha blending for whole layer 
  In: Layer context, alpha (0.0 ... 1.0) 1.0 is no transparency 
 *****************************************************************************/
void sil_setAlphaLayer(SILLYR *layer, float alpha) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("setAlpha on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
  if (alpha>1) alpha=1;
  if (alpha<=0) alpha=0;
  layer->alpha=alpha;
  layer->internal|=SILFLAG_ALPHACHANGED;
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************
  Set view of layer
  Despite the dimensions and position of layer, it will only display pixels 
  within this view. 
  In: Layer context, x,y postion top left, x,y postion bottom right, relative
      to top left position of layer itself
 *****************************************************************************/
void sil_setView(SILLYR *layer,UINT minx,UINT miny,UINT maxx,UINT maxy) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("setView on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
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

/*****************************************************************************
  Reset the view to display the whole layer
  In: Layer context
 *****************************************************************************/
void sil_resetView(SILLYR *layer) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("resetView on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
  layer->view.minx=0;
  layer->view.miny=0;
  layer->view.maxx=layer->fb->width;
  layer->view.maxy=layer->fb->height;
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************
  Resize layer (used for cropping and turning) will create a temporary 
  framebuffer

  In: Layer context x,y top left, x,y bottom right, using the the top left 
      position of the original layer as reference.
 *****************************************************************************/
UINT sil_resizeLayer(SILLYR *layer, UINT minx,UINT miny,UINT maxx,UINT maxy) {
  SILFB *tmpfb;
  BYTE red,green,blue,alpha;
  UINT err=0;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("resetView on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return SILERR_NOTINIT;
  }
#endif
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

SILLYR *sil_PNGtoNewLayer(char *filename,UINT x,UINT y) {
  SILLYR *layer=NULL;
  BYTE *image =NULL;
  UINT err=0;
  UINT pos=0;
  UINT width=0;
  UINT height=0;
  UINT maxwidth=0;
  UINT maxheight=0;
  BYTE red,green,blue,alpha;

  /* load image in framebuffer */
  err=lodepng_decode32_file(&image,&width,&height,filename);
  if ((!err)&&((0==width)||(0==height))) {
    /* doesn't make sense loading a PNG file with no height and/or width */
    log_warn("'%s' appears to have unusual width x height (%d x %d)",filename,width,height);
    err=666; /* will be handled later with err switch */
  }

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
    return NULL;
  }
  /* first create layer */
  layer=sil_addLayer(width,height,x,y,SILTYPE_ABGR);
  if (NULL==layer) {
    log_warn("Can't create layer for loaded PNG file");
    sil_setErr(SILERR_WRONGFORMAT);
    return NULL;
  }

  /* free the framebuffer memory */
  if (!(( layer->fb) && (layer->fb->buf))) {
    log_warn("Created layer for PNG has incorrect or missing framebuffer");
    sil_setErr(SILERR_NOTINIT);
    return NULL;
  }
  free(layer->fb->buf);

  /* and swap the buf with the loaded image */
  layer->fb->buf=image;

  sil_setErr(SILERR_ALLOK);
  return layer;
}

/*****************************************************************************

  draw all layers, from bottom till top, into a single Framebuffer
  Mostly used by display functions, updating display framebuffer,
  However can be also be used for making screendumps, testing or generating
  image .png files

 *****************************************************************************/

void LayersToFB(SILFB *fb) {
  SILLYR *layer;
  BYTE red,green,blue,alpha;
  BYTE mixred,mixgreen,mixblue,mixalpha;
  UINT pos,pos2;
  float af;
  float negaf;
  SILBOX rview;
  int absx,absy;

#ifndef SIL_LIVEDANGEROUS
  if (0==fb->size) {
    log_warn("Trying to merge layers to uninitialized framebuffer");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif

  layer=sil_getBottom();
  sil_clearFB(fb);
  while (layer) {
    if (!(layer->flags&SILFLAG_INVISIBLE)) {
      for (int x=layer->view.minx; x<layer->view.maxx; x++) {
        for (int y=layer->view.miny; y<layer->view.maxy; y++) {
          int absx=x+layer->relx-layer->view.minx;
          int absy=y+layer->rely-layer->view.miny;
          int rx=x;
          int ry=y;

          /* prevent drawing outside borders of display */
          if ((absx>=fb->width)||(absy>=fb->height)) continue;

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
}


SILLYR *sil_findHighestClick(UINT x,UINT y) {
  SILLYR *layer;

  layer=sil_getTop();
  while (layer) {
    if (!(layer->flags&SILFLAG_INVISIBLE)) {
      if (((NULL!=layer->click)||(sil_checkFlags(layer,SILFLAG_DRAGGABLE))) &&
          (x>=layer->relx+layer->view.minx) &&
          (x<=layer->relx+layer->view.maxx) &&
          (y>=layer->rely+layer->view.miny) &&
          (y<=layer->rely+layer->view.maxy)) {
        return layer;
      }
    }
    layer=layer->previous;
  }
  return NULL;
}

SILLYR *sil_findHighestHover(UINT x,UINT y) {
  SILLYR *layer;

  layer=sil_getTop();
  while (layer) {
    if (!(layer->flags&SILFLAG_INVISIBLE)) {
      if ((NULL!=layer->hover) &&
          (x>=layer->relx+layer->view.minx) &&
          (x<=layer->relx+layer->view.maxx) &&
          (y>=layer->rely+layer->view.miny) &&
          (y<=layer->rely+layer->view.maxy)) {
        return layer;
      }
    }
    layer=layer->previous;
  }
  return NULL;
}


SILLYR *sil_findHighestKeyPress(UINT c,BYTE modifiers) {
  SILLYR *layer;

  layer=sil_getTop();
  while (layer) {
    if (NULL!=layer->keypress) {
      if ((layer->key)||(layer->modifiers)) {
        /* layer has a keypress handler, but is it looking for this shortcut key ? */
        if ((c==layer->key) && (modifiers==layer->modifiers)) {
          return layer;
        }
      } else {
        /* no key or modifier set, so its a "catch all" for all keyevents */
        return layer;
      }
    }
    layer=layer->previous;
  }
  return NULL;
}
