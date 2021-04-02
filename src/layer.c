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
  layer->view.width=width;
  layer->view.height=height;
  layer->relx=relx;
  layer->rely=rely;
  layer->alpha=1;
  layer->flags=0;
  layer->internal=0;
  layer->id=glyr.idcount++;
  layer->texture=NULL;
  layer->user=NULL;
  layer->hover=NULL;
  layer->click=NULL;
  layer->keypress=NULL;
  layer->drag=NULL;
  layer->key=0;
  layer->modifiers=0;
  layer->prevx=0;
  layer->prevy=0;
  layer->sprite.width=0;
  layer->sprite.height=0;
  layer->sprite.pos=0;

  layer->init=1;
  sil_setErr(SILERR_ALLOK);
  return layer;
}

/*****************************************************************************
  Create a mirror of given layer. New layer will share the same framebuffer 
  as the original, so all drawings and filters on it will be the same as the 
  original, however, the new layer can have different position, view or visability

  In:  Layer to mirror
  Out: Mirrored copy of the Layer

 *****************************************************************************/
SILLYR *sil_mirrorLayer(SILLYR *layer, UINT relx, UINT rely) {
  SILLYR *newlayer=NULL;

  if (NULL==layer) {
    log_warn("mirroring layer that isn't initialized");
    sil_setErr(SILERR_NOTINIT);
    return NULL;
  }

  newlayer=calloc(1,sizeof(SILLYR));
  if (NULL==newlayer) {
    log_info("ERR: Can't allocate memory for addLayer");
    sil_setErr(SILERR_NOMEM);
    return NULL;
  }

  /* copy all information */
  memcpy(newlayer,layer,sizeof(SILLYR));

  /* and set new id & position*/
  newlayer->id=glyr.idcount++;
  newlayer->relx=relx;
  newlayer->rely=rely;

  /* make sure we dont accidently copy handlers & states */
  newlayer->internal=0;
  newlayer->hover=NULL;
  newlayer->click=NULL;
  newlayer->keypress=NULL;
  newlayer->drag=NULL;
  newlayer->key=0;
  newlayer->modifiers=0;
  newlayer->prevx=0;
  newlayer->prevy=0;
  newlayer->user=NULL;

  /* add layer to double linked list of layers */
  newlayer->next=NULL;
  if (glyr.top) {
    glyr.top->next=newlayer;
    newlayer->previous=glyr.top;
  } else {
    glyr.bottom=newlayer;
    newlayer->previous=NULL;
  }
  glyr.top=newlayer;

  sil_setErr(SILERR_ALLOK);
  return newlayer;
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

 Draw a pixel "zoomlevel" times as big as normal, for debugging drawing
 algorithms. Be aware ; x,y are also multiplied by zoomlevel !

 *****************************************************************************/

void sil_putBigPixelLayer(SILLYR *layer, UINT x, UINT y, BYTE red, BYTE green, BYTE blue, BYTE alpha) {
  BYTE lvl=sil_getZoom();
  if (lvl<2) {
    /* width of one or none, so just a "normal" pixel */
    sil_putPixelLayer(layer,x,y,red,green,blue,alpha);
    return;
  }
  for (int cy=0;cy<lvl;cy++) {
    for (int cx=0;cx<lvl;cx++) {
      sil_putPixelLayer(layer,(x*lvl)+cx,(y*lvl)+cy,red,green,blue,alpha);
    }
  }
}


void sil_blendBigPixelLayer(SILLYR *layer, UINT x, UINT y, BYTE red, BYTE green, BYTE blue, BYTE alpha) {
  BYTE lvl=sil_getZoom();
  if (lvl<2) {
    /* width of one or none, so just a "normal" pixel */
    sil_blendPixelLayer(layer,x,y,red,green,blue,alpha);
    return;
  }
  for (int cy=0;cy<lvl;cy++) {
    for (int cx=0;cx<lvl;cx++) {
      sil_blendPixelLayer(layer,(x*lvl)+cx,(y*lvl)+cy,red,green,blue,alpha);
    }
  }
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
  sil_getPixelLayer(layer,x,y,&mixred,&mixgreen,&mixblue,&mixalpha);
  if (mixalpha>0) {
    /* only mix when underlaying pixel doesn't have 0 alpha */
    if (0==alpha) return; /* nothing to do */
    if (alpha<255) {
      /* only calculate when its less then 100% opaque */
      af=((float)alpha)/255;
      negaf=1-af;
      red=red*af+negaf*mixred;
      green=green*af+negaf*mixgreen;
      blue=blue*af+negaf*mixblue;
      if (mixalpha>alpha) alpha=mixalpha;
    }
  }
  sil_putPixelLayer(layer,x,y,red,green,blue,alpha);
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
  Internal function to check if layer is instanciated and twin(s) of it are
  still around...
  return amount of instances found

 *****************************************************************************/

static int hasInstance(SILLYR *layer) {
  UINT cnt=0;
  SILLYR *search;

  if (0==layer->internal&SILFLAG_INSTANCIATED) return 0;
  search=glyr.bottom;
  while(search) {
    if ((search != layer)&&(search->internal&SILFLAG_INSTANCIATED)) {
      if (search->fb==layer->fb) cnt++; 
    }
    search=search->next;
  }
  return cnt;
}

/*****************************************************************************
  Remove layer
 *****************************************************************************/
void sil_destroyLayer(SILLYR *layer) {
  if ((layer)&&(layer->init)) {

    if (0==hasInstance(layer)) sil_destroyFB(layer->fb);

    layer->init=0;
    if (layer==glyr.bottom) {
      glyr.bottom=layer->next;
    } else {
      layer->previous->next=layer->next;
    }
    if (layer->user) free(layer->user);
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
  In: Layer context, x,y postion top left, width and height
 *****************************************************************************/
void sil_setView(SILLYR *layer,UINT minx,UINT miny,UINT width,UINT height) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("setView on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
  if (minx>=layer->fb->width) minx=layer->fb->width-1;
  if (miny>=layer->fb->height) miny=layer->fb->height-1;
  if (width>layer->fb->width) width=layer->fb->width;
  if (height>layer->fb->height) height=layer->fb->height;
  layer->view.minx=minx;
  layer->view.miny=miny;
  layer->view.width=width;
  layer->view.height=height;
  if (sil_checkFlags(layer,SILFLAG_VIEWPOSSTAY)) {
    layer->relx+=minx;
    layer->rely+=miny;
  }
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
  if (sil_checkFlags(layer,SILFLAG_VIEWPOSSTAY)) {
    layer->relx-=layer->view.minx;
    layer->rely-=layer->view.miny;
  } 
  layer->view.minx=0;
  layer->view.miny=0;
  layer->view.width=layer->fb->width;
  layer->view.height=layer->fb->height;
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************
  Resize layer (used for cropping and turning) will create a temporary 
  framebuffer

  In: Layer context x,y top left within layer + width & height of view

 *****************************************************************************/
UINT sil_resizeLayer(SILLYR *layer, UINT minx,UINT miny,UINT width,UINT height) {
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

  /* no use to create 'empty' sizes... */
  if ((0==width)||(0==height)) return SILERR_WRONGFORMAT;

  /* create temporary framebuffer to copy from old one into */
  tmpfb=sil_initFB(width,height,layer->fb->type);
  if (NULL==tmpfb) {
    log_info("ERR: Can't create temporary framebuffer for resizing");
    return sil_getErr();
  }


  /* copy selected part */
  for (int x=minx;x<minx+width;x++) {
    for (int y=miny;y<miny+height;y++) {
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
  layer->view.width=tmpfb->width;
  layer->view.height=tmpfb->height;
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
    if (image) free(image);
    return NULL;
  }

  /* free the framebuffer memory */
  if (!(( layer->fb) && (layer->fb->buf))) {
    log_warn("Created layer for PNG has incorrect or missing framebuffer");
    sil_setErr(SILERR_NOTINIT);
    if (image) free(image);
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
  float af;
  float negaf;
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
     /*
      if (sil_getBottom()==layer) log_debug("Bottom");
      if (sil_getTop()==layer) log_debug("Top");
      log_debug("Processing layer %d",layer->id);
      */
      for (int y=layer->view.miny; y<(layer->view.miny+layer->view.height); y++) {
        for (int x=layer->view.minx; x<(layer->view.minx+layer->view.width); x++) {
          int absx=x+layer->relx-layer->view.minx;
          int absy=y+layer->rely-layer->view.miny;
          int rx=x;
          int ry=y;

          /* prevent drawing outside borders of display */
          if ((absx>fb->width)||(absy>fb->height)) continue;

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
          (x<layer->relx+(layer->view.minx+layer->view.width)) &&
          (y>=layer->rely+layer->view.miny) &&
          (y<layer->rely+(layer->view.miny+layer->view.height))) {
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
          (x<layer->relx+(layer->view.minx+layer->view.width)) &&
          (y>=layer->rely+layer->view.miny) &&
          (y<layer->rely+(layer->view.miny+layer->view.height))) {
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

void sil_initSpriteSheet(SILLYR *layer,UINT hparts, UINT vparts) {
  UINT x=0,y=0;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("initSpriteSheet on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
  
  /* calculate dimensions of single part */
  layer->sprite.width=(layer->fb->width)/hparts;
  layer->sprite.height=(layer->fb->height)/vparts;


  /* set view accordingly */
  sil_setSprite(layer,0);
}


void sil_nextSprite(SILLYR *layer) {
  UINT maxpos=0;
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("setSprite on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return;
  }

  if ((0==layer->sprite.width)||(0==layer->sprite.height)) {
    log_warn("spritesheet isn't initialized, or wrong format");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif

  maxpos=((layer->fb->width/layer->sprite.width) * (layer->fb->height/layer->sprite.height));
  if (layer->sprite.pos < maxpos) {
    layer->sprite.pos++;
  } else {
    layer->sprite.pos=0;
  }
  sil_setSprite(layer,layer->sprite.pos);
}


void sil_prevSprite(SILLYR *layer) {
  UINT maxpos=0;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("setSprite on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return;
  }

  if ((0==layer->sprite.width)||(0==layer->sprite.height)) {
    log_warn("spritesheet isn't initialized, or wrong format");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif

  maxpos=((layer->fb->width/layer->sprite.width) * (layer->fb->height/layer->sprite.height));
  if (layer->sprite.pos>0) {
    layer->sprite.pos--;
  } else {
    layer->sprite.pos=maxpos-1;
  }
  sil_setSprite(layer,layer->sprite.pos);
}

void sil_setSprite(SILLYR *layer,UINT pos) {
  UINT maxpos=0;
  UINT x=0;
  UINT y=0;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("setSprite on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return;
  }

  if ((0==layer->sprite.width)||(0==layer->sprite.height)) {
    log_warn("spritesheet isn't initialized, or wrong format");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif

  maxpos=((layer->fb->width/layer->sprite.width) * (layer->fb->height/layer->sprite.height));
  pos%=maxpos;

  /* calculate position of given part number */
  layer->sprite.pos=pos;
  while(pos) {
    if (x+layer->sprite.width< layer->fb->width) {
      x+=layer->sprite.width;
    } else {
      x=0;
      if (y+layer->sprite.height< layer->fb->height) {
        y+=layer->sprite.height;
      } else {
        y=0;
      }
    }
    pos--;
  }
  sil_setView(layer,x,y,layer->sprite.width,layer->sprite.height);
}

/*****************************************************************************

  Functions to change stacking position of layers 
  toTop    : brings layer to the top of the stack (in front of every layer)
  toBottom : brings layer to the bottom of the stack (behind every layer) 
  toAbove  : brings layer (first argument) just in front of another layer (2nd)
  toBelow  : brings layer (first argument) just behind another layer (2nd)

 *****************************************************************************/

void sil_toTop(SILLYR *layer) {
  SILLYR *tnext;
  SILLYR *tprevious;

#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("trying to change stacking order on layer that isn't initialized");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif

  /* don't move when already on top */
  if (glyr.top==layer) return;

  tnext=layer->next;
  tprevious=layer->previous;
  
  if (tnext) tnext->previous=tprevious;
  if (tprevious) tprevious->next=tnext;
  if (glyr.bottom==layer) glyr.bottom=tnext;
  layer->previous=glyr.top;
  layer->next=NULL;
  glyr.top->next=layer;
  glyr.top=layer;

}

void sil_toBottom(SILLYR *layer) {
  SILLYR *tnext;
  SILLYR *tprevious;

#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("trying to change stacking order on layer that isn't initialized");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif

  /* don't move when already on bottom */
  if (glyr.bottom==layer) return;

  tnext=layer->next;
  tprevious=layer->previous;
  
  if (tnext) tnext->previous=tprevious;
  if (tprevious) tprevious->next=tnext;
  if (glyr.top==layer) glyr.top=tprevious;
  layer->next=glyr.bottom;
  layer->previous=NULL;
  glyr.bottom->previous=layer;
  glyr.bottom=layer;
}

void sil_toAbove(SILLYR *layer,SILLYR *target) {
  SILLYR *lnext;
  SILLYR *lprevious;
  SILLYR *tnext;
  SILLYR *tprevious;
  
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==target)) {
    log_warn("trying to change stacking order on layer that isn't initialized");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif

  /* moveing above yourself ? */
  if (target==layer) return;

  /* don't move when already on position */
  if (layer->previous==target) return;

  tnext=target->next;
  tprevious=target->previous;
  lnext=layer->next;
  lprevious=layer->previous;
  
  if (target==glyr.top) {
    sil_toTop(layer);
    return;
  }

  if (target==glyr.bottom) {
    sil_toBottom(layer);
    sil_swap(layer,target);
    return;
  }

  if (tprevious!=layer) {
    /* they are not connected to each other    */
    /* so it is save to just move the pointers */
    layer->previous=target;
    if (lnext) lnext->previous=lprevious;
    layer->next=tnext;
    if (lprevious) lprevious->next=lnext;
    if (tnext) tnext->previous=layer;
    target->next=layer;
    if (layer==glyr.top) glyr.top=lprevious;
    return;

  }

  /* target->previous == layer is only possible here */
  /* so just swap ...                            */
  sil_swap(layer,target);
  return;

}

void sil_toBelow(SILLYR *layer,SILLYR *target) {
  SILLYR *lnext;
  SILLYR *lprevious;
  SILLYR *tnext;
  SILLYR *tprevious;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==target)) {
    log_warn("trying to change stacking order on layer that isn't initialized");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif

  /* moveing below yourself ? */
  if (target==layer) return;

  /* don't move when already on position */
  if (layer->next==target) return;

  tnext=target->next;
  tprevious=target->previous;
  lnext=layer->next;
  lprevious=layer->previous;

  if (target==glyr.top) {
    sil_toTop(layer);
    sil_swap(layer,target);
    return;
  }

  if (target==glyr.bottom) {
    sil_toBottom(layer);
    return;
  }


  if (tnext!=layer) {
    /* they are not connected to each other    */
    /* so it is save to just move the pointers */
    layer->next=target;
    if (lnext) lnext->previous=lprevious;
    layer->previous=tprevious;
    if (lprevious) lprevious->next=lnext;
    if (tprevious) tprevious->next=layer;
    target->previous=layer;
    if (layer==glyr.top) glyr.top=lprevious;
    return;
  }

  /* target->next == layer is only possible here */
  /* so just swap ...                            */
  sil_swap(layer,target);
  return;
}


void sil_swap(SILLYR *layer,SILLYR *target) {
  SILLYR *lnext;
  SILLYR *lprevious;
  SILLYR *tnext;
  SILLYR *tprevious;

#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("trying to change stacking order on layer that isn't initialized");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif

  if (target==layer) {
    /* swap with yourself ? nothing to do */
    return;
  }

  if (glyr.top==target) {
    glyr.top=layer;
  } else {
    if (glyr.top==layer) {
      glyr.top=target;
    }
  }
  if (glyr.bottom==target) {
    glyr.bottom=layer;
  } else {
    if (glyr.bottom==layer) glyr.bottom=target;
  }
  lnext=layer->next;
  lprevious=layer->previous;
  tnext=target->next;
  tprevious=target->previous;

  if ((lnext!=target)&&(tnext!=layer)) {
    /* they are not connected to each other    */
    /* so it is save to just swap the pointers */
    layer->next      = tnext;
    layer->previous  = tprevious;
    target->next     = lnext;
    target->previous = lprevious;
    if (tnext)     tnext->previous = layer;
    if (lnext)     lnext->previous = target;
    if (tprevious) tprevious->next = layer;
    if (lprevious) lprevious->next = target;
    return;
  }

  /* layer is lower then target ?*/
  if (lnext==target) {
    layer->next       = tnext;
    layer->previous   = target;
    target->next      = layer;
    target->previous  = lprevious;
    if (lprevious) lprevious->next = target;
    if (tnext)     tnext->previous = layer;
    return;
  } 

  /* target is lower then layer      */
  /* (tnext==layer) is only one left */
  target->next       = lnext;
  target->previous   = layer;
  layer->next        = target;
  layer->previous    = tprevious;
  if (tprevious) tprevious->next  = layer;
  if (lnext)     lnext->previous  = target;
}

/*****************************************************************************

  internal function :
    copies the relevant information from one layer to another one

 *****************************************************************************/
static void copylayerinfo(SILLYR *from, SILLYR *to) {
  to->view.minx= from->view.minx; 
  to->view.miny= from->view.miny; 
  to->view.width= from->view.width; 
  to->view.height= from->view.height; 
  if (sil_checkFlags(from,SILFLAG_INVISIBLE)) sil_setFlags(to,SILFLAG_INVISIBLE);
  if (sil_checkFlags(from,SILFLAG_DRAGGABLE)) sil_setFlags(to,SILFLAG_DRAGGABLE);
  if (sil_checkFlags(from,SILFLAG_VIEWPOSSTAY)) sil_setFlags(to,SILFLAG_VIEWPOSSTAY);
  if (from->internal & SILKT_SINGLE) to->internal|=SILKT_SINGLE;
  if (from->internal & SILKT_ONLYUP) to->internal|=SILKT_ONLYUP;
  to->hover= from->hover;
  to->click= from->click;
  to->keypress= from->keypress;
  to->drag= from->drag;
  to->texture=from->texture;
  to->sprite.width= from->sprite.width; 
  to->sprite.height= from->sprite.height; 
  to->sprite.pos= from->sprite.pos; 
}


/*****************************************************************************

  addCopy : creates a new layer, but copies all layer information to new one
            New layer will be placed at given x,y postion

 *****************************************************************************/
SILLYR *sil_addCopy(SILLYR *layer,UINT relx,UINT rely) {
  SILLYR *ret=NULL;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("addCopy on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return NULL;
  }
#endif
  ret=sil_addLayer(layer->fb->width,layer->fb->height,relx,rely,layer->fb->type);
  if (NULL==ret) {
    log_warn("Can't create extra layer for addCopy");
    return NULL;
  }
  memcpy(ret->fb->buf,layer->fb->buf,layer->fb->size);
  copylayerinfo(layer,ret);

  return ret;
}


/*****************************************************************************

  addInstance: 
   creates a new layer, but shares the same framebuffer to save memory.
   New layer will be placed at given x,y postion

 *****************************************************************************/

SILLYR *sil_addInstance(SILLYR *layer,UINT relx,UINT rely) {
  SILLYR *ret=NULL;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("addCopy on layer that isn't initialized, or with uninitialized FB");
    sil_setErr(SILERR_NOTINIT);
    return NULL;
  }
#endif
  /* create layer of size 1x1, since fb will be thrown away later */
  ret=sil_addLayer(1,1,relx,rely,layer->fb->type);
  if (NULL==ret) {
    log_warn("Can't create extra layer for addCopy");
    return NULL;
  }
  free(ret->fb);
  ret->fb=layer->fb;

  copylayerinfo(layer,ret);
  /* set flag to instanciated, preventing throwing away framebuffer */
  /* if there is still a copy of it left                            */
  layer->internal|=SILFLAG_INSTANCIATED;
  ret->internal|=SILFLAG_INSTANCIATED;
  return ret;
}
