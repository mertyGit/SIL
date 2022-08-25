/*

   winGDIdisplay.c CopyRight 2021 Remco Schellekens, see LICENSE for more details.

   This file contains all functions for displaying the layers on a Windows environment, using standard
   (non-hardware accelerated) GDI API.
   It will do so by using a framebuffer as source for a bitmap inside a fixed size window. Every time 
   framebuffer is changed, bitmap is replaced.
   This isn't the fastest method, however, it will come close to the usual "software framebuffer" use
   on non-hardware accelerated platforms. If you want speed, use additional SDL one (winSDLdisplay.c)

   every "...display.c" file should have these functions
   -sil_initDisplay        ; create initial display, called via initializing SIL
   -sil_updateDisplay      ; update display, will check all layers updates display accordingly
   -sil_destroyDisplay     ; remove display, called via destroying SIL
   -sil_getEventDisplay    ; Wait or get first event ( mouse / keys or closing window )
   -sil_getTypefromDisplay ; will return the "native" color type of the display
   -sil_setTimerDisplay    ; will set a repeating timer to interrupt the wait loop 
   -sil_stopTimerDisplay   ; stops the repeating timer
   -sil_setCursor          ; sets the mouse cursor (in windowed environments) 


*/

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <sys/time.h>
#include "sil.h"
#include "log.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

typedef struct {
  HWND        window;
  WNDCLASSW    wc;
  BITMAPINFO  *bitmapInfo;
  unsigned int w;
  unsigned int h;
} SIL_WINDOW_DATA;



typedef struct _GDISP {
 SIL_WINDOW_DATA win;
 WCHAR name[1000];
 SILFB *fb;
 SILEVENT se;
 HINSTANCE hInstance;
 struct timeval lasttimer;
 HCURSOR cursor;
 BYTE ctype;
} GDISP;

static GDISP gdisp;

/*****************************************************************************

  retrieve color type from Display (SILTYPE... , see framebuffer.c for info)
  Used as "default" when no type is given when creating framebuffer
  Usually, it is the same type as the primary framebuffer the displayfuntions
  are using

 *****************************************************************************/

UINT sil_getTypefromDisplay() {
  if (NULL==gdisp.fb) {
    log_warn("trying to get display color type from non-initialized display");
    return 0;
  }
  return gdisp.fb->type;
}

/*****************************************************************************
  
  Internal function to get status of special keys (shift,menu,control,caps) 
  returns byte with bits set for given keys. Note: it doesn't distinguish 
  between keys on the right or left side of keyboard

 *****************************************************************************/

static BYTE modifiers2sil() {
  BYTE ret=0;
  if (GetKeyState(VK_SHIFT)   & 0x8000) ret|=SILKM_SHIFT;
  if (GetKeyState(VK_MENU)    & 0x8000) ret|=SILKM_ALT;
  if (GetKeyState(VK_CONTROL) & 0x8000) ret|=SILKM_CTRL;
  if (GetKeyState(VK_CAPITAL) &      1) ret|=SILKM_CAPS;
  return ret;
}

/*****************************************************************************
  
  Internal function to map Windows virtual key codes to SIL keycodes

 *****************************************************************************/
static UINT code2sil(WPARAM wParam) {
  
  switch(wParam) {
    case VK_BACK:       return SILKY_BACK;
    case VK_TAB:        return SILKY_TAB;
    case VK_RETURN:     return SILKY_ENTER;
    case VK_SHIFT:      return SILKY_SHIFT;
    case VK_CONTROL:    return SILKY_CTRL;
    case VK_MENU:       return SILKY_ALT;
    case VK_PAUSE:      return SILKY_PAUSE;
    case VK_CAPITAL:    return SILKY_CAPS;
    case VK_ESCAPE:     return SILKY_ESC;
    case VK_SPACE:      return SILKY_SPACE;
    case VK_PRIOR:      return SILKY_PAGEUP;
    case VK_NEXT:       return SILKY_PAGEDOWN;
    case VK_END:        return SILKY_END;
    case VK_HOME:       return SILKY_HOME;
    case VK_LEFT:       return SILKY_LEFT;
    case VK_UP:         return SILKY_UP;
    case VK_RIGHT:      return SILKY_RIGHT;
    case VK_DOWN:       return SILKY_DOWN;
    case VK_SNAPSHOT:   return SILKY_PRINTSCREEN;
    case VK_INSERT:     return SILKY_INSERT;
    case VK_DELETE:     return SILKY_DELETE;
    case 0x30:          return SILKY_0;
    case 0x31:          return SILKY_1;
    case 0x32:          return SILKY_2;
    case 0x33:          return SILKY_3;
    case 0x34:          return SILKY_4;
    case 0x35:          return SILKY_5;
    case 0x36:          return SILKY_6;
    case 0x37:          return SILKY_7;
    case 0x38:          return SILKY_8;
    case 0x39:          return SILKY_9;
    case 0x41:          return SILKY_A;
    case 0x42:          return SILKY_B;
    case 0x43:          return SILKY_C;
    case 0x44:          return SILKY_D;
    case 0x45:          return SILKY_E;
    case 0x46:          return SILKY_F;
    case 0x47:          return SILKY_G;
    case 0x48:          return SILKY_H;
    case 0x49:          return SILKY_I;
    case 0x4A:          return SILKY_J;
    case 0x4B:          return SILKY_K;
    case 0x4C:          return SILKY_L;
    case 0x4D:          return SILKY_M;
    case 0x4E:          return SILKY_N;
    case 0x4F:          return SILKY_O;
    case 0x50:          return SILKY_P;
    case 0x51:          return SILKY_Q;
    case 0x52:          return SILKY_R;
    case 0x53:          return SILKY_S;
    case 0x54:          return SILKY_T;
    case 0x55:          return SILKY_U;
    case 0x56:          return SILKY_V;
    case 0x57:          return SILKY_W;
    case 0x58:          return SILKY_X;
    case 0x59:          return SILKY_Y;
    case 0x5A:          return SILKY_Z;
    case VK_NUMPAD0:    return SILKY_NUM0;
    case VK_NUMPAD1:    return SILKY_NUM1;
    case VK_NUMPAD2:    return SILKY_NUM2;
    case VK_NUMPAD3:    return SILKY_NUM3;
    case VK_NUMPAD4:    return SILKY_NUM4;
    case VK_NUMPAD5:    return SILKY_NUM5;
    case VK_NUMPAD6:    return SILKY_NUM6;
    case VK_NUMPAD7:    return SILKY_NUM7;
    case VK_NUMPAD8:    return SILKY_NUM8;
    case VK_NUMPAD9:    return SILKY_NUM9;
    case VK_MULTIPLY:   return SILKY_NUMMULTIPLY;
    case VK_ADD:        return SILKY_NUMPLUS;
    case VK_SUBTRACT:   return SILKY_NUMMINUS;
    case VK_DECIMAL:    return SILKY_NUMPOINT;
    case VK_DIVIDE:     return SILKY_NUMSLASH;
    case VK_F1:         return SILKY_F1;
    case VK_F2:         return SILKY_F2;
    case VK_F3:         return SILKY_F3;
    case VK_F4:         return SILKY_F4;
    case VK_F5:         return SILKY_F5;
    case VK_F6:         return SILKY_F6;
    case VK_F7:         return SILKY_F7;
    case VK_F8:         return SILKY_F8;
    case VK_F9:         return SILKY_F9;
    case VK_F10:        return SILKY_F10;
    case VK_F11:        return SILKY_F11;
    case VK_F12:        return SILKY_F12;
    case VK_NUMLOCK:    return SILKY_NUMLOCK;
    case VK_SCROLL:     return SILKY_SCROLLLOCK;
    case VK_OEM_1:      return SILKY_SEMICOLON;
    case VK_OEM_PLUS:   return SILKY_PLUS;
    case VK_OEM_COMMA:  return SILKY_COMMA;
    case VK_OEM_MINUS:  return SILKY_MINUS;
    case VK_OEM_PERIOD: return SILKY_PERIOD;
    case VK_OEM_2:      return SILKY_SLASH;
    case VK_OEM_3:      return SILKY_ACUTE;
    case VK_OEM_4:      return SILKY_OPENBRACKET;
    case VK_OEM_5:      return SILKY_BACKSLASH;
    case VK_OEM_6:      return SILKY_CLOSEBRACKET;
    case VK_OEM_7:      return SILKY_APOSTROPHE;
  }
  return 0;
}


/*****************************************************************************
  
  Initialize Display (called by initSIL) only for GDI, we need to have the "hIntance" 
  of WinMain call (SDL can use its own, using flag DSL_MAIN_HANDLED)
  other options are width,height and title of window

 *****************************************************************************/
UINT sil_initDisplay(void *hI, UINT width, UINT height, char *title) {
  unsigned char *image;
  unsigned error;
  WNDCLASSW wc = {0};
  int cnt=0;
  RECT sz;


  /* hI = HINSTANCE is returned by calling WinMain, therefore should be given to SIL */
  gdisp.hInstance=(HINSTANCE) hI;

  
  /* this framebuffer will be dedicated for the window */
  gdisp.fb=sil_initFB(width, height, SILTYPE_ARGB);
  if (NULL==gdisp.fb) {
    log_info("ERR: Can't create framebuffer for display");
    return SILERR_NOMEM;
  }

  /* simple C-string to more unicode "widechar" string */
  MultiByteToWideChar(CP_ACP,0,title,-1,(LPWSTR)gdisp.name,sizeof(gdisp.name)-1);

  wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpszClassName = gdisp.name;
  wc.hInstance     = gdisp.hInstance;
  wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
  wc.lpfnWndProc   = WndProc;
  wc.hCursor       = LoadCursor(0, IDC_ARROW);
  wc.hIcon         = LoadIcon(hI, MAKEINTRESOURCE(1));

  /* since we set it to arrow in the class, use it as default for sil_setCursor */
  gdisp.ctype=SILCUR_ARROW;

  /* Set windows options */
  RegisterClassW(&wc);

  /* and create the window */
  gdisp.win.window=CreateWindowExW(
    0,     /* WS_EX_... possibilities                          */
    wc.lpszClassName,wc.lpszClassName, /* should be the same ? */
    WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
    CW_USEDEFAULT, CW_USEDEFAULT,   /* initial position of window (left upper corner)   */
    gdisp.fb->width+6,gdisp.fb->height+30,
    NULL,NULL,NULL,NULL /* no hWndParent,hMenu,hInstance or lpParam to set */
  );

  if (NULL==gdisp.win.window) {
    printf("ERROR: Can't create window (%d)\n",GetLastError());

    return SILERR_NOTINIT;
  }

  /* display window (nothing in it, yet) */
  ShowWindow(gdisp.win.window, SW_NORMAL);

  /* now create a bitmap that has the same colordepth and dimensions as our framebuffer    */
  gdisp.win.bitmapInfo = (BITMAPINFO  *) calloc(1,sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) *3);
  gdisp.win.bitmapInfo->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
  gdisp.win.bitmapInfo->bmiHeader.biPlanes      = 1;
  gdisp.win.bitmapInfo->bmiHeader.biBitCount    = 32;
  gdisp.win.bitmapInfo->bmiHeader.biCompression = BI_BITFIELDS;
  gdisp.win.bitmapInfo->bmiHeader.biWidth       = gdisp.fb->width;
  gdisp.win.bitmapInfo->bmiHeader.biHeight      = -gdisp.fb->height; /* yup, minus, from bottom to top */
  gdisp.win.bitmapInfo->bmiColors[0].rgbRed     = 0xff;
  gdisp.win.bitmapInfo->bmiColors[1].rgbGreen   = 0xff;
  gdisp.win.bitmapInfo->bmiColors[2].rgbBlue    = 0xff;
  GetWindowRect(gdisp.win.window,&sz);
  AdjustWindowRect(&sz,WS_OVERLAPPEDWINDOW  & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,FALSE);

  /* fix for hanging mousepointer */
  sil_setCursor(SILCUR_HAND);
  sil_setCursor(SILCUR_ARROW);

  return SILERR_ALLOK;

}

/*****************************************************************************
  
  Update Display

 *****************************************************************************/
void sil_updateDisplay() {
  HDC         hdc;

  /* get all layerinformation into a single fb */
  LayersToFB(gdisp.fb);

  /* get device context (GDI handle) */
  hdc = GetDC(gdisp.win.window);

  /* now "stretch" the framebuffer over the earlier created bitmap */
  /* since sizes are the same, there will be no scaling */
  StretchDIBits(
    hdc,                    /* GDI context                      */
    0, 0,                   /* upper left corner of destination */
    gdisp.fb->width, gdisp.fb->height,  /* width/height of destination      */
    0, 0,                   /* upper left corner of source (FB) */
    gdisp.fb->width, gdisp.fb->height,  /* width/height of source           */
    gdisp.fb->buf,                /* pointer source pixels            */
    gdisp.win.bitmapInfo,         /* pointer to window bitmap         */
    DIB_RGB_COLORS,         /* use RGB values                   */
    SRCCOPY                 /* just plain-old-copy              */
  );
  ValidateRect(gdisp.win.window,NULL);

  /* release device context (GDI handle) */
  ReleaseDC(gdisp.win.window,hdc);
}

/*****************************************************************************
  
  Destroy display information (called by destroy SIL, to cleanup everything )

 *****************************************************************************/
void sil_destroyDisplay() { 
  if (gdisp.fb->type) sil_destroyFB(gdisp.fb);
}


/*****************************************************************************
  
  Get event from display
  depending on abilities to do so, setting "wait" on 1 will wait until event 
  comes (blocking) and setting to zero will only poll.
  However, on Windows platform we only use blocking wait 

  It will return a SILEVENT, containing:
   * type (SILDISP... types, from key-up/down till mouse events)
   * val => MOUSE_DOWN/UP: which button is pressed 1:left 2:middle 3:right
            MOUSEWHEEL   : direction of wheel 1:up 2:down
            KEY_DOWN/UP  : ascii value of key(-combination)
    
   * x,y       => position of mouse at that time
   * code      => 'native' keycode (in this case windows 'virtual keycode')
   * key       => translated 'common' SIL keycode 
   * modifiers => byte with bits reflecting status of shift,alt,ctrl and caps

   Note that keycodes are -trying to- refferring to the pressed key, 
   *not the ascii or unicode code that is printed on it *. To figure out if 'a'
   or 'A' is typed , check for keycode 'a' + modifiers "Shift" or just get the
   'val', where OS is trying to translate keycode to ascii.
   (Actually, unicode, but I'm lazy and only want single-byte codes ... )

 *****************************************************************************/
SILEVENT *sil_getEventDisplay() {
  MSG  msg;
  gdisp.se.type=SILDISP_NOTHING;
  gdisp.se.val=0;
  gdisp.se.x=0;
  gdisp.se.y=0;
  gdisp.se.code=0;
  gdisp.se.key=0;
  gdisp.se.modifiers=0;
  gdisp.se.layer=NULL;
  while ((SILDISP_NOTHING==gdisp.se.type)&&(GetMessage(&msg, NULL, 0, 0))) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return &gdisp.se;
}


void sil_setTimerDisplay(UINT amount) {
  gettimeofday(&gdisp.lasttimer,NULL);
  SetTimer(gdisp.win.window, 666, amount, (TIMERPROC) NULL);
}

void sil_stopTimerDisplay() {
  KillTimer(gdisp.win.window, 666);
}

void sil_setCursor(BYTE type) {
  /* only load if cursor has been changed */
  if (type!=gdisp.ctype) {
    switch(type) {
      case SILCUR_ARROW:
        gdisp.cursor=LoadCursor(NULL,IDC_ARROW);
        break;
      case SILCUR_HAND:
        gdisp.cursor=LoadCursor(NULL,IDC_HAND);
        break;
      case SILCUR_HELP:
        gdisp.cursor=LoadCursor(NULL,IDC_HELP);
        break;
      case SILCUR_NO:
        gdisp.cursor=LoadCursor(NULL,IDC_NO);
        break;
      case SILCUR_IBEAM:
        gdisp.cursor=LoadCursor(NULL,IDC_IBEAM);
        break;
    }
    if (gdisp.cursor) {
      SetCursor(gdisp.cursor);
      gdisp.ctype=type;
    }
  }
}


/*****************************************************************************
  
  Event handler called when window receives an event, indirectly via 
  "DispatchMessage"

 *****************************************************************************/

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  HDC         hdc;
  BYTE kstate[256];
  WCHAR  kbuf[5];
  struct timeval tv;

  
  switch(msg) {
    case WM_TIMER:
      gdisp.se.type=SILDISP_TIMER;
      gdisp.se.code=wParam;
      gettimeofday(&tv,NULL);
      gdisp.se.val=(tv.tv_sec-gdisp.lasttimer.tv_sec)*1000+(tv.tv_usec-gdisp.lasttimer.tv_usec)/1000;
      gettimeofday(&gdisp.lasttimer,NULL);
      return 0;
      break;

    case WM_PAINT:
      hdc = GetDC(gdisp.win.window);
      StretchDIBits(hdc, 0, 0, gdisp.fb->width, gdisp.fb->height, 0, 0, 
        gdisp.fb->width, gdisp.fb->height, gdisp.fb->buf, gdisp.win.bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
      ValidateRect(hwnd,0);
      ReleaseDC(gdisp.win.window,hdc);
      break;

    case WM_LBUTTONDOWN:
      gdisp.se.type=SILDISP_MOUSE_DOWN;
      gdisp.se.x = GET_X_LPARAM(lParam);
      gdisp.se.y = GET_Y_LPARAM(lParam);
      gdisp.se.val=1;
      return 0;
      break;

    case WM_LBUTTONUP:
      gdisp.se.type=SILDISP_MOUSE_UP;
      gdisp.se.x = GET_X_LPARAM(lParam);
      gdisp.se.y = GET_Y_LPARAM(lParam);
      gdisp.se.val=1;
      return 0;
      break;

    case WM_MBUTTONDOWN:
      gdisp.se.type=SILDISP_MOUSE_DOWN;
      gdisp.se.x = GET_X_LPARAM(lParam);
      gdisp.se.y = GET_Y_LPARAM(lParam);
      gdisp.se.val=2;
      return 0;
      break;

    case WM_MBUTTONUP:
      gdisp.se.type=SILDISP_MOUSE_UP;
      gdisp.se.x = GET_X_LPARAM(lParam);
      gdisp.se.y = GET_Y_LPARAM(lParam);
      gdisp.se.val=2;
      return 0;
      break;

    case WM_RBUTTONDOWN:
      gdisp.se.type=SILDISP_MOUSE_DOWN;
      gdisp.se.x = GET_X_LPARAM(lParam);
      gdisp.se.y = GET_Y_LPARAM(lParam);
      gdisp.se.val=3;
      return 0;
      break;

    case WM_RBUTTONUP:
      gdisp.se.type=SILDISP_MOUSE_UP;
      gdisp.se.x = GET_X_LPARAM(lParam);
      gdisp.se.y = GET_Y_LPARAM(lParam);
      gdisp.se.val=3;
      return 0;
      break;

    case WM_MOUSEMOVE:
      gdisp.se.type=SILDISP_MOUSE_MOVE;
      gdisp.se.x = GET_X_LPARAM(lParam);
      gdisp.se.y = GET_Y_LPARAM(lParam);
      gdisp.se.val=0;
      return 0;
      break;

    case WM_MOUSEWHEEL:
      gdisp.se.type=SILDISP_MOUSEWHEEL;
      gdisp.se.x = GET_X_LPARAM(lParam);
      gdisp.se.y = GET_Y_LPARAM(lParam);
      if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
        gdisp.se.val=2;
      else 
        gdisp.se.val=1;
      return 0;
      break;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      GetKeyboardState(kstate);
      ToUnicode(wParam,MapVirtualKey(wParam, MAPVK_VK_TO_VSC),kstate,kbuf,sizeof(kbuf),0);
      gdisp.se.type=SILDISP_KEY_DOWN;
      gdisp.se.code=wParam;
      gdisp.se.key=code2sil(wParam);
      gdisp.se.modifiers=modifiers2sil();
      gdisp.se.val=kbuf[0];
      return 0;
      break; 


    case WM_SYSKEYUP:
    case WM_KEYUP:
      GetKeyboardState(kstate);
      ToUnicode(wParam,MapVirtualKey(wParam, MAPVK_VK_TO_VSC),kstate,kbuf,sizeof(kbuf),0);
      gdisp.se.type=SILDISP_KEY_UP;
      gdisp.se.code=wParam;
      gdisp.se.key=code2sil(wParam);
      gdisp.se.modifiers=modifiers2sil();
      gdisp.se.val=kbuf[0];
      return 0;
      break;

    case WM_DESTROY:
      gdisp.se.type=SILDISP_QUIT;
      PostQuitMessage(0);
      return 0;
      break;

    case WM_SETCURSOR:
      return 0;
      break;
  }

  return DefWindowProcW(hwnd, msg, wParam, lParam);
}

/* capture screen and put it in a layer for further use */
SILLYR *sil_screenCapture() {
  HDC hdc,hdcc;
  SILLYR *lyr=NULL;
  BITMAPINFOHEADER bi;
  HBITMAP hbwin;
  DWORD bsize;
  HANDLE gl;
  BYTE *bmp;
  int scale,screenx,screeny,width,height;

  /* get and create handles */
  hdc = GetDC(0);
  hdcc = CreateCompatibleDC(hdc);

  /* get screen size */
  SetStretchBltMode(hdcc,COLORONCOLOR);
  scale=1;
  screenx=GetSystemMetrics(SM_XVIRTUALSCREEN);
  screeny=GetSystemMetrics(SM_YVIRTUALSCREEN);
  width=GetSystemMetrics(SM_CXVIRTUALSCREEN);
  height=GetSystemMetrics(SM_CYVIRTUALSCREEN);

  /* init bitmap */
  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = width;
  bi.biHeight = -height;
  bi.biPlanes = 1;
  bi.biBitCount = 32;
  bi.biCompression = BI_RGB;
  bi.biSizeImage = 0;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrUsed = 0;
  bi.biClrImportant = 0;
  hbwin =  CreateCompatibleBitmap(hdc,width,height);

  /* Create layer */
  lyr=sil_addLayer(0,0,width,height,SILTYPE_ARGB);
  if (NULL==lyr) {
    log_warn("Can't create layer for screenshot");
    return NULL;
  }

  /* get screen */
  SelectObject(hdcc,hbwin);
  StretchBlt(hdcc,0,0,width,height,hdc,screenx,screeny,width,height,SRCCOPY);
  GetDIBits(hdcc,hbwin,0,height,lyr->fb->buf, (BITMAPINFO*)&bi,DIB_RGB_COLORS);

  /* delete and release handles */
  DeleteDC(hdcc);
  ReleaseDC(gdisp.win.window,hdc);
  return lyr;
}
