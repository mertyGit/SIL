
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
