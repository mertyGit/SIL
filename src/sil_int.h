#ifndef SILINT_H
#define SILINT_H
#include "sil.h"

/* layer.c */


SILLYR *sil_findHighestClick(UINT,UINT);
SILLYR *sil_findHighestHover(UINT,UINT);
SILLYR *sil_findHighestKeyPress(UINT,BYTE);
void sil_LayersToFB(SILFB *);

#endif
