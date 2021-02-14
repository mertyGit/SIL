#ifndef SIL_H
#define SIL_H

/* tired of typing "unsigned" everywhere .... */
#define BYTE unsigned char
#define UINT unsigned int

/* sil.c */


/* logging level of all sil functions */
#define SIL_QUIET   1
#define SIL_FATAL   2
#define SIL_ERROR   3
#define SIL_WARNING 4
#define SIL_DEBUG   5

/* general errors */
#define SILERR_ALLOK         0 /* No error                            */
#define SILERR_CANTOPENFILE  1 /* Can't open file                     */
#define SILERR_NOMEM         2 /* can't allocate memory               */
#define SILERR_CANTDECODEPNG 3 /* Can't decode PNG                    */
#define SILERR_NOFILEFOUND   4 /* No Filename in font file found      */
#define SILERR_NOCHARS       5 /* can't find chars in fontfile        */
#define SILERR_WRONGFORMAT   6 /* something wrong during decoding     */
#define SILERR_NOTINIT       7 /* Paramaters that are not initialized */

typedef struct _SILCONTEXT {
  UINT lasterr;
  UINT init;
  
} SILCONTEXT;

UINT sil_initSIL(UINT, UINT, char *, void *,char *,BYTE );
void sil_setErr(UINT);
void sil_destroySIL();
UINT sil_getErr();
const char *sil_err2Txt(UINT errorcode);
  

/* framebuffer.c */
/* using little endian naming convention, since most of the libraries are using the same */

#define SILTYPE_332RGB    1 
#define SILTYPE_332BGR    2 
#define SILTYPE_444RGB    3 
#define SILTYPE_444BGR    4 
#define SILTYPE_555RGB    5 
#define SILTYPE_555BGR    6 
#define SILTYPE_565RGB    7 
#define SILTYPE_565BGR    8 
#define SILTYPE_666RGB    9 
#define SILTYPE_666BGR   10
#define SILTYPE_888RGB   11
#define SILTYPE_888BGR   12
#define SILTYPE_ABGR     13
#define SILTYPE_ARGB     14

typedef struct _SILFB {
  BYTE *buf;
  UINT width;
  UINT height;
  BYTE type;
  UINT size;
  BYTE changed;
} SILFB;


SILFB *sil_initFB(UINT,UINT,BYTE) ;
void sil_putPixelFB(SILFB *,UINT,UINT,BYTE,BYTE,BYTE,BYTE);
void sil_getPixelFB(SILFB *,UINT,UINT,BYTE *,BYTE *,BYTE *,BYTE *);
void sil_clearFB(SILFB *);
void sil_destroyFB(SILFB *);


/* layer.c */

/* maximum number of layers, make sure its lower then max UINT */
#define SILMAXLAYERS 1000

/* bitmask for flags */

#define SILFLAG_ALPHACHANGED   1
#define SILFLAG_INVISIBLE      2
#define SILFLAG_NOBLEND        4

typedef struct _SILBOX {
  UINT minx;
  UINT miny;
  UINT maxx;
  UINT maxy;
} SILBOX;

typedef struct _SILLYR {
  SILFB *fb;
  struct _SILLYR *previous;
  struct _SILLYR *next;
  SILBOX view;
  BYTE init;
  BYTE flags;
  float alpha;
  UINT relx;
  UINT rely;
  UINT id;
  void *texture;
} SILLYR;

SILLYR *sil_addLayer(UINT, UINT, UINT, UINT, BYTE);
void sil_putPixelLayer(SILLYR *, UINT, UINT, BYTE, BYTE, BYTE, BYTE);
void sil_blendPixelLayer(SILLYR *, UINT, UINT, BYTE, BYTE, BYTE, BYTE);
void sil_getPixelLayer(SILLYR *, UINT, UINT, BYTE *, BYTE *, BYTE *, BYTE *);
SILLYR *sil_getBottomLayer();
SILLYR *sil_getTopLayer();
void sil_destroyLayer(SILLYR *);
void sil_setFlagsLayer(SILLYR *,BYTE);
void sil_clearFlagsLayer(SILLYR *,BYTE);
UINT sil_checkFlagsLayer(SILLYR *,BYTE);
void sil_setAlphaLayer(SILLYR *,float);
void sil_setViewLayer(SILLYR *,UINT,UINT,UINT,UINT);
void sil_resetViewLayer(SILLYR *);
UINT sil_resizeLayer(SILLYR *, UINT,UINT,UINT,UINT);
void sil_moveLayer(SILLYR *,int, int);
void sil_placeLayer(SILLYR *,UINT, UINT);
SILLYR *sil_PNGtoNewLayer(char *,UINT,UINT);
void LayersToFB(SILFB *);

/* font.c */

#define SILFONT_MAXWIDTH   256  /* max width of a single line in a .fnt file */

typedef struct _SILFCHAR {
  UINT id;
  UINT x;
  UINT y;
  UINT width;
  UINT height;
  int xoffset;
  int yoffset;
  UINT xadvance;
  UINT page;
  UINT chnl;
} SILFCHAR;

typedef struct _SILFKERNING {
  UINT first;   
  UINT second;  
  int amount;
} SILFKERNING;


typedef struct _SILFONT {
  /* font imnage */
  BYTE *image;
  UINT width;
  UINT height;
  /* Common parameters */
  UINT lineHeight;
  UINT base;
  UINT scaleW;
  UINT scaleH;
  UINT pages;
  UINT packed;
  UINT alphaChnl;
  UINT redChnl;
  UINT greenChnl;
  UINT blueChnl;
  /* amounts of other lines */
  UINT chars;
  UINT kernings;
  /* used for parsing lines */
  char line[SILFONT_MAXWIDTH+1] ;     /* buffer for parsing line                       */
  char scratch[SILFONT_MAXWIDTH+1] ;  /* scratch buffer for parsing values             */
  /* array of character definitions */
  SILFCHAR *cdefs;
  /* array of kerning definitions   */
  SILFKERNING *kdefs;
  /* internal handling parameters */
  float alpha;
  UINT mspace;
} SILFONT;

SILFONT *sil_loadFont(char *);
void sil_getPixelFont(SILFONT *,UINT,UINT,BYTE *,BYTE*, BYTE*, BYTE* );
SILFCHAR *sil_getCharFont(SILFONT *, char);
int sil_getKerningFont(SILFONT *, char, char);
void sil_setAlphaFont(SILFONT *, float);
void sil_destroyFont(SILFONT *);


/* filter.c */

/* filters */
#define SILFLTR_AUTOCROPALPHA  1
#define SILFLTR_AUTOCROPFIRST  2
#define SILFLTR_BLUR           3
#define SILFLTR_GRAYSCALE      4
#define SILFLTR_FLIPX          5
#define SILFLTR_FLIPY          6
#define SILFLTR_REVERSECOLOR   7
#define SILFLTR_BORDER         8
#define SILFLTR_DARKEN         9
#define SILFLTR_LIGHTEN       10
#define SILFLTR_ROTATECOLOR   11


UINT sil_applyFilterLayer(SILLYR *, BYTE);

/* drawing.c */

/* textdrawingflags */

#define SILTXT_NOKERNING        1
#define SILTXT_BLENDLAYER       2
#define SILTXT_MONOSPACE        4
#define SILTXT_KEEPCOLOR        8

UINT sil_PNGintoLayer(SILLYR *,char *, UINT,UINT);
void sil_paintLayer(SILLYR *,BYTE,BYTE,BYTE,BYTE);
void sil_drawTextLayer(SILLYR *,SILFONT *, char *, UINT, UINT, BYTE);
void sil_getBackgroundColor(BYTE *,BYTE *, BYTE *, BYTE *);
void sil_setBackgroundColor(BYTE,BYTE,BYTE,BYTE);
void sil_getForegroundColor(BYTE *,BYTE *, BYTE *, BYTE *);
void sil_setForegroundColor(BYTE,BYTE,BYTE,BYTE);

/* x11display.c  / winSDLdisplay.c / winGDIdisplay.c / lnxdisplay.c */

#define SILDISP_NOTHING     0
#define SILDISP_QUIT        1
#define SILDISP_MOUSE_DOWN  2
#define SILDISP_MOUSE_UP    3
#define SILDISP_KEY_DOWN    4
#define SILDISP_KEY_UP      5
#define SILDISP_NOKEYS      6
#define SILDISP_MOUSE_MOVE  7
#define SILDISP_MOUSEWHEEL  8


typedef struct _SILEVENT {
  BYTE type;
  UINT val;
  UINT x;
  UINT y;
  UINT code;
  UINT key;
  BYTE modifiers;
} SILEVENT;

UINT sil_initDisplay(void *, UINT, UINT ,char *);
void sil_updateDisplay();
void sil_destroyDisplay();
UINT sil_getTypefromDisplay();
SILEVENT *sil_getEventDisplay(BYTE);

/* bitmasks for keymodifiers/special keys */
#define SILKM_SHIFT  1
#define SILKM_ALT    2
#define SILKM_CTRL   4
#define SILKM_CAPS   8

/* keycodes */
#define SILKY_BACK          1
#define SILKY_TAB           2
#define SILKY_ENTER         3
#define SILKY_SHIFT         4
#define SILKY_CTRL          5
#define SILKY_ALT           6
#define SILKY_PAUSE         7
#define SILKY_CAPS          8
#define SILKY_ESC           9
#define SILKY_SPACE        10
#define SILKY_PAGEUP       11
#define SILKY_PAGEDOWN     12
#define SILKY_END          13
#define SILKY_HOME         14
#define SILKY_LEFT         15
#define SILKY_UP           16
#define SILKY_RIGHT        17
#define SILKY_DOWN         18
#define SILKY_PRINTSCREEN  19
#define SILKY_INSERT       20
#define SILKY_DELETE       21
#define SILKY_0            22
#define SILKY_1            23
#define SILKY_2            24
#define SILKY_3            25
#define SILKY_4            26
#define SILKY_5            27
#define SILKY_6            28
#define SILKY_7            29
#define SILKY_8            30
#define SILKY_9            31
#define SILKY_A            32
#define SILKY_B            33
#define SILKY_C            34
#define SILKY_D            35
#define SILKY_E            36
#define SILKY_F            37
#define SILKY_G            38
#define SILKY_H            39
#define SILKY_I            40
#define SILKY_J            41
#define SILKY_K            42
#define SILKY_L            43
#define SILKY_M            44
#define SILKY_N            45
#define SILKY_O            46
#define SILKY_P            47
#define SILKY_Q            48
#define SILKY_R            49
#define SILKY_S            50
#define SILKY_T            51
#define SILKY_U            52
#define SILKY_V            53
#define SILKY_W            54
#define SILKY_X            55
#define SILKY_Y            56
#define SILKY_Z            57
#define SILKY_NUM0         58
#define SILKY_NUM1         59
#define SILKY_NUM2         60
#define SILKY_NUM3         61
#define SILKY_NUM4         62
#define SILKY_NUM5         63
#define SILKY_NUM6         64
#define SILKY_NUM7         66
#define SILKY_NUM8         67
#define SILKY_NUM9         68
#define SILKY_NUMMULTIPLY  69
#define SILKY_NUMPLUS      70
#define SILKY_NUMMINUS     71
#define SILKY_NUMPOINT     72
#define SILKY_NUMSLASH     73
#define SILKY_F1           74
#define SILKY_F2           75
#define SILKY_F3           76
#define SILKY_F4           77
#define SILKY_F5           78
#define SILKY_F6           79
#define SILKY_F7           80
#define SILKY_F8           81
#define SILKY_F9           82
#define SILKY_F10          83
#define SILKY_F11          84
#define SILKY_F12          85
#define SILKY_NUMLOCK      86
#define SILKY_SCROLLLOCK   87
#define SILKY_SEMICOLON    88
#define SILKY_PLUS         89
#define SILKY_COMMA        90
#define SILKY_MINUS        91
#define SILKY_PERIOD       92
#define SILKY_SLASH        93
#define SILKY_ACUTE        94
#define SILKY_OPENBRACKET  95
#define SILKY_BACKSLASH    96
#define SILKY_CLOSEBRACKET 97
#define SILKY_APOSTROPHE   98

#endif 