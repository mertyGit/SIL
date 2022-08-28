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
