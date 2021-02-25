/*

   framebuffer.c CopyRight 2021 Remco Schellekens, see LICENSE for more details.

   This file contains all functions for handling of framebuffers and 
   different byte packing methods. Each layer do have a single framebuffer
   and in most cases, a display has a "final" framebuffer all other 
   are written on top of it.

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "log.h"
#include "sil.h"


/*****************************************************************************
  Initialize Framebuffer
  In: width & height of framebuffer + RGB format

  format names are little endian, so lowest value first. choose depending 
  on display and how many colors you need (and/or alpha blending)

  332 = 3bits + 3bits + 2bits               = 1   byte per pixel
  444 = 4bits + 4bits + 4bits               = 1.5 byte per pixel or 3 bytes for 2 pixelx
  555 = 5bits + 5bits + 5bits               = 2   bytes per pixel (1 bit unused)
  565 = 5bits + 6bits + 5bits               = 2   bytes per pixel 
  666 = 6bits + 6bits + 6bits               = 3   bytes per pixel (2 bits unused)
  888 = 8bits + 8bits + 8bits               = 3   bytes per pixel 
  ABGR/ARGB = 8bits + 8bits + 8bits + 8bits = 4   bytes per pixel 

  SILTYPE_EMPTY is used to just to support empty layers with resizable 
  dimensions, used attach eventhandlers to it.

 *****************************************************************************/


SILFB *sil_initFB(UINT width, UINT height, BYTE type) {
  SILFB *fb;
  UINT size=0;

#ifndef SIL_LIVEDANGEROUS

  if ((0==width)||(0==height)||(0==type)) {
      log_warn("can't initialize framebuffer; One or more parameters are zero");
      sil_setErr(SILERR_WRONGFORMAT);
      return NULL;
  }

#endif

  switch(type) {
    case SILTYPE_332RGB:
    case SILTYPE_332BGR:
      size=width*height;
      break;
    case SILTYPE_444RGB:
    case SILTYPE_444BGR:
      size=1+width*height*1.5;
      break;
    case SILTYPE_555RGB:
    case SILTYPE_565RGB:
    case SILTYPE_555BGR:
    case SILTYPE_565BGR:
      size=width*height*2;
      break;
    case SILTYPE_666RGB:
    case SILTYPE_666BGR:
    case SILTYPE_888RGB:
    case SILTYPE_888BGR:
      size=width*height*3;
      break;
    case SILTYPE_ABGR:
    case SILTYPE_ARGB:
      size=width*height*4;
      break;

    case SILTYPE_EMPTY:
      size=1;
    default:
      /* unknown type */
      log_info("ERR: Unknown RGB format given to greate framebuffer: %d",type);
      break;
  }
  if (0==size) {
    sil_setErr(SILERR_WRONGFORMAT);
    return NULL;
  }

  fb=calloc(1,sizeof(SILFB));
  if (NULL==fb) {
    log_info("ERR: Can't allocate memory for framebuffer struct");
    sil_setErr(SILERR_NOMEM);
    return NULL;
  }

  fb->buf=calloc(1,size);
  if (NULL==fb->buf) {
    free(fb);
    log_info("ERR: Can't allocate memory for buffer of framebuffer");
    sil_setErr(SILERR_NOMEM);
    return NULL;
  }
  fb->size=size;
  fb->width=width;
  fb->height=height;
  fb->type=type;
  fb->changed=1;
  sil_setErr(SILERR_ALLOK);
  return fb;
}

/*****************************************************************************
  draws a pixel in FB, 
  For speed purposes, it just overwrites all color and alpha data and therefore 
  doesn't do blending on its own with existing/previous pixels. 

  Alhtough colors & alpha are given as full-byte values, it might be downscaled
  to lower colordepth with or without alpha, depending on framebuffer type

  In: x,y (left corner of Framebuffer is 0,0) , BYTE red/green/blue/alpha values
      Alpha starts from 0 (full-transparent) till 255 (no transparency)      

 *****************************************************************************/

void sil_putPixelFB(SILFB *fb,UINT x,UINT y,BYTE red, BYTE green, BYTE blue, BYTE alpha) {
  int pos=0;
  BYTE *buf=NULL;
  UINT width=0;
  UINT height=0;

#ifndef SIL_LIVEDANGEROUS
  if (NULL==fb) {
    log_warn("trying to putpixel on non-initialized FB ");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
  if (NULL==fb->buf) {
    log_warn("trying to a non-allocated FB buffer ");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
  if (0==fb->size) {
    log_warn("trying to putpixel on zero size size FB buffer ");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif

  /* don't bother drawing outside of buffer area */
  if ((x>=fb->width)||(y>=fb->height)) {
    return;
  }

  buf=fb->buf;
  width=fb->width;
  height=fb->height;

  switch(fb->type) {
    case SILTYPE_EMPTY:
      /* don't do anything */
      break;
    case SILTYPE_332RGB:  
      buf[x+width*y]=(red&0xE0)|((green&0xE0)>>3)|(blue>>6);
      break;
    case SILTYPE_332BGR:  
      buf[x+width*y]=(blue&0xE0)|((green&0xE0)>>3)|(red>>6);
      break;
    case SILTYPE_444BGR:
      pos=x*1.5+width*y*1.5;
      if ((x+width*y)%2) {
        buf[pos]|=((red&0xF0)>>4);
        buf[pos+1]=(green&0xF0)|((blue&0xF0)>>4);
      } else {
        buf[pos]=red&0xF0|((green&0xF0)>>4);
        buf[pos+1]|=blue&0xF0;
      }
      break;
    case SILTYPE_444RGB:
      pos=x*1.5+width*y*1.5;
      if ((x+width*y)%2) {
        buf[pos]|=((blue&0xF0)>>4);
        buf[pos+1]=(green&0xF0)|((red&0xF0)>>4);
      } else {
        buf[pos]=blue&0xF0|((green&0xF0)>>4);
        buf[pos+1]|=red&0xF0;
      }
      break;
    case SILTYPE_555BGR: 
      buf[x*2+  width*y*2]= (red  &0xF8)    |((green&0xE0)>>5);
      buf[x*2+1+width*y*2]=((green&0x18)<<3)|((blue &0xF8)>>2);
      break;
    case SILTYPE_555RGB: 
      buf[x*2+  width*y*2]= (blue &0xF8)    |((green&0xE0)>>5);
      buf[x*2+1+width*y*2]=((green&0x18)<<3)|((red  &0xF8)>>2);
      break;
    case SILTYPE_565BGR: 
      buf[x*2+  width*y*2]= (red  &0xF8)    |((green&0xE0)>>5);
      buf[x*2+1+width*y*2]=((green&0x1C)<<3)| (blue>>3);
      break;
    case SILTYPE_565RGB: 
      buf[x*2+  width*y*2]= (blue &0xF8)    |((green&0xE0)>>5);
      buf[x*2+1+width*y*2]=((green&0x1C)<<3)| (red >>3);
      break;
    case SILTYPE_666BGR:
      buf[x*3+  width*3*y]=red>>2;
      buf[x*3+1+width*3*y]=green>>2;
      buf[x*3+2+width*3*y]=blue>>2;
      break;
    case SILTYPE_666RGB:
      buf[x*3+  width*3*y]=blue>>2;
      buf[x*3+1+width*3*y]=green>>2;
      buf[x*3+2+width*3*y]=red>>2;
      break;
    case SILTYPE_888BGR:
      buf[x*3+  width*3*y]=red;
      buf[x*3+1+width*3*y]=green;
      buf[x*3+2+width*3*y]=blue;
      break;
    case SILTYPE_888RGB:
      buf[x*3+  width*3*y]=blue;
      buf[x*3+1+width*3*y]=green;
      buf[x*3+2+width*3*y]=red;
    case SILTYPE_ABGR:
      buf[x*4+  y*width*4]=red;
      buf[x*4+1+y*width*4]=green;
      buf[x*4+2+y*width*4]=blue;
      buf[x*4+3+y*width*4]=alpha;
      break;
    case SILTYPE_ARGB:
      buf[x*4+  y*width*4]=blue;
      buf[x*4+1+y*width*4]=green;
      buf[x*4+2+y*width*4]=red;
      buf[x*4+3+y*width*4]=alpha;
      break;
  }
  fb->changed=1;
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************

  get pixel values from FB, 

  Although colors & alpha are given as full-byte values, it might be upscaled
  from lower colordepth, depending on framebuffer type
  Will return 0,0,0,0 when outside of framebuffer area

  In: x,y (left corner of Framebuffer is 0,0) , BYTE red/green/blue/alpha values
      Alpha starts from 0 (full-transparent) till 255 (no transparency)      


 *****************************************************************************/

void sil_getPixelFB(SILFB *fb,UINT x,UINT y, BYTE *red, BYTE *green, BYTE *blue, BYTE *alpha) {
  BYTE val1, val2;
  BYTE *buf=NULL;
  UINT width=0;
  UINT height=0;
  int pos=0;

#ifndef SIL_LIVEDANGEROUS
  if (NULL==fb) {
    log_warn("trying to getpixel an non-initialized FB ");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
  if ((0==fb->size)||(NULL==fb->buf)) {
    log_warn("trying to getpixel an zero size or non-allocated FB buffer ");
    sil_setErr(SILERR_NOTINIT);
    return;
  }

  /* don't get pixel outside of buffer area */
  if ((x>=fb->width)||(y>=fb->height)) {
    red=0;
    green=0;
    blue=0;
    alpha=0;
    return;
  }
#endif

  buf=fb->buf;
  width=fb->width;
  height=fb->height;
  *alpha=0;
  switch(fb->type) {
    case SILTYPE_EMPTY:
      *blue =0;
      *green=0;
      *red  =0;
      *alpha=0;
      break;
    case SILTYPE_332RGB: 
      *red   = (buf[x+width*y]   )&0xE0;
      *green = (buf[x+width*y]<<3)&0xE0;
      *blue  = (buf[x+width*y]<<6)&0xC0;
      break;
    case SILTYPE_332BGR: 
      *blue  = (buf[x+width*y]   )&0xE0;
      *green = (buf[x+width*y]<<3)&0xE0;
      *red   = (buf[x+width*y]<<6)&0xC0;
      break;
    case SILTYPE_444RGB:
      pos=x*1.5+width*y*1.5;
      if ((x+width*y)%2) {
        *red   = ((buf[pos]  )&0x0F)<<4;
        *green =  (buf[pos+1])&0xF0;
        *blue  = ((buf[pos+1])&0x0F)<<4;
      } else {
        *red   =  (buf[pos]  )&0xF0;
        *green = ((buf[pos]  )&0x0F)<<4;
        *blue  =  (buf[pos+1])&0xF0;
      }
      break;
    case SILTYPE_444BGR:
      pos=x*1.5+width*y*1.5;
      if ((x+width*y)%2) {
        *blue  = ((buf[pos]  )&0x0F)<<4;
        *green =  (buf[pos+1])&0xF0;
        *red   = ((buf[pos+1])&0x0F)<<4;
      } else {
        *blue  =  (buf[pos]  )&0xF0;
        *green = ((buf[pos]  )&0x0F)<<4;
        *red   =  (buf[pos+1])&0xF0;
      }
      break;
    case SILTYPE_555RGB: 
      val1=buf[x*2+  width*y*2];
      val2=buf[x*2+1+width*y*2];
      *red  =   val1 & 0xF8;
      *green= ((val1 & 0x07)<<5)|((val2 & 0xC0)>>3);
      *blue =  (val2 & 0x3E)<<2;
      break;
    case SILTYPE_555BGR: 
      val1=buf[x*2+  width*y*2];
      val2=buf[x*2+1+width*y*2];
      *blue =   val1 & 0xF8;
      *green= ((val1 & 0x07)<<5)|((val2 & 0xC0)>>3);
      *red  =  (val2 & 0x3E)<<2;
      break;
    case SILTYPE_565RGB: 
      val1=buf[x*2+  width*y*2];
      val2=buf[x*2+1+width*y*2];
      *red  =   val1 & 0xF8;
      *green= ((val1 & 0x07)<<5)|((val2 & 0xE0)>>3);
      *blue =  (val2 & 0x1F)<<3;
      break;
    case SILTYPE_565BGR: 
      val1=buf[x*2+  width*y*2];
      val2=buf[x*2+1+width*y*2];
      *blue =   val1 & 0xF8;
      *green= ((val1 & 0x07)<<5)|((val2 & 0xE0)>>3);
      *red  =  (val2 & 0x1F)<<3;
      break;
    case SILTYPE_666RGB:
      *red  =buf[x*3+  width*y*3]<<2;
      *green=buf[x*3+1+width*y*3]<<2;
      *blue =buf[x*3+2+width*y*3]<<2;
      break;
    case SILTYPE_666BGR:
      *blue =buf[x*3+  width*y*3]<<2;
      *green=buf[x*3+1+width*y*3]<<2;
      *red  =buf[x*3+2+width*y*3]<<2;
      break;
    case SILTYPE_888RGB:
      *red  =buf[x*3+  width*y*3];
      *green=buf[x*3+1+width*y*3];
      *blue =buf[x*3+2+width*y*3];
      break;
    case SILTYPE_888BGR:
      *blue =buf[x*3+  width*y*3];
      *green=buf[x*3+1+width*y*3];
      *red  =buf[x*3+2+width*y*3];
      break;
    case SILTYPE_ABGR:
      *red  =buf[x*4+  width*y*4];
      *green=buf[x*4+1+width*y*4];
      *blue =buf[x*4+2+width*y*4];
      *alpha=buf[x*4+3+width*y*4];
      break;
    case SILTYPE_ARGB:
      *blue =buf[x*4+  width*y*4];
      *green=buf[x*4+1+width*y*4];
      *red  =buf[x*4+2+width*y*4];
      *alpha=buf[x*4+3+width*y*4];
      break;
  }
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************

  Clear Framebuffer (buffer part) by setting all bytes in it to to zero, 
  wiping out all drawn pixels, but keeping the framebuffer in tact.

  In: SILFB framebuffer context

 *****************************************************************************/

void sil_clearFB(SILFB *fb) {
  /* size is used to check for initialization of variables inside FB context */
  if ((fb)&&(fb->size)) {
    memset(fb->buf,0,fb->size);
    sil_setErr(SILERR_ALLOK);
  } else {
    log_warn("trying to clear a non-initialized FB ");
    sil_setErr(SILERR_NOTINIT);
  }
}

/*****************************************************************************
  
  Destroy Framebuffer by releasing allocated memory for framebuffer struct and
  accompanied buffer inside.

  In: SILFB Framebuffer context

 *****************************************************************************/

void sil_destroyFB(SILFB *fb) {
  if (fb) {
    if (fb->size && fb->buf) {
      free(fb->buf);
      sil_setErr(SILERR_ALLOK);
    } else {
      log_warn("trying to destroy an empty FB buffer ");
      sil_setErr(SILERR_NOTINIT);
    }
    free(fb);
    fb=NULL;
  } else {
    log_warn("trying to destroy a non-initialized FB ");
    sil_setErr(SILERR_NOTINIT);
  }
}
