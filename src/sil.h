#ifndef SIL_H
#define SIL_H

/* tired of typing "unsigned" everywhere .... */
#define BYTE unsigned char
#define UINT unsigned int

/* the useual helpers */
#define SIL_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define SIL_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define SIL_ABS(x) ((x) < 0 ? -(x) : (x))


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

UINT sil_initSIL(UINT, UINT, char *, void *);
UINT sil_setLog(char *logname, BYTE flags);
void sil_setErr(UINT);
void sil_destroySIL();
UINT sil_getErr();
const char *sil_err2Txt(UINT errorcode);
void sil_quitLoop();
void sil_mainLoop();
void sil_setTimeval(UINT);
UINT sil_getTimeval();
  

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
#define SILTYPE_EMPTY    15

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

#define SILFLAG_INVISIBLE      1
#define SILFLAG_NOBLEND        2
#define SILFLAG_BUTTONDOWN     4
#define SILFLAG_DRAGGABLE      8
#define SILFLAG_VIEWPOSSTAY   16
#define SILFLAG_FREEUSER      32

/* bitmask for internal */
#define SILFLAG_ALPHACHANGED   1
#define SILFLAG_KEYEVENT       2
#define SILKT_SINGLE           4
#define SILKT_ONLYUP           8
#define SILFLAG_INSTANCIATED  16

/* also used by display.c */
typedef struct _SILEVENT {
  BYTE type;
  UINT val;
  UINT x;
  UINT y;
  UINT code;
  UINT key;
  BYTE modifiers;
  struct _SILLYR *layer;
} SILEVENT;


typedef struct _SILBOX {
  UINT minx;
  UINT miny;
  UINT width;
  UINT height;
} SILBOX;

typedef struct _SILSPRITE {
  UINT width;
  UINT height;
  UINT pos;
} SILSPRITE;

typedef struct _SILLYR {
  SILFB *fb;
  struct _SILLYR *previous;
  struct _SILLYR *next;
  SILBOX view;
  BYTE init;
  BYTE flags;
  BYTE internal;
  float alpha;
  UINT relx;
  UINT rely;
  UINT id;
  void *texture;
  UINT (*hover   )(SILEVENT *);
  UINT (*click   )(SILEVENT *);
  UINT (*keypress)(SILEVENT *);
  UINT (*drag    )(SILEVENT *);
  UINT key;
  BYTE modifiers;
  UINT prevx;
  UINT prevy;
  SILSPRITE sprite;
  void *user;
} SILLYR;

/* this one is in sil.c, not layer.c but needs SILEVENT defined */
void sil_setTimerHandler(UINT (*)(SILEVENT *));

SILLYR *sil_addLayer(UINT, UINT, UINT, UINT, BYTE);
SILLYR *sil_mirrorLayer(SILLYR *, UINT, UINT);
void sil_putPixelLayer(SILLYR *, UINT, UINT, BYTE, BYTE, BYTE, BYTE);
void sil_blendPixelLayer(SILLYR *, UINT, UINT, BYTE, BYTE, BYTE, BYTE);
void sil_putBigPixelLayer(SILLYR *, UINT, UINT, BYTE, BYTE, BYTE, BYTE);
void sil_blendBigPixelLayer(SILLYR *, UINT, UINT, BYTE, BYTE, BYTE, BYTE);
void sil_getPixelLayer(SILLYR *, UINT, UINT, BYTE *, BYTE *, BYTE *, BYTE *);
SILLYR *sil_getBottom();
SILLYR *sil_getTop();
void sil_destroyLayer(SILLYR *);
void sil_setFlags(SILLYR *,BYTE);
void sil_clearFlags(SILLYR *,BYTE);
UINT sil_checkFlags(SILLYR *,BYTE);
void sil_setAlphaLayer(SILLYR *,float);
void sil_setView(SILLYR *,UINT,UINT,UINT,UINT);
void sil_resetView(SILLYR *);
UINT sil_resizeLayer(SILLYR *, UINT,UINT,UINT,UINT);
void sil_moveLayer(SILLYR *,int, int);
void sil_placeLayer(SILLYR *,UINT, UINT);
SILLYR *sil_PNGtoNewLayer(char *,UINT,UINT);
void LayersToFB(SILFB *);
void sil_setKeyHandler(SILLYR *,UINT, BYTE, BYTE, UINT (*)(SILEVENT *));
void sil_setClickHandler(SILLYR *,UINT (*)(SILEVENT *));
void sil_setHoverHandler(SILLYR *,UINT (*)(SILEVENT *));
void sil_setDragHandler(SILLYR *,UINT (*)(SILEVENT *));
SILLYR *sil_findHighestClick(UINT,UINT);
SILLYR *sil_findHighestHover(UINT,UINT);
SILLYR *sil_findHighestKeyPress(UINT,BYTE);
void sil_initSpriteSheet(SILLYR *,UINT ,UINT);
void sil_nextSprite(SILLYR *);
void sil_prevSprite(SILLYR *);
void sil_setSprite(SILLYR *,UINT);
void sil_toAbove(SILLYR *,SILLYR *);
void sil_toBelow(SILLYR *,SILLYR *);
void sil_toTop(SILLYR *);
void sil_toBottom(SILLYR *);
void sil_swap(SILLYR *,SILLYR *);
SILLYR *sil_addCopy(SILLYR *,UINT,UINT);
SILLYR *sil_addInstance(SILLYR *,UINT,UINT);

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
  UINT outline;
} SILFONT;

SILFONT *sil_loadFont(char *);
void sil_getPixelFont(SILFONT *,UINT,UINT,BYTE *,BYTE*, BYTE*, BYTE* );
SILFCHAR *sil_getCharFont(SILFONT *, char);
int sil_getKerningFont(SILFONT *, char, char);
void sil_setAlphaFont(SILFONT *, float);
void sil_destroyFont(SILFONT *);
UINT sil_getOutlineFont(SILFONT *);
UINT sil_getHeightFont(SILFONT *);


/* filter.c */

UINT sil_cropAlphaFilter(SILLYR *);
UINT sil_cropFirstpixelFilter(SILLYR *);
UINT sil_brightnessFilter(SILLYR *, int);
UINT sil_blurFilter(SILLYR *);
UINT sil_borderFilter(SILLYR *);
UINT sil_alphaFirstpixelFilter(SILLYR *);
UINT sil_flipxFilter(SILLYR *);
UINT sil_flipyFilter(SILLYR *);
UINT sil_rotateColorFilter(SILLYR *);
UINT sil_reverseColorFilter(SILLYR *);
UINT sil_grayFilter(SILLYR *);

/* drawing.c */

/* textdrawingflags */

#define SILTXT_NOKERNING        1
#define SILTXT_PUNCHOUT         2
#define SILTXT_MONOSPACE        4
#define SILTXT_KEEPCOLOR        8
#define SILTXT_KEEPBLACK       16

/* kinds of line overlap */
#define SILLO_NONE              0
#define SILLO_MAJOR             1
#define SILLO_MINOR             2


void sil_initDraw();
UINT sil_PNGintoLayer(SILLYR *,char *, UINT,UINT);
void sil_paintLayer(SILLYR *,BYTE,BYTE,BYTE,BYTE);
void sil_drawText(SILLYR *,SILFONT *, char *, UINT, UINT, BYTE);
UINT sil_getTextWidth(SILFONT *, char *, BYTE);
void sil_getBackgroundColor(BYTE *,BYTE *, BYTE *, BYTE *);
void sil_setBackgroundColor(BYTE,BYTE,BYTE,BYTE);
void sil_getForegroundColor(BYTE *,BYTE *, BYTE *, BYTE *);
void sil_setForegroundColor(BYTE,BYTE,BYTE,BYTE);
UINT sil_saveDisplay(char *,UINT,UINT,UINT,UINT);
void sil_drawLine(SILLYR *, UINT, UINT, UINT, UINT);
void sil_drawLineAA(SILLYR *, UINT, UINT, UINT, UINT);
void sil_drawBigLine(SILLYR *, UINT, UINT, UINT, UINT);
void sil_drawBigLineAA(SILLYR *, UINT, UINT, UINT, UINT);
void sil_drawCircle(SILLYR *, UINT, UINT, UINT);
void sil_drawCircleAA(SILLYR *, UINT, UINT, UINT);
void sil_drawRectangle(SILLYR *, UINT, UINT, UINT, UINT);
void sil_drawPixel(SILLYR *, UINT, UINT);
void sil_blendPixel(SILLYR *, UINT, UINT);
void sil_setDrawWidth(UINT);
UINT sil_getDrawWidth();
void sil_setZoom(BYTE);
BYTE sil_getZoom();

/* x11display.c  / winSDLdisplay.c / winGDIdisplay.c / lnxdisplay.c */

#define SILDISP_NOTHING      0
#define SILDISP_QUIT         1
#define SILDISP_MOUSE_DOWN   2
#define SILDISP_MOUSE_UP     3
#define SILDISP_KEY_DOWN     4
#define SILDISP_KEY_UP       5
#define SILDISP_NOKEYS       6
#define SILDISP_MOUSE_MOVE   7
#define SILDISP_MOUSEWHEEL   8
#define SILDISP_MOUSE_LEFT   9
#define SILDISP_MOUSE_ENTER 10
#define SILDISP_MOUSE_DRAG  11
#define SILDISP_TIMER       12



UINT sil_initDisplay(void *, UINT, UINT ,char *);
void sil_updateDisplay();
void sil_destroyDisplay();
UINT sil_getTypefromDisplay();
SILEVENT *sil_getEventDisplay();
void sil_setTimerDisplay(UINT);
void sil_stopTimerDisplay();
void sil_setCursor(BYTE);

/* bitmasks for keymodifiers/special keys */
#define SILKM_SHIFT  1
#define SILKM_ALT    2
#define SILKM_CTRL   4
#define SILKM_CAPS   8

/* mouse cursor types */
#define SILCUR_HAND  1
#define SILCUR_ARROW 2
#define SILCUR_WAIT  3
#define SILCUR_HELP  4
#define SILCUR_NO    5
#define SILCUR_IBEAM 6

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


/* colors */

#define SILCOLOR_MAROON 128,0,0
#define SILCOLOR_DARK_RED 139,0,0
#define SILCOLOR_BROWN 165,42,42
#define SILCOLOR_FIREBRICK 178,34,34
#define SILCOLOR_CRIMSON 220,20,60
#define SILCOLOR_RED 255,0,0
#define SILCOLOR_TOMATO 255,99,71
#define SILCOLOR_CORAL 255,127,80
#define SILCOLOR_INDIAN_RED 205,92,92
#define SILCOLOR_LIGHT_CORAL 240,128,128
#define SILCOLOR_DARK_SALMON 233,150,122
#define SILCOLOR_SALMON 250,128,114
#define SILCOLOR_LIGHT_SALMON 255,160,122
#define SILCOLOR_ORANGE_RED 255,69,0
#define SILCOLOR_DARK_ORANGE 255,140,0
#define SILCOLOR_ORANGE 255,165,0
#define SILCOLOR_GOLD 255,215,0
#define SILCOLOR_DARK_GOLDEN_ROD 184,134,11
#define SILCOLOR_GOLDEN_ROD 218,165,32
#define SILCOLOR_PALE_GOLDEN_ROD 238,232,170
#define SILCOLOR_DARK_KHAKI 189,183,107
#define SILCOLOR_KHAKI 240,230,140
#define SILCOLOR_OLIVE 128,128,0
#define SILCOLOR_YELLOW 255,255,0
#define SILCOLOR_YELLOW_GREEN 154,205,50
#define SILCOLOR_DARK_OLIVE_GREEN 85,107,47
#define SILCOLOR_OLIVE_DRAB 107,142,35
#define SILCOLOR_LAWN_GREEN 124,252,0
#define SILCOLOR_CHART_REUSE 127,255,0
#define SILCOLOR_GREEN_YELLOW 173,255,47
#define SILCOLOR_DARK_GREEN 0,100,0
#define SILCOLOR_GREEN 0,128,0
#define SILCOLOR_FOREST_GREEN 34,139,34
#define SILCOLOR_LIME 0,255,0
#define SILCOLOR_LIME_GREEN 50,205,50
#define SILCOLOR_LIGHT_GREEN 144,238,144
#define SILCOLOR_PALE_GREEN 152,251,152
#define SILCOLOR_DARK_SEA_GREEN 143,188,143
#define SILCOLOR_MEDIUM_SPRING_GREEN 0,250,154
#define SILCOLOR_SPRING_GREEN 0,255,127
#define SILCOLOR_SEA_GREEN 46,139,87
#define SILCOLOR_MEDIUM_AQUA_MARINE 102,205,170
#define SILCOLOR_MEDIUM_SEA_GREEN 60,179,113
#define SILCOLOR_LIGHT_SEA_GREEN 32,178,170
#define SILCOLOR_DARK_SLATE_GRAY 47,79,79
#define SILCOLOR_TEAL 0,128,128
#define SILCOLOR_DARK_CYAN 0,139,139
#define SILCOLOR_AQUA 0,255,255
#define SILCOLOR_CYAN 0,255,255
#define SILCOLOR_LIGHT_CYAN 224,255,255
#define SILCOLOR_DARK_TURQUOISE 0,206,209
#define SILCOLOR_TURQUOISE 64,224,208
#define SILCOLOR_MEDIUM_TURQUOISE 72,209,204
#define SILCOLOR_PALE_TURQUOISE 175,238,238
#define SILCOLOR_AQUA_MARINE 127,255,212
#define SILCOLOR_POWDER_BLUE 176,224,230
#define SILCOLOR_CADET_BLUE 95,158,160
#define SILCOLOR_STEEL_BLUE 70,130,180
#define SILCOLOR_CORN_FLOWER_BLUE 100,149,237
#define SILCOLOR_DEEP_SKY_BLUE 0,191,255
#define SILCOLOR_DODGER_BLUE 30,144,255
#define SILCOLOR_LIGHT_BLUE 173,216,230
#define SILCOLOR_SKY_BLUE 135,206,235
#define SILCOLOR_LIGHT_SKY_BLUE 135,206,250
#define SILCOLOR_MIDNIGHT_BLUE 25,25,112
#define SILCOLOR_NAVY 0,0,128
#define SILCOLOR_DARK_BLUE 0,0,139
#define SILCOLOR_MEDIUM_BLUE 0,0,205
#define SILCOLOR_BLUE 0,0,255
#define SILCOLOR_ROYAL_BLUE 65,105,225
#define SILCOLOR_BLUE_VIOLET 138,43,226
#define SILCOLOR_INDIGO 75,0,130
#define SILCOLOR_DARK_SLATE_BLUE 72,61,139
#define SILCOLOR_SLATE_BLUE 106,90,205
#define SILCOLOR_MEDIUM_SLATE_BLUE 123,104,238
#define SILCOLOR_MEDIUM_PURPLE 147,112,219
#define SILCOLOR_DARK_MAGENTA 139,0,139
#define SILCOLOR_DARK_VIOLET 148,0,211
#define SILCOLOR_DARK_ORCHID 153,50,204
#define SILCOLOR_MEDIUM_ORCHID 186,85,211
#define SILCOLOR_PURPLE 128,0,128
#define SILCOLOR_THISTLE 216,191,216
#define SILCOLOR_PLUM 221,160,221
#define SILCOLOR_VIOLET 238,130,238
#define SILCOLOR_MAGENTA 255,0,255
#define SILCOLOR_ORCHID 218,112,214
#define SILCOLOR_MEDIUM_VIOLET_RED 199,21,133
#define SILCOLOR_PALE_VIOLET_RED 219,112,147
#define SILCOLOR_DEEP_PINK 255,20,147
#define SILCOLOR_HOT_PINK 255,105,180
#define SILCOLOR_LIGHT_PINK 255,182,193
#define SILCOLOR_PINK 255,192,203
#define SILCOLOR_ANTIQUE_WHITE 250,235,215
#define SILCOLOR_BEIGE 245,245,220
#define SILCOLOR_BISQUE 255,228,196
#define SILCOLOR_BLANCHED_ALMOND 255,235,205
#define SILCOLOR_WHEAT 245,222,179
#define SILCOLOR_CORN_SILK 255,248,220
#define SILCOLOR_LEMON_CHIFFON 255,250,205
#define SILCOLOR_LIGHT_GOLDEN_ROD_YELLOW 250,250,210
#define SILCOLOR_LIGHT_YELLOW 255,255,224
#define SILCOLOR_SADDLE_BROWN 139,69,19
#define SILCOLOR_SIENNA 160,82,45
#define SILCOLOR_CHOCOLATE 210,105,30
#define SILCOLOR_PERU 205,133,63
#define SILCOLOR_SANDY_BROWN 244,164,96
#define SILCOLOR_BURLY_WOOD 222,184,135
#define SILCOLOR_TAN 210,180,140
#define SILCOLOR_ROSY_BROWN 188,143,143
#define SILCOLOR_MOCCASIN 255,228,181
#define SILCOLOR_NAVAJO_WHITE 255,222,173
#define SILCOLOR_PEACH_PUFF 255,218,185
#define SILCOLOR_MISTY_ROSE 255,228,225
#define SILCOLOR_LAVENDER_BLUSH 255,240,245
#define SILCOLOR_LINEN 250,240,230
#define SILCOLOR_OLD_LACE 253,245,230
#define SILCOLOR_PAPAYA_WHIP 255,239,213
#define SILCOLOR_SEA_SHELL 255,245,238
#define SILCOLOR_MINT_CREAM 245,255,250
#define SILCOLOR_SLATE_GRAY 112,128,144
#define SILCOLOR_LIGHT_SLATE_GRAY 119,136,153
#define SILCOLOR_LIGHT_STEEL_BLUE 176,196,222
#define SILCOLOR_LAVENDER 230,230,250
#define SILCOLOR_FLORAL_WHITE 255,250,240
#define SILCOLOR_ALICE_BLUE 240,248,255
#define SILCOLOR_GHOST_WHITE 248,248,255
#define SILCOLOR_HONEYDEW 240,255,240
#define SILCOLOR_IVORY 255,255,240
#define SILCOLOR_AZURE 240,255,255
#define SILCOLOR_SNOW 255,250,250
#define SILCOLOR_BLACK 0,0,0
#define SILCOLOR_DIM_GRAY 105,105,105
#define SILCOLOR_GRAY 128,128,128
#define SILCOLOR_DARK_GRAY 169,169,169
#define SILCOLOR_SILVER 192,192,192
#define SILCOLOR_LIGHT_GRAY 211,211,211
#define SILCOLOR_GAINSBORO 220,220,220
#define SILCOLOR_WHITE_SMOKE 245,245,245
#define SILCOLOR_WHITE 255,255,255

#endif
