/* fbdisplay.c CopyRight 2021 Remco Schellekens, see LICENSE for more details.*/
/* just a place holder for different display options and documentation        */


#ifdef SIL_TARGET_NONE

/* Group: About displays */
/*

About: Driving the display

  A *display* is anything that is able to output a self-constructed image with pixels and to get
  events like mouse of touch back from it. So it can be a "real" touchscreen display, but also 
  a single window of an operating system. Since the combination of operating systems, hardware and 
  used libraries is different, each environment do have its own ..display.c file, but do have the 
  same functions exposed to the rest of SIL. All function calls that are specific for that 
  environment do only live inside that file, to make the rest of SIL as portable as possible. 
  You could see them as crude display drivers...

  Because timers are very operating system and/or library dependend, those functions are also
  included. In some cases, ..display.c files do have more then the standaard functions described
  here, but use those at your own risk and they are not portable to other environments..

  Be aware that information in events are as close to what is really possible. For instance,
  a touchscreen will never send MOUSEWHEEL or BUTTON_RIGHT event, because most touchscreens can
  only register a single touch. They can't send KEY_xxx event either, since there is no keyboard. 
  Some send the latest update of mousecursor for every event, even non-mouse events, some don't, 
  so the only thing we can do is send the last coordinates we know, and the list goes on...

  These are the environments that can be build, by setting the "DEST" value in the makefile, or 
  if you use something else, use the -DSIL_TARGET_xxxxx flag when compiling:

___ Text ____________

  Environment      │ DEST in Makefile │ -DSIL_TARGET_xxx  │ ...display.c file
  ═════════════════╪══════════════════╪═══════════════════╪════════════════════
  Windows native   │ gdi              │ SIL_TARGET_GDI    │ winGDIdisplay.c
  using GDI        │                  │                   │    
  ─────────────────┼──────────────────┼───────────────────┼────────────────────
  Windows using    │ winsdl           │ SIL_TARGET_WINSDL │ SDLdisplay.c
  SDL2             │                  │                   │    
  ─────────────────┼──────────────────┼───────────────────┼────────────────────
  Linux using      │ lnxsdl           │ SIL_TARGET_LNXSDL │ SDLdisplay.c
  SDL2             │                  │                   │    
  ─────────────────┼──────────────────┼───────────────────┼────────────────────
  Mac OSX using    │ macsdl           │ SIL_TARGET_MACSDL │ SDLdisplay.c
  SDL2             │                  │                   │    
  ─────────────────┼──────────────────┼───────────────────┼────────────────────
  Linux using      │ x11              │ SIL_TARGET_X11    │ x11display.c
  X11 / Xlib       │                  │                   │    
  ─────────────────┼──────────────────┼───────────────────┼────────────────────
  Linux/rpi using  │ fb               │ SIL_TARGET_FB     │ lnxFBdisplay.c
  framebuffer +    │                  │                   │    
  touchscreen      │                  │                   │    
  ─────────────────┼──────────────────┼───────────────────┼────────────────────
___

  It isn't that hard to write your own display "driver". If you know how to put
  a pixel on the display, or even better, a complete image, create a SIL framebuffer
  with the dimensons, including colorcoding and pixeldepth, of the display at init 
  and within sil_updateDisplay, call layersToFB to fill this framebuffer with all 
  the pixels of all layers. After that you can send the image out the way you
  want.

  The easiest way to start, is to take lnxFBdisplay.c as "skeleton" and build around
  it.

*/


/* Group: Handling display */
/*
Function: sil_getTypefromDisplay
  Retrieves the "native" RGB type for display. 

Returns:
  RGB type, see <RGB types> for more information about the possible types.

Remarks:
  Usually the less conversion SIL has to do from layers to final framebuffer, 
  the faster, easier and less memory consuming it can be. So, when creating 
  layers on your own, it is the best to use the same RGB type as display, 
  hence this function to retrieve it.

  Keep in mind that display RGB type might not support alpha, but layers can
  still make use of it.
  


*/
UINT sil_getTypefromDisplay() { }


/*
Function: sil_initDisplay
  Initialize display.

Parameters:
  hI     - Only for windows+GDI: hInstance of WinMain Call, ignored for others
  width  - Width of "display", only if display dimensions are variable, like a window
  height - Height of "display", only if display dimensions are variable, like a window
  title  - If "display" is a window, the title of the window will be set to this
  
Remarks:
  - Only for reference, don't call this function directly, it is called via <sil_initSIL()>.
  - Compile SDL width flag DSL_MAIN_HANDLED, and it doesn't require passing hInstance..

*/
UINT sil_initDisplay(void *hI, UINT width, UINT height, char *title) { }

/*
Function: sil_updateDisplay
  Update Display with latest layer information

Remarks:
  - It's better to "batch" updates on layers, like drawing, moving and so on, 
    together before calling sil_updateDisplay. This will save costly time.
  - A lot of SIL functions do call sil_updateDisplay themselves. When processing 
    a handler call, you can tell SIL to update display by returning a positive value 
  
*/
void sil_updateDisplay() {}

/*
Function: sil_destroyDisplay
  Destroy display information and frees memory
  
Remarks:
  - Only for reference, don't call this function directly, it is called via 
  <sil_destroySIL()>.

 */
void sil_destroyDisplay() { }

/*
Function: sil_getEventDisplay()
  Get event from display

Returns: 
  Pointer to event structure:

  type      - Type of event
  val       - More information about event, like which mouse button is pressed
  x,y       - (last known) position of mouse at that moment
  dx,dy     - If mouse is moved, difference between last known position and current.
  code      - 'native' keycode for keyevents (in case of windows: "virtual keycode")
  key       - keycode translated to 'common' SIL keycodes (see <keyboard codes>)
  modifiers - Flags of modifiers, joined together with or, see <Key modifiers>

  See, <sil_setHoverHandler()> , <sil_setClickHandler()> , <sil_setDragHandler()> 
  for mouse/touch events, <sil_setKeyHandler()> for keyevents, 
  and <sil_setTimerHandler()> for timer events.

Remarks:
  - This call is blocking until event comes in. 
  - To prevent blocking forever, you can set a timer (via <sil_setTimerHandler()> and 
    <sil_setTimeval()>, which will create an event on its own, or use a seperate 
    thread to handle other things.
  - <sil_mainLoop()> will keep calling this function and dispatching events 
    until <sil_quitLoop()>. 
  - Some event cannot be handled by program, but will be handled by operating system, 
    like alt+F4 and closing the window
  - Keep in mind that x,y coordinates are *ints and can be negative*. Although it 
    seems illogical, mouse pointer can go outside boundaries of "display", like pressing
    button inside window, but releasing it outside the window.
  - Note that keycodes are -trying to- refferring to the pressed key, 
    not the ascii or unicode code that is printed on it. To figure out if 'a'
    or 'A' is typed , check for keycode 'a' + modifiers "Shift" or just get the
   'val', where we let the OS trying to translate keycode to ascii.

*/
SILEVENT *sil_getEventDisplay() {}

/* Group: Timers */

/*
Function: sil_setTimerDisplay
  Set timer with given amount of milliseconds

Parameters:
  amount - Milliseconds before timer goes off
  
Remarks:
  - Only for reference, don't call this function directly, it is called via 
    <sil_setTimeval()>
*/
void sil_setTimerDisplay(UINT amount) { }

/*
Function: sil_stopTimerDisplay
  Stop running timer
  
Remarks:
  - Only for reference, don't call this function directly, it is called via 
    <sil_setTimeval()> when amount=0 or within <sil_setTimerHandler()> when
    handler is removed.
*/
void sil_stopTimerDisplay() { }

/* Group: Mouse */

/*
Function: sil_getMouse
  Retrieves the latest coordinates of the mousepointer and status of mouse buttons

Parameters:
  x - Pointer to int for returning x coordinate, or NULL if not needed
  y - Pointer to int for returning y coordinate, or NULL if not needed

Returns:
  x,y coordinates (if pointer is not NULL) and single BYTE containing MOUSE_BTN flags,
  see <Mouse codes>.

Remarks:
  It depends on environment if it is able to retrieve the "real" coordinates, or just
  use the "last known location by SIL". For instance, it doesn't make sense when using
  a touchscreen or pencil that isn't on the display the whole time.
*/
BYTE sil_getMouse(int *x,int *y) { }

/*
Function: sil_setCursor

Parameters: 
  type - Mousecursor type (see <Mouse Cursors>)

*/
void sil_setCursor(BYTE type) { }
#endif

#ifdef SIL_TARGET_WINSDL
#include "SDLdisplay.c"
#endif

#ifdef SIL_TARGET_LNXSDL
#include "SDLdisplay.c"
#endif

#ifdef SIL_TARGET_MACSDL
#include "SDLdisplay.c"
#endif

#ifdef SIL_TARGET_X11
#include "x11display.c"
#endif

#ifdef SIL_TARGET_GDI
#include "winGDIdisplay.c"
#endif

#ifdef SIL_TARGET_FB
#include "lnxFBdisplay.c"
#endif

