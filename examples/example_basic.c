#include <stdio.h>
#include "sil.h"

int main() {
  SILLYR *whitelayer,*greenlayer;

  // Initialize SIL; Create a window of 200x200 with title "basic"
  sil_initSIL(200,200,"basic",NULL);
  
  // Create a layer in this window at position 10,10 and dimensions 60x60
  whitelayer=sil_addLayer(10,10,60,60,SILTYPE_ABGR);

  // paint layer white, no transparancy 
  sil_paintLayer(whitelayer,SILCOLOR_WHITE,255);

  // Create another layer in this window at position 40,40 and dimensions 100x100
  greenlayer=sil_addLayer(40,40,100,100,SILTYPE_ABGR);

  // paint layer green, 50% transparancy  
  sil_paintLayer(greenlayer,SILCOLOR_GREEN,127);

  // update display (window in this case)
  sil_updateDisplay();

  // Wait untill an event makes it quit, like alt+F4 or closing window
  sil_mainLoop();

  // Cleanup
  sil_destroySIL();
}
