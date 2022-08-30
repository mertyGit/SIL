/*

   layer.c CopyRight 2021 Remco Schellekens, see LICENSE for more details.

   This file contains all functions for creating and manipulation of layers

*/
/* Group: About Layers */
/*
 
About: Using Layers

  Layer are the main elements of SIL. Although "the real thing" is handled
  by framebuffers under every layer and pushed out to the display via platform
  specific display routines, as user you should not be knowing anything about that 
  and just focus on the layer routines and if needed - primitive - drawing, imageloading
  and textdrawing routines. 

  In SIL, we talk about *layers* , *views* on layers and the *display* the combination has
  to be rendered to. The term "display" can be confusing, because initially, SIL was 
  written with embedded systems in mind that don't use a sophisticated operating system
  or windowing system. In this terminology, the final target is "just a single display",
  nothing more, nothing less. However, to run it on operating systems that do have a 
  windowing system, its easier to use that system to display on. Therefore, in SIL,
  "display" is the same as "single window SIL run in". 

  (see layersexplanation.png)

  So, the layer concept is simple. Layers are like multiple images with different 
  dimensions place on each other to form a stack. The image on top of the stack might
  be covering the view on the image below, and that image does the same for the image
  below that image and so on. But there is a twist, layers are rectangular but pixels 
  within that layer can be made a bit transparant, where it will blend with the pixels
  of the layer right under it, or even completely transparant, making it look like the
  layer isn't rectangular anymore, but a cut-out. 

  Of course you can create a layer "masking" parts of the layer under it, but it is much easier
  to set a 'view'. This way, you save a layer and you don't need to load,draw or move the image 
  again, you can set the borders of the part you want to see within the layer. See for
  example the use of views to create an animation using a "spritesheet" in a single 
  layer (<sil_initSpriteSheet()>).

  So a "dumb" display isn't aware of any "layers" or "views", but rendering to a display
  or a window, isn't that hard. On every update, just start with bottom layer, get all 
  pixels within the view of that layer and place them in a framebuffer that is as big as 
  the display or window. continue with next layer above it, and it will overwrite 
  existing pixels (or blend, depending on transparancy of the pixel) at places where 
  this layers "covers". Do this until you reached the top layer. Now, push out the 
  framebuffer to display or window, and you have a fresh update of the whole display.
  Check the LayersToFB function in this file how that is done.

  However, parsing every, visible, layer for every small update is not the fastest way 
  to do so, there are much better algorithms out there to only update what is really
  changed on the display and leave the rest there is. It might not be the best for
  fast games, but good enough for GUI kind of tasks. Also, on most embedded systems,
  the communication to the display can be the bottle neck, not the CPU itselfes.

  If you really want speed and you are using a computer or system that do have a graphical 
  unit , either as graphics card or as part of the CPU, use SDL who can make use of any GPU. 
  In SIL, the SDL display routines don't use "LayersToFB", but translate every layer to 
  a seperate texture that will be loaded into the graphics card memory. 
  Moving, rearranging, showing, hiding will all be done inside the graphics card 
  and only changes to layers, but not the whole screen, has to be pushed to the GPU. 
  The CPU doesn't need to render, the graphic card will render it much, much faster.

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lodepng.h"
#include "sil_int.h"
#include "log.h"

typedef struct _GLYR {
  /* head & tail of linked list of layers */
  SILLYR *top;
  SILLYR *bottom;
  UINT idcount;  /* unique identifiers for layers (not used at the moment) */
} GLYR;

static GLYR gv={NULL,NULL,0}; /* holds all global variables used only within layers.c */


/* Group: Creating and destroying
/* 
Function: sil_addLayer 
  Create a layer and put to top of stack of layers

Parameters: 
  relx   - x position relative to top left of display
  rely   - y position relative to top left of display
  width  - width of layer
  height - height of layer
  type   - RGB type to store (see: <RGB types>). Use '0' to use same type as the one from the display.

Returns:
  pointer to created layer or NULL if error occured

Remarks:
  addLayer adds the layer on top of the stack of existing layers. 
  If you want them somewhere else in the stack, use the "to.." sil functions afterwards:
  <sil_toTop()>,<sil_toBottom()>,<sil_toAbove()>,<sil_toBelow()> or even <sil_swap()>
  

*/
SILLYR *sil_addLayer(int relx, int rely, UINT width, UINT height, BYTE type) {
  SILLYR *layer=NULL;

  layer=calloc(1,sizeof(SILLYR));
  if (NULL==layer) {
    log_info("ERR: Can't allocate memory for addLayer");
    return NULL;
  }

  /* if no type is given - '0' - use type of framebuffer of display for best performance */
  /* and maximum colordepth                                                              */
  if (0==type) type=sil_getTypefromDisplay();

  /* create framebuffer for that layer */
  layer->fb=sil_initFB(width, height, type);
  if (NULL==layer->fb) {
    log_info("ERR: Can't create framebuffer for added layer");
    return NULL;
  }

  /* add layer to double linked list of layers */
  layer->next=NULL;
  if (gv.top) {
    gv.top->next=layer;
    layer->previous=gv.top;
  } else {
    gv.bottom=layer;
    layer->previous=NULL;
  }
  gv.top=layer;

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
  layer->id=gv.idcount++;
  layer->texture=NULL;
  layer->user=NULL;
  layer->hover=NULL;
  layer->click=NULL;
  layer->keypress=NULL;
  layer->drag=NULL;
  layer->pointer=NULL;
  layer->key=0;
  layer->modifiers=0;
  layer->sprite.width=0;
  layer->sprite.height=0;
  layer->sprite.pos=0;

  layer->init=1;
  return layer;
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


/*

Function: sil_addCopy

  creates a new layer, but copies all layer information to new one.The New layer will be placed at given x,y postion

Parameters: 
  layer - pointer to layer to copy
  relx  - x position of new layer relative to top left of display
  rely  - y position of new layer relative to top left of display

Returns:
  pointer to newly created layer

*/
SILLYR *sil_addCopy(SILLYR *layer,int relx,int rely) {
  SILLYR *ret=NULL;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("addCopy on layer that isn't initialized, or with uninitialized FB");
    return NULL;
  }
#endif
  ret=sil_addLayer(relx,rely,layer->fb->width,layer->fb->height,layer->fb->type);
  if (NULL==ret) {
    log_warn("Can't create extra layer for addCopy");
    return NULL;
  }
  memcpy(ret->fb->buf,layer->fb->buf,layer->fb->size);
  copylayerinfo(layer,ret);

  return ret;
}


/* 
Function: sil_addInstance

  Create an extra instance of given layer. New layer will share the same framebuffer 
  as the original, so all drawings and filters on it will be the same as the 
  original, however, the new layer can have different position, view, visability or 
  handlers.

  Use this to save memory when using the same image over and over again, like "tiling"
  a background.

Parameters: 
  layer - pointer to layer to instanciate from
  relx  - x position of new layer relative to top left of display
  rely  - y position of new layer relative to top left of display

Returns:
  pointer to newly created layer

Remarks:
  The only advantage of this function is that it will save memory. However, if you use SDL, 
  every layer -including instanciated ones- will end up having a seperate texture to be send 
  to GPU, therefore, it isn't recommended to use this function for SDL environments. 
  If you just want a copy of an existing layer, use <sil_addCopy()> instead

 */

SILLYR *sil_addInstance(SILLYR *layer,int relx,int rely) {
  SILLYR *ret=NULL;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("addCopy on layer that isn't initialized, or with uninitialized FB");
    return NULL;
  }
#endif
  /* create layer of size 1x1, since fb will be thrown away later */
  ret=sil_addLayer(relx,rely,1,1,layer->fb->type);
  if (NULL==ret) {
    log_warn("Can't create extra layer for addCopy");
    return NULL;
  }
  free(ret->fb);
  ret->fb=layer->fb;

  copylayerinfo(layer,ret);

  /* make sure we dont accidently copy handlers & states */
  ret->internal=0;
  ret->hover=NULL;
  ret->click=NULL;
  ret->keypress=NULL;
  ret->drag=NULL;
  ret->pointer=NULL;
  ret->key=0;
  ret->modifiers=0;
  ret->user=NULL;

  /* set flag to instanciated, preventing throwing away framebuffer */
  /* if there is still a copy of it left                            */
  layer->internal|=SILFLAG_INSTANCIATED;
  ret->internal|=SILFLAG_INSTANCIATED;

  /* For SDL: to be sure, set flag that fb is changed to notify it has to */
  /* render the texture for this layer first                              */
  layer->fb->changed=1;
  layer->fb->resized=1;
  return ret;
}

/*
Function: sil_PNGtoNewLayer

  Creat a new layer based on given PNG filename and place it on location x,y

Parameters:
  filename - name of .png file to be loaded
  x        - x coordinate of new layer
  y        - y coordinate of new layer

Returns:
  pointer to newly created layer. Pointer is NULL if error occured.

Remarks:
  - Width and Height of layer are automatically adjusted to size of png file.
  - Although pngfiles can have different colordepths and bit alignment,
  all layers will be generated with SILTYPE_ABGR, so 1 byte per color + alpha
  - This function is way faster then <PNGintoLayer()> which will load and put in 
  an existing layer. It will not use a temporary framebuffer but will claim 
  the created framebuffer from the lodepng function as part of the layer.
 */
SILLYR *sil_PNGtoNewLayer(char *filename,UINT x,UINT y) {
  SILLYR *layer=NULL;
  BYTE *image =NULL;
  UINT err=0;
  UINT width=0;
  UINT height=0;

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
    if (image) free(image);
    return NULL;
  }
  /* first create layer */
  layer=sil_addLayer(x,y,width,height,SILTYPE_ABGR);
  if (NULL==layer) {
    log_warn("Can't create layer for loaded PNG file");
    if (image) free(image);
    return NULL;
  }

  /* free the framebuffer memory */
  if (!(( layer->fb) && (layer->fb->buf))) {
    log_warn("Created layer for PNG has incorrect or missing framebuffer");
    if (image) free(image);
    return NULL;
  }
  free(layer->fb->buf);

  /* and swap the buf with the loaded image */
  layer->fb->buf=image;
  layer->fb->changed=1;
  layer->fb->resized=1;

  return layer;
}

/*****************************************************************************
  Internal function to check if layer is instanciated and twin(s) of it are
  still around...
  return amount of instances found

 *****************************************************************************/

static int hasInstance(SILLYR *layer) {
  UINT cnt=0;
  SILLYR *search;

  if (0==(layer->internal&SILFLAG_INSTANCIATED)) return 0;
  search=gv.bottom;
  while(search) {
    if ((search != layer)&&(search->internal&SILFLAG_INSTANCIATED)) {
      if (search->fb==layer->fb) cnt++; 
    }
    search=search->next;
  }
  return cnt;
}

/*
Function: sil_destroyLayer
  remove layer from stack and delete it

Remarks:
  If layer flag *SILFLAG_FREEUSER* is set and layer->user is not NULL, it will free 
  any allocated memory pointed by layer->user. 
 
*/
void sil_destroyLayer(SILLYR *layer) {
  if ((layer)&&(layer->init)) {
    if (0==hasInstance(layer)) sil_destroyFB(layer->fb);
    layer->init=0;
    sil_toBottom(layer);
    gv.bottom=layer->next;
    layer->next->previous=NULL;
    if ((layer->flags&SILFLAG_FREEUSER)&&(layer->user)) free(layer->user);
    free(layer);
  } else {
    log_warn("removing non-existing or non-initialized layer");
  }
}

/* Group: Setting handlers */
/*
Function: sil_setHoverHandler
  
  Set handler for "hover", when mousepointer is above the given layer, it will 
  trigger the given hoverhandler. Handlers will receive the event, including 
  targeted layer and should return '0' for "do nothing" or '1' for 
  "update display".

Parameters: 
  layer - layer to attach handler to 
  hover - addresss of the handler function

Remarks:

  - *event->type* can be of:

    * *SILDISP_MOUSE_MOVE*   : Mousepointer is moving within the view (and above visible pixels)
    * *SILDISP_MOUSE_LEFT*   : Mousepointer left the view of the layer
    * *SILDISP_MOUSE_ENTER*  : Mousepointer enters the view of the layer

  - *event->x, event->y* will be the coordinates of the mouse pointer within the layer.
  - *event->dx, event->dy* will be the difference between previous and this movement
  - Like all handlers, setting this handler to NULL will prevent these events send to the layer
  - By default, the function will only triggers if mousepointer is above layer *and* 
    there is a visible pixel. If you set the layer flag for *SILFLAG_MOUSEALLPIX*, it will 
    use all pixels within view, visible or not. However, the layer might be blocking visible 
    pixels of the layers underneath.

*/
void sil_setHoverHandler(SILLYR *layer, UINT (*hover)(SILEVENT *)) {
#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("setting handler on layer that isn't initialized");
    return;
  }
#endif
  layer->hover=hover;
}

/*
Function: sil_setClickHandler
  
  Set handler for "click", when mousepointer is above the given layer AND 
  mousebutton has been pressed, it will trigger the given clickhandler. 
  Handlers will receive the event, including targeted layer and should return '0' 
  for "do nothing" or '1' for "update display".

Parameters: 
  layer - layer to attach handler to 
  click - addresss of the handler function


Remarks: 
  - Only 3 mousebuttons are recognized: 

    * *SIL_BTN_LEFT*
    * *SIL_BTN_MIDDLE* (same as scrollwheel click)
    * *SIL_BTN_RIGHT*
    
  - Mousewheel event is also converted to a "click"

    * *SIL_WHEEL_UP* rotating upwards
    * *SIL_WHEEL_DOWN* rotating downwards

  - *event->type* can be of:

    * *SILDISP_MOUSE_DOWN*  : mousebutton is clicked, *event->val* contains mousebutton code 
    * *SILDISP_MOUSE_UP*    : mousebutton is released, *event->val* contains mousebutton code 
    * *SILDISP_MOUSEWHEEL*  : mousewheel is rotated, *event->val* contains wheel code

  - *event->x,event->y* will contain the current location of the mousepointer within display
    (!), so not within layer,  because mouse-up event might even happen outside of layer

  - *event->val* will contain button/wheel code
  - Like all handlers, setting this handler to NULL will prevent these events send to the layer

Example:

  ( file "example_clickhandler.c" in examples directory )

--- Code
#include <stdio.h>
#include "sil.h"
#include "log.h"

SILLYR *whitelayer,*yellowlayer;

UINT myclick(SILEVENT *event) {
  log_info("---------------------------");
  log_info("GOT Click Event !");
  if (whitelayer==event->layer) {
    log_info("From white rectangle layer");
  } else {
    log_info("From yellow rectangle layer");
  }
  switch (event->type) {
    case SILDISP_MOUSE_DOWN:
      log_info("Mouse Down at %d,%d",event->x,event->y);
      if (event->val&SIL_BTN_LEFT) log_info("LEFT pressed");
      if (event->val&SIL_BTN_MIDDLE) log_info("MIDDLE pressed");
      if (event->val&SIL_BTN_RIGHT) log_info("RIGHT pressed");
      break;
    case SILDISP_MOUSE_UP:
      log_info("Mouse Up at %d,%d",event->x,event->y);
      if (event->val&SIL_BTN_LEFT) log_info("LEFT released");
      if (event->val&SIL_BTN_MIDDLE) log_info("MIDDLE released");
      if (event->val&SIL_BTN_RIGHT) log_info("RIGHT released");
      break;
    case SILDISP_MOUSEWHEEL:
      log_info("Mouse Wheel at %d,%d",event->x,event->y);
      if (SIL_WHEEL_UP==event->val) {
        log_info("Wheel Up");
      } else {
        log_info("Wheel Down");
      }
      break;
  }

  return 0; // Don't need no screen update afterwards
}

int main() {
  sil_initSIL(200,200,"basic example",NULL);

  whitelayer=sil_addLayer(10,10,150,150,SILTYPE_ABGR);
  sil_setForegroundColor(SILCOLOR_WHITE,255);
  sil_setDrawWidth(4);
  sil_drawRectangle(whitelayer,0,0,150,150);

  yellowlayer=sil_addLayer(20,20,150,150,SILTYPE_ABGR);
  sil_setForegroundColor(SILCOLOR_YELLOW,200);
  sil_setDrawWidth(4);
  sil_drawRectangle(yellowlayer,0,0,150,150);

  // link our own click handler to both layers
  sil_setClickHandler(whitelayer,myclick);
  sil_setClickHandler(yellowlayer,myclick);


  sil_updateDisplay();
  sil_mainLoop();
  sil_destroySIL();
}

---


*/
void sil_setClickHandler(SILLYR *layer, UINT (*click)(SILEVENT *)) {
#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("setting handler on layer that isn't initialized");
    return;
  }
#endif
  layer->click=click;
}

/*
Function: sil_setKeyHandler
  
  Set handler for keyboard events. 

Parameters: 
  layer     - layer to attach handler to 
  key       - keycode to trigger on
  modifiers - flags of extra modifiers keys that has to be pressed at same time
  flags     - extra options

Key: 
  A single key from <Keyboard codes>, or '0' to get all key codes (if modifiers is also set to '0')

Modifiers:
  One or more modifierkeys that has to be pressed can be selected concatting with 'or' for 
  example SILKM_SHIFT|SILKM_ALT to trigger keyhandler only when keycode + shift + alt are pressed at the same time

  * *SILKM_SHIFT*
  * *SILKM_ALT*
  * *SILKM_CTRL*
  * *SILKM_CAPS*

  Note that there is no difference between left or right of same modifier type

Flags:

  These can also be combined together with or ('|'):

  * SILKT_SINGLE - Ignore any autorepeat, just send code once
  * SILKT_ONLYUP - Don't send any *SILDISP_KEY_DOWN* event, only the *SILDISP_KEY_UP*


Event:
  Two event types can be send to handler:

  * KEY_DOWN - key has been pressed (or is still being pressed, depending on SILKT_SINGLE flag if 
               you want or don't want those additional events )
  * KEY_UP   - key has been released

  *event->key*       - "native" code translated to one of the <Keyboard codes>, platform independend
  *event->modifiers* - Any special keys pressed at the same time (shift,alt,ctrl and or caps)
  *event->val*       - Will be the "guessed" character, based on modifier key like shift and caps. 
                       It is like event->key but with the right case
  *event->code*      - Will be the "native","scancode" or "raw" keycode that might be different per platform. 
                       This is without "translating" it what really is printed on the keycap;


Remarks:
  - The highest layer with matching key/modifiers will be triggered. 
    By doing so you can "bind" special keycombo's per layer, for instance menu items.
  - Like all handlers, setting this handler to NULL will prevent these events send to the layer
  - The layer doesn't have to be visible to receive keyevents
  - By setting both key and modifiers to zero, you create a "catch all" for all keyboard codes.
    You can choose to have one layer handler handle - or dispatch - all pressed keys to 
    make it less complex
  - Depending on platform, some keycombinations cannot be intercepted and are handled directly 
    by operating system, like ALT-F4 or CTRL-C.
  - Don't use *event->x* or *event->y*; they might be not set at all or lagging. 
    Use <sil_getMouse()> to retrieve to current location of mouse pointer if needed.
  - *event->val* is only one byte, so it will probably misintepret any 'special' UTF-8 keyboard 
    codes longer then one byte. Try to look at *event->code* in these cases
    

 */
void sil_setKeyHandler(SILLYR *layer, UINT key, BYTE modifiers, BYTE flags, UINT (*keypress)(SILEVENT *)) {
#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("setting handler on layer that isn't initialized");
    return;
  }
#endif
  layer->internal&=~(SILKT_ONLYUP|SILKT_SINGLE);
  layer->internal|=(flags&(SILKT_ONLYUP|SILKT_SINGLE));
  layer->keypress=keypress;
  layer->key=key;
  layer->modifiers=modifiers;
}


/*
Function: sil_setDragHandler

  Set handler for "drag". 


Parameters: 
  layer - layer to attach handler to 
  drag  - addresss of the handler function

Remarks:
  Drag is actually combination of 2 events,  mouseclick & move, that will be "translated" by
  SIL to a drag event if there is a drag handler linked to the layer where mouseclick happend.
  The handler will recieve the event with the "proposed" new x,y coordinates of the layer 
  in *event->x* and *event->y* and change since last event in *event->dx* and *event->dy*.
  These will be the target coordinates of the layer within the display, not the current (!).
  Returning 0 will ignores the proposed coordinates, returning 1 will set the layer at 
  the proposed coordinates and updates the display.
  This way you will have a simple way to define "boundaries" for dragging layers or even handle 
  your own "dragging/shifting/moving" algorithm and returning 0.

 */

void sil_setDragHandler(SILLYR *layer, UINT (*drag)(SILEVENT *)) {
#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("setting handler on layer that isn't initialized");
    return;
  }
#endif
  layer->drag=drag;
  sil_setFlags(layer,SILFLAG_DRAGGABLE);
}

/* Group: Moving */
/*
Function: sil_moveLayer

  Move layer to new position, relative to current position

Parameters: 
  layer - Layer to move
  x     - distance to move on X axis (can be negative)
  y     - distance to move on Y axis (can be negative)

Remarks:
  It is possible to move layers outside the boundaries of the display. 
  Of course this will result in the layer not - or partly - being drawn when updating.

 */
void sil_moveLayer(SILLYR *layer,int x,int y) {
#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("moving a layer that isn't initialized");
    return;
  }
#endif
  layer->relx+=x;
  layer->rely+=y;
}

/*
Function: sil_placeLayer

  place layer to new position, relative to upper left corner of display

Parameters: 
  layer - Layer to place 
  x     - x position, relative to upper left corner of display
  y     - y position, relative to upper left corner of display

Remarks:
  - Coordinates can be negative, meaning drawn (partly) "off screen". 
  Same goes for x,y higher then right lower corner.
  - If you want to stop showing layer, it is still faster and easier to use 
    <sil_hide()> and <sil_show()> instead of setting it outside dimensions of display


*/
void sil_placeLayer(SILLYR *layer, int x,int y) {
#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("placing a layer that isn't initialized");
    return;
  }
#endif
  layer->relx=x;
  layer->rely=y;
}

/* Group: Adjusting */
/*
Function: sil_setFlags
  set layer flags

Parameters:

  layer - layer to set flags of 
  flags - one or more flags to set, joined by bitwise "or" ('|')

Flags:

  SILFLAG_INVISIBLE   - don't show layer, don't scan for mouse handlers. 
                        Setting and resetting is the same as <sil_hide()> and <sil_show()>
  SILFLAG_NOBLEND     - don't use blend when drawing text
  SILFLAG_VIEWPOSSTAY - if view is changed of layer, keep the layer at same position
  SILFLAG_MOUSESHIELD - Stop scanning for layers with mouse handlers under this layer
  SILFLAG_MOUSEALLPIX - When scanning for valid mouse handlers, all pixels of this layer 
                        are eligable , instead of only visible ones
  SILFLAG_FREEUSER    - Let SIL free up any userdefined memory pointed by layer->user
                        when layer is destroyed

*/
void sil_setFlags(SILLYR *layer,BYTE flags) {
#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("setFlags on layer that isn't initialized");
    return;
  }
#endif
  layer->flags|=flags;
}

/*
Function: sil_clearFlags
  clear one or more flags from layer

Parameters:

  layer - layer that has the flags
  flags - one or more flags to clear, joined by bitwise "or" ('|')

Flags:

  SILFLAG_INVISIBLE   - don't show layer, don't scan for mouse handlers. 
                        Setting and resetting is the same as <sil_hide()> and <sil_show()>
  SILFLAG_NOBLEND     - don't use blend when drawing text
  SILFLAG_VIEWPOSSTAY - if view is changed of layer, keep the layer at same position
  SILFLAG_MOUSESHIELD - Stop scanning for layers with mouse handlers under this layer
  SILFLAG_MOUSEALLPIX - When scanning for valid mouse handlers, all pixels of this layer 
                        are eligable , instead of only visible ones
  SILFLAG_FREEUSER    - Let SIL free up any userdefined memory pointed by layer->user
                        when layer is destroyed

*/
void sil_clearFlags(SILLYR *layer,BYTE flags) {
#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("clearFlags on layer that isn't initialized");
    return;
  }
#endif
  layer->flags&=~flags;
}

/*
Function: sil_checkFlags
  check if one or more flags are set for given layer

Parameters:

  layer - layer to get flags from
  flags - one or more flags to check on, joined by bitwise "or" ('|')

Flags:

  SILFLAG_INVISIBLE   - don't show layer, don't scan for mouse handlers. 
                        Setting and resetting is the same as <sil_hide()> and <sil_show()>
  SILFLAG_NOBLEND     - don't use blend when drawing text
  SILFLAG_VIEWPOSSTAY - if view is changed of layer, keep the layer at same position
  SILFLAG_MOUSESHIELD - Stop scanning for layers with mouse handlers under this layer
  SILFLAG_MOUSEALLPIX - When scanning for valid mouse handlers, all pixels of this layer 
                        are eligable , instead of only visible ones
  SILFLAG_FREEUSER    - Let SIL free up any userdefined memory pointed by layer->user
                        when layer is destroyed

Returns:
  zero if not all given flags are set, one if they are

*/
UINT sil_checkFlags(SILLYR *layer,BYTE flags) {
#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("checkFlags on layer that isn't initialized");
    return SILERR_NOTINIT;
  }
#endif
  if ((layer->flags&flags)==flags) return 1;
  return 0;
}

/*
Function: sil_setAlphaLayer
  set alpha blending factor for layer

Parameters:

  layer - layer to set alpha value
  alpha - alpha value 0.0 (transparant) to 1.0 (opaque)
  
Remarks:
  - Be aware: Although alpha values in pixel functions do use range from 0 to 255, 
    this one uses 0 to 1.0 as float
  - Setting this value on non-SDL platforms might slow down update proces because all
    pixels have to be calculated seperately. 
 
*/
void sil_setAlphaLayer(SILLYR *layer, float alpha) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("setAlpha on layer that isn't initialized, or with uninitialized FB");
    return;
  }
#endif
  if (alpha>1) alpha=1;
  if (alpha<=0) alpha=0;
  layer->alpha=alpha;
  layer->internal|=SILFLAG_ALPHACHANGED;
}

/*
Function: sil_setView
  Set view of layer

  Despite the dimensions and position of layer, it will only display pixels 
  within this view. Also, all mouse handlers only react on pixels within this view.

  (see layerandview.png)

Paramaters:
  layer  - Layer to set view of
  minx   - x offset from top left of layer
  miny   - y offset from top left of layer
  width  - width of view 
  height - height of view


Remarks:
  - If values given creates a view that doesn't fit in layer, the values will 
    be adjusted to fit maximum width and height.

 */
void sil_setView(SILLYR *layer,UINT minx,UINT miny,UINT width,UINT height) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("setView on layer that isn't initialized, or with uninitialized FB");
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
}

/*
Function: sil_resetView
  Reset view settings of layer to default. Default values will be the same as the 
  surface of the layer in total. So minx,miny at 0 and width&height the same as 
  layer width and height 

Paramaters:
  layer  - Layer to set view of

Remark:
  Usefull after resizing the layer or applying filters that result in resizing.

 */
void sil_resetView(SILLYR *layer) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("resetView on layer that isn't initialized, or with uninitialized FB");
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
}

/*
Function: sil_resizeLayer
  resize the given layer to new dimensions and offset.
  Offset are the distances to add from pixellocation in original layer and
  the one in the resized one. 

Paramaters:
  layer - layer to resize
  minx  - X offset 

Remarks:
  - Using offsets, you are also able to crop or shift the layer. 
  - It only cuts off or add pixels, it doesn't scale the layer. If you want that,
    use <sil_rescale()> function.
  - It will be resizing by creating a new framebuffer and copying
    all pixels (with offset) from old one, so it is time and memory
    consuming to resize it this way. Often, the use of views is a
    better alternative is layer has to be smaller.
  - Old version of resize did reset the view, but this version doesn't. 
    Use <sil_setView()> <sil_resetView()> afterwards to compensate that
    if needed.
  
Returns:
  SILERR_ALLOK (0) when everything wend ok, otherwise errorcode
  

 */
UINT sil_resizeLayer(SILLYR *layer, int minx,int miny,UINT width,UINT height) {
  SILFB *tmpfb;
  BYTE red,green,blue,alpha;
  UINT maxx,maxy;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("resize on layer that isn't initialized, or with uninitialized FB");
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
    return SILERR_NOMEM;
  }


  /* copy selected part */
  maxx=minx+width;
  maxy=miny+height;
  if (maxx>layer->fb->width) maxx=layer->fb->width;
  if (maxy>layer->fb->height) maxy=layer->fb->height;
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
  layer->fb->changed=1;
  layer->fb->resized=1;

  return SILERR_ALLOK;
}

/*
Function: sil_hide
  Hide the given layer.

Parameters:
  layer - Layer to hide

Remarks:
  - Result is the same as setting SILFLAG_INVISIBLE via <sil_setFlags()> 
  - Hidden layers aren't shown on display when updating
  - Hidden layers cannot receive any mouse events
  - Hidden layers can do receive key events, though
*/
void sil_hide(SILLYR *layer) {
  sil_setFlags(layer,SILFLAG_INVISIBLE);
}


/*
Function: sil_show
  Unhide the given layer.

Parameters:
  layer - Layer to show

Remarks:
  - Result is the same as resetting SILFLAG_INVISIBLE via <sil_setFlags()> 
*/
void sil_show(SILLYR *layer) {
  sil_clearFlags(layer,SILFLAG_INVISIBLE);
}
/* Group: Change stack order */
/*
Function: sil_toTop
  Move given layer to the top of the stack of layers

Parameters: 
  layer - Layer to move to top
*/
void sil_toTop(SILLYR *layer) {
  SILLYR *tnext;
  SILLYR *tprevious;

#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("trying to change stacking order on layer that isn't initialized");
    return;
  }
#endif

  /* don't move when already on top */
  if (gv.top==layer) return;

  tnext=layer->next;
  tprevious=layer->previous;
  
  if (tnext) tnext->previous=tprevious;
  if (tprevious) tprevious->next=tnext;
  if (gv.bottom==layer) gv.bottom=tnext;
  layer->previous=gv.top;
  layer->next=NULL;
  gv.top->next=layer;
  gv.top=layer;

}


/*
Function: sil_toBottom
  Move given layer to the top of the stack of layers

Parameters: 
  layer - Layer to move to top
*/
void sil_toBottom(SILLYR *layer) {
  SILLYR *tnext;
  SILLYR *tprevious;

#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("trying to change stacking order on layer that isn't initialized");
    return;
  }
#endif

  /* don't move when already on bottom */
  if (gv.bottom==layer) return;

  tnext=layer->next;
  tprevious=layer->previous;
  
  if (tnext) tnext->previous=tprevious;
  if (tprevious) tprevious->next=tnext;
  if (gv.top==layer) gv.top=tprevious;
  layer->next=gv.bottom;
  layer->previous=NULL;
  gv.bottom->previous=layer;
  gv.bottom=layer;
}


/*
Function: sil_toAbove
  Move given layer just above target layer in stack

Parameters: 
  layer  - Layer to move 
  target - Layer to move on top of
*/
void sil_toAbove(SILLYR *layer,SILLYR *target) {
  SILLYR *lnext;
  SILLYR *lprevious;
  SILLYR *tnext;
  SILLYR *tprevious;
  
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==target)) {
    log_warn("trying to change stacking order on layer that isn't initialized");
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
  
  if (target==gv.top) {
    sil_toTop(layer);
    return;
  }

  if (target==gv.bottom) {
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
    if (layer==gv.top) gv.top=lprevious;
    return;

  }

  /* target->previous == layer is only possible here */
  /* so just swap ...                            */
  sil_swap(layer,target);
  return;

}


/*
Function: sil_toBelow
  Move given layer to the bottom of the stack of layers

Parameters: 
  layer - Layer to move to bottom
*/
void sil_toBelow(SILLYR *layer,SILLYR *target) {
  SILLYR *lnext;
  SILLYR *lprevious;
  SILLYR *tnext;
  SILLYR *tprevious;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==target)) {
    log_warn("trying to change stacking order on layer that isn't initialized");
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

  if (target==gv.top) {
    sil_toTop(layer);
    sil_swap(layer,target);
    return;
  }

  if (target==gv.bottom) {
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
    if (layer==gv.top) gv.top=lprevious;
    return;
  }

  /* target->next == layer is only possible here */
  /* so just swap ...                            */
  sil_swap(layer,target);
  return;
}

/*
Function: sil_swap
  Swap position of two layers in stack with eachother 

Parameters: 
  layer - Layer to swap position with target
  target- Layer to swap position with layer
*/
void sil_swap(SILLYR *layer,SILLYR *target) {
  SILLYR *lnext;
  SILLYR *lprevious;
  SILLYR *tnext;
  SILLYR *tprevious;

#ifndef SIL_LIVEDANGEROUS
  if (NULL==layer) {
    log_warn("trying to change stacking order on layer that isn't initialized");
    return;
  }
#endif

  if (target==layer) {
    /* swap with yourself ? nothing to do */
    return;
  }

  if (gv.top==target) {
    gv.top=layer;
  } else {
    if (gv.top==layer) {
      gv.top=target;
    }
  }
  if (gv.bottom==target) {
    gv.bottom=layer;
  } else {
    if (gv.bottom==layer) gv.bottom=target;
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

/*
Function: sil_getBottom
  get layer on the bottom of the stack

Returns: 
  Pointer to bottom layer (suprise)

Remarks:
  If you want to iterate to all layers, you can do it the best from bottom
  till top, by following the layer->next link
 
*/
SILLYR *sil_getBottom() {
  return gv.bottom;
}

/*
Function: sil_getTop
  get the top most layer 

Returns: 
  Pointer to top layer 

*/
SILLYR *sil_getTop() {
  return gv.top;
}

/* Group: Drawing primitives */

/*
Function: sil_clearLayer
  clear all contents of layer

Parameters:
  layer - layer to clear

Remarks:
  "clearing" in this case is setting all pixels and alpha values to '0' - 
  black with maximum transparency 
*/
void sil_clearLayer(SILLYR *layer) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("clearing layer that isn't initialized or with empty fb");
    return ;
  }
#endif
  sil_clearFB(layer->fb);
}
/*
Function: sil_putPixelLayer
  
  Draw a pixel on given location inside a layer.

Parameters:
  layer  - layer to draw on
  x      - x position, relative from upper left corner of layer
  y      - y position, relative from upper left corner of layer
  red    - amount of red   0..255
  green  - amount of green 0..255
  blue   - amount of blue  0..255
  alpha  - Opacity -> from 0 (not visible) to 255 (no transparancy)

Remarks:
  - you can also used predefined SIL <color codes>, instead of writing 
    ..layer,35,139,34,alpha.. you can use ..layer,SILCOLOR_FOREST_GREEN,alpha....
  - Any pixels that are not within boundaries of layer are not drawn at all
  - If framebuffer type of layer doesn't support alpha value, alpha will be ignored
  - Although it uses 8 bit values for RGB, if framebuffer type of layer uses less bits for 
    each color, they will be adjusted 

*/
void sil_putPixelLayer(SILLYR *layer, UINT x, UINT y, BYTE red, BYTE green, BYTE blue, BYTE alpha) {
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("putPixel on layer that isn't initialized, or with uninitialized FB");
    return;
  }
#endif
  /* don't draw if outside of dimensions of framebuffer */
  if ((x >= layer->fb->width)||(y >= layer->fb->height)) return;
  sil_putPixelFB(layer->fb, x,y,red,green,blue,alpha);
}


/*
Function: sil_blendPixelLayer
  
  Draw a pixel on given location inside a layer and blend it with existing pixel


Parameters:
  layer  - layer to draw on
  x      - x position, relative from upper left corner of layer
  y      - y position, relative from upper left corner of layer
  red    - amount of red   0..255
  green  - amount of green 0..255
  blue   - amount of blue  0..255
  alpha  - Opacity -> from 0 (not visible) to 255 (no transparancy)

Remarks:
  - you can also used predefined SIL <color codes>, instead of writing 
    ..layer,35,139,34,alpha.. you can use ..layer,SILCOLOR_FOREST_GREEN,alpha....
  - Any pixels that are not within boundaries of layer are not drawn at all
  - Although it uses 8 bit values for RGB, if framebuffer type of layer uses less bits for 
    each color, they will be adjusted 
  - This function is in the rare case you want to overwrite pixels or framebuffer/display type
    doesn't support alpha blending. But in other cases, if you want to blend images, it is 
    easier -and much faster !- to have each image in a seperate layer and use alpha settings of 
    layers to blend them

*/
void sil_blendPixelLayer(SILLYR *layer, UINT x, UINT y, BYTE red, BYTE green, BYTE blue, BYTE alpha) {
  BYTE mixred,mixgreen,mixblue,mixalpha;
  float af,negaf;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("blendPixelLayer on layer that isn't initialized, or with uninitialized FB");
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
}

/*
Function: sil_getPixelLayer
  
  get red,green,blue and alpha information for pixel on given location inside a layer.

Parameters:
  layer  - layer to draw on
  x      - x position, relative from upper left corner of layer
  y      - y position, relative from upper left corner of layer
  red    - return amount of red   0..255
  green  - return amount of green 0..255
  blue   - return amount of blue  0..255
  alpha  - return Opacity -> from 0 (not visible) to 255 (no transparancy)

Remarks:
  - you can also used predefined SIL <color codes>, instead of writing 
    ..layer,35,139,34,alpha.. you can use ..layer,SILCOLOR_FOREST_GREEN,alpha....
  - all RGB codes are returned as bytes. If Framebuffer uses fewer bits, they will be translated 
    to 8 bits.
  - If framebuffer type doesn't support alpha value, 255 will be returned.
  - Any pixels that are not within boundaries of layer will be returning zero for all values

*/
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
}

/*****************************************************************************

  Internal function,draw all layers, from bottom till top, into a single 
  Framebuffer mostly used by display functions, updating display framebuffer,
  However can be also be used for making screendumps, testing or generating
  image .png files
  This function can be called from display file. Since SDL wil use textures, 
  and not framebuffer, it is the only one not calling this function.

 *****************************************************************************/

void sil_LayersToFB(SILFB *fb) {
  SILLYR *layer;
  BYTE red,green,blue,alpha;
  BYTE mixred,mixgreen,mixblue,mixalpha;
  float af;
  float negaf;

#ifndef SIL_LIVEDANGEROUS
  if (0==fb->size) {
    log_warn("Trying to merge layers to uninitialized framebuffer");
    return;
  }
#endif

  layer=sil_getBottom();
  sil_clearFB(fb);
  while (layer) {
    if (!(layer->flags&SILFLAG_INVISIBLE)) {
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
            /* lets do our own alpha blending */
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
  
  /* clear changed flags, although they are only use by SDL platform at the moment */
  /* but just to be sure or for further development                                */
  layer=sil_getBottom();
  while(layer) {
    layer->fb->changed=0;
    layer->fb->resized=0;
    layer=layer->next;
  }
}

/*****************************************************************************

  Internal function

  if mousebutton has been clicked, find the highest layer that is right under 
  the mousepointer and 
  - layer is visible
  - has a clickhandler attached to it or is marked "draggable" by 
    either a draghandler or set manually
  - pointer is within view of layer
  - pointer is above a visible, non-transparant, pixel 
    (you can turn this check off via setting of SILFLAG_MOUSEALLPIX)

  if a layer has been found that has the flag SILFLAG_MOUSESHIELD, it will 
  not traverse further down, "shielding" all mouse events to layers under it.
  Generally used for messages/pop-ups mechanisms, to prevent clicking outside
  it.

 *****************************************************************************/
SILLYR *sil_findHighestClick(UINT x,UINT y) {
  SILLYR *layer;
  BYTE red,green,blue,alpha;
  int xl,xr,yt,yb;

  layer=sil_getTop();
  while (layer) {
    if (!(layer->flags&SILFLAG_INVISIBLE)) {
      if ((NULL!=layer->click)||(sil_checkFlags(layer,SILFLAG_DRAGGABLE))) {
        /* calculate boundaries of layers where x,y must fall into */
        xl=layer->relx-(int)(layer->view.minx);
        xr=layer->relx+(int)(layer->view.width);
        yt=layer->rely-(int)(layer->view.miny);
        yb=layer->rely+(int)(layer->view.height);
        if ((x>=xl) && (x<xr) && (y>=yt) && (y<yb)) {
          /* return inmediatly when all pixels within view can be considered as target */
          if (layer->flags&SILFLAG_MOUSEALLPIX) return layer;
          /* otherwise, fetch pixel info and only target if pixel isn't transparant    */
          sil_getPixelLayer(layer,x-(layer->relx)+(layer->view.minx),
              y-(layer->rely)+(layer->view.miny),&red,&green,&blue,&alpha);
          if (alpha>0) return layer;
        }
      }
      /* if we find layer with flag "MOUSESHIELD" , we stop searching */
      /* therefore blocking/shielding any mouseevent for layer under  */
      /* this layer                                                   */
      if (layer->flags&(SILFLAG_MOUSESHIELD)) return NULL;
    }
    layer=layer->previous;
  }
  return NULL;
}

/*****************************************************************************
 
  Internal function

  if mouse has been moved, find the highest layer that is right under 
  the mousepointer and 
  - layer is visible
  - has a hoverhandler attached to it 
  - pointer is within view of layer
  - pointer is above a visible, non-transparant, pixel 
    (you can turn this check off via setting of SILFLAG_MOUSEALLPIX)

  if a layer has been found that has the flag SILFLAG_MOUSESHIELD, it will 
  not traverse further down, "shielding" all mouse events to layers under it.
  Generally used for messages/pop-ups mechanisms, to prevent clicking outside
  it.

 *****************************************************************************/
SILLYR *sil_findHighestHover(UINT x,UINT y) {
  SILLYR *layer;
  BYTE red,green,blue,alpha;
  int xl,xr,yt,yb;

  layer=sil_getTop();
  while (layer) {
    if (!(layer->flags&SILFLAG_INVISIBLE)) {
      if (NULL!=layer->hover) {
        /* calculate boundaries of layers where x,y must fall into */
        xl=layer->relx-(int)(layer->view.minx);
        xr=layer->relx+(int)(layer->view.width);
        yt=layer->rely-(int)(layer->view.miny);
        yb=layer->rely+(int)(layer->view.height);
        if ((x>=xl) && (x<xr) && (y>=yt) && (y<yb)) {
          /* return inmediatly when all pixels within view can be considered as target */
          if (layer->flags&SILFLAG_MOUSEALLPIX) return layer;
          /* otherwise, fetch pixel info and only target if pixel isn't transparant    */
          sil_getPixelLayer(layer,x-(layer->relx)+(layer->view.minx),
              y-(layer->rely)+(layer->view.miny),&red,&green,&blue,&alpha);
          if (alpha>0) return layer;
        }
      }
      /* if we find layer with flag "MOUSESHIELD" , we stop searching */
      /* therefore blocking/shielding any mouseevent for layer under  */
      /* this layer                                                   */

      if (layer->flags&(SILFLAG_MOUSESHIELD)) { return NULL; }
    }
    layer=layer->previous;
  }
  return NULL;
}

/*****************************************************************************
   Internal function
   
   A key has been pressed and find the highest layer that is waiting for the 
   given key and modifiers. 
   Note that all layers - even invisible - can receive keyevents
   handlers who has key=0, modifiers=0 set, will catch all keyevents.

 *****************************************************************************/
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

/* Group: Sprites */

/*
Function: sil_initSpriteSheet

  Initialize given layer as "SpriteSheet", containing different images/frames, hence 
  called 'sprites', in a single image. 
  Every sprite should have the same with and height and distance to eachother in 
  the file.

  See also example 'combined.c' using the dancing bananana spritesheet. In this 
  example hparts=8 and vparts=2

  (see dancingbanana.png)

Parameters:
  layer  - layer containing sprites
  hparts - amount of sprites there are horizontally
  vparts - amount of sprites there are vertically

Remarks:
  - The easiest way to animate using spritesheet is to use <sil_setTimerHandler()> 
    together with <sil_setTimeval()> and each time timer goes off, call <sil_nextSprite> 
    (and set new timer for next call)
  - Function assumes the sprites are spread evenly on the layer. therefore the
    size of each sprite should be layer width divided by hparts x layer height 
    divided by vpart
  - It will start with first sprite (top left), and will continue, when using
    <sil_nextSprite()> from left to right, top to bottom and back to top left.
  - Use <sil_nextSprite()> <sil_prevSprite()> or <sil_setSprite()> to display
    one of the sprites.
  - The "displaying" of the sprite is done via setting the view of the layer to just
    a single sprite within the whole image. So, don't alter view of these kind of 
    layers.

 */
void sil_initSpriteSheet(SILLYR *layer,UINT hparts, UINT vparts) {

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("initSpriteSheet on layer that isn't initialized, or with uninitialized FB");
    return;
  }
#endif
  
  /* calculate dimensions of single part */
  layer->sprite.width=(layer->fb->width)/hparts;
  layer->sprite.height=(layer->fb->height)/vparts;


  /* set view accordingly */
  sil_setSprite(layer,0);
}

/*
Function: sil_nextSprite

  Move view of layer to next sprite (counting from left upper corner to 
  right down corner). If there isn't, sprite will be set to first again

Parameters:
  layer - Sprite layer


 */
void sil_nextSprite(SILLYR *layer) {
  UINT maxpos=0;
#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("setSprite on layer that isn't initialized, or with uninitialized FB");
    return;
  }

  if ((0==layer->sprite.width)||(0==layer->sprite.height)) {
    log_warn("spritesheet isn't initialized, or wrong format");
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

/*
Function: sil_prevSprite

  Move view of layer to previous sprite (counting from left upper corner to 
  right down corner). If there isn't, sprite will be set to last sprite 

Parameters:
  layer - Sprite layer

 */
void sil_prevSprite(SILLYR *layer) {
  UINT maxpos=0;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("setSprite on layer that isn't initialized, or with uninitialized FB");
    return;
  }

  if ((0==layer->sprite.width)||(0==layer->sprite.height)) {
    log_warn("spritesheet isn't initialized, or wrong format");
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

/*
Function: sil_setSprite

  Set view of layer to sprite with index "pos" (counting from 0, left upper 
  corner to right down corner). 

Parameters:
  layer - Sprite layer to use
  pos   - Position

*/
void sil_setSprite(SILLYR *layer,UINT pos) {
  UINT maxpos=0;
  UINT x=0;
  UINT y=0;

#ifndef SIL_LIVEDANGEROUS
  if ((NULL==layer)||(NULL==layer->fb)||(0==layer->fb->size)) {
    log_warn("setSprite on layer that isn't initialized, or with uninitialized FB");
    return;
  }

  if ((0==layer->sprite.width)||(0==layer->sprite.height)) {
    log_warn("spritesheet isn't initialized, or wrong format");
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
