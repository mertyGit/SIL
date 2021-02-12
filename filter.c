#include <stdlib.h>
#include <stdio.h>
#include "sil.h"
#include "log.h"

UINT sil_applyFilterLayer(SILLYR *layer, BYTE filter) {
  UINT err=0;
  SILFB *dest;
  UINT cnt;
  int minx,miny,maxx,maxy;
  BYTE red,green,blue,alpha;
  UINT dred,dgreen,dblue,dalpha;
  BYTE red2,green2,blue2,alpha2;
  if (!layer->init) return SILERR_NOTINIT;

  switch(filter) {
    case SILFLTR_LIGHTEN:
      for (int x=0;x<layer->fb->width;x++) {
        for (int y=0;y<layer->fb->height;y++) {
          sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha); 
          if (red<205)  {
            red+=50;
          } else
            red=255;
          if (blue<205) 
            blue+=50;
          else
            blue=255;
          if (green<205) 
            green+=50;
          else
            green=255;
          sil_putPixelLayer(layer,x,y,red,green,blue,alpha); 
        }
      }
      break;

    case SILFLTR_DARKEN:
      for (int x=0;x<layer->fb->width;x++) {
        for (int y=0;y<layer->fb->height;y++) {
          sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha); 
          if (red>50)  {
            red-=50;
          } else
            red=0;
          if (blue>50) 
            blue-=50;
          else
            blue=0;
          if (green>50) 
            green-=50;
          else
            green=0;
          sil_putPixelLayer(layer,x,y,red,green,blue,alpha); 
        }
      }
      break;


    case SILFLTR_BORDER:
      for (int x=0;x<layer->fb->width;x++) {
        sil_putPixelLayer(layer,x,0,255*(x%2),255*(x%2),255*(x%2),255);
        sil_putPixelLayer(layer,x,layer->fb->height-1,255*(x%2),255*(x%2),255*(x%2),255);
      }
      for (int y=0;y<layer->fb->height;y++) {
        sil_putPixelLayer(layer,0,y,255*(y%2),255*(y%2),255*(y%2),255);
        sil_putPixelLayer(layer,layer->fb->width-1,y,255*(y%2),255*(y%2),255*(y%2),255);
      }
      break;

    case SILFLTR_AUTOCROPALPHA:
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
      err=sil_resizeLayer(layer,minx,miny,maxx+1,maxy+1);
      break;
    case SILFLTR_FLIPX:
      for (int x=0;x<layer->fb->width;x++) {
        for (int y=0;y<(layer->fb->height)/2;y++) {
          sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha); 
          sil_getPixelLayer(layer,x,(layer->fb->height)-y-1,&red2,&green2,&blue2,&alpha2); 
          sil_putPixelLayer(layer,x,y,red2,green2,blue2,alpha2); 
          sil_putPixelLayer(layer,x,(layer->fb->height)-y-1,red,green,blue,alpha); 
        }
      }
      break;
    case SILFLTR_FLIPY:
      for (int x=0;x<(layer->fb->width)/2;x++) {
        for (int y=0;y<layer->fb->height;y++) {
          sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha); 
          sil_getPixelLayer(layer,(layer->fb->width)-x-1,y,&red2,&green2,&blue2,&alpha2); 
          sil_putPixelLayer(layer,x,y,red2,green2,blue2,alpha2); 
          sil_putPixelLayer(layer,(layer->fb->width)-x-1,y,red,green,blue,alpha); 
        }
      }
      break;
    case SILFLTR_GRAYSCALE:
      for (int x=0;x<layer->fb->width;x++) {
        for (int y=0;y<layer->fb->height;y++) {
          sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha); 
          red=0.21*(float)red+0.71*(float)green+0.07*(float)blue;
          green=red;
          blue=red;
          sil_putPixelLayer(layer,x,y,red,green,blue,alpha); 
        }
      }
      break;
    case SILFLTR_REVERSECOLOR:
      for (int x=0;x<layer->fb->width;x++) {
        for (int y=0;y<layer->fb->height;y++) {
          sil_getPixelLayer(layer,x,y,&red,&green,&blue,&alpha); 
          sil_putPixelLayer(layer,x,y,255-red,255-green,255-blue,alpha); 
        }
      }
      break;
    case SILFLTR_BLUR:
      /* for this, we need to create a seperate FB in between */

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
      break;
  }
  return err;
}
