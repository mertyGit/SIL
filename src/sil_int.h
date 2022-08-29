#ifndef SILINT_H
#define SILINT_H
#include "sil.h"

/* layer.c */

/* bitmask for internal */
#define SILFLAG_ALPHACHANGED   1
#define SILFLAG_KEYEVENT       2
#define SILKT_SINGLE           4
#define SILKT_ONLYUP           8
#define SILFLAG_INSTANCIATED  16

SILLYR *sil_findHighestClick(UINT,UINT);
SILLYR *sil_findHighestHover(UINT,UINT);
SILLYR *sil_findHighestKeyPress(UINT,BYTE);
void sil_LayersToFB(SILFB *);

#endif
