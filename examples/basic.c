#include <stdio.h>
#include "sil.h"
#include "log.h"

int main() {
  SILLYR *drawing;

  // Initialize SIL; Create a window of 200x200 with title "basic example"
  sil_initSIL(200,200,"basic example",NULL);
  
  // Create layer in this window at position 10,10 and dimension 150x150
  drawing=sil_addLayer(10,10,150,150,SILTYPE_ABGR);

  // paint background of layer white, no transparancy 
  sil_paintLayer(drawing,SILCOLOR_WHITE,255);
  // set foreground color - in this case the border - to dark green, no transparancy
  sil_setForegroundColor(SILCOLOR_DARK_GREEN,255);
  // set background color - in this fill of rectangle - to green, opacity at 50
  sil_setBackgroundColor(SILCOLOR_GREEN,50);
  // set width of lines (for border) to 4
  sil_setDrawWidth(4);
  // Draw a rectangle inside layer "drawing" at 15,15 and dimension 100x100
  sil_drawRectangle(drawing,15,15,100,100);

  // update display (window in this case)
  sil_updateDisplay();

  // Wait till event makes it quit, like alt+F4 or closing window
  sil_mainLoop();

  // Cleanup
  sil_destroySIL();
}
