/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)windows/DAL-gfx.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"
#include "DAL-gfx-palette.h"

/*************************************************************************
 *
 * Platform Implementation Structures
 *
 *************************************************************************/

typedef struct _Gfx_DeviceInfo
{
  gfx_window *win;
} _Gfx_DeviceInfo;

typedef struct _WindowsWindow
{
  HDC         dc;                         // window dc handle
  BYTE        bmi[sizeof(BITMAPINFO) + (sizeof(RGBQUAD) * (PALETTE_SIZE - 1))];   // palette & bitmap info

  HBITMAP     bmp;                        // bitmap handle
  void       *bits;                       // offscreen buffer pointer
  UINT        width;                      // window width
  UINT        height;                     // window height
} _WindowsWindow;

typedef struct _FontInfo
{
  _WindowsWindow **winFonts;
  coord           *fontHeight;
  uint8          **fontWidth;
  coord          **fontOffset;          // font resources
} _FontInfo;

// local functions
static void _GfxInitialize();
#ifdef GFX_12BPP_CUSTOM
static void _GfxCopyToDisplay12bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y);
#endif
static void _GfxCopyToDisplay16bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y);
static void _GfxTerminate();

static gfx_window *_GfxCreateWindowX(coord width, coord height, int16 depth);

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

void
_GfxInitialize()
{
  _Gfx_DeviceInfo *deviceP;
  _FontInfo       *fontP;
  int              i;

  gDALGlobals.gfx.info = (void *)malloc(sizeof(_Gfx_DeviceInfo));
  memset(gDALGlobals.gfx.info, 0, sizeof(_Gfx_DeviceInfo));
  deviceP = (_Gfx_DeviceInfo *)gDALGlobals.gfx.info;

  gDALGlobals.gfx.font = (void *)malloc(sizeof(_FontInfo));
  memset(gDALGlobals.gfx.font, 0, sizeof(_FontInfo));
  fontP = (_FontInfo *)gDALGlobals.gfx.font;

  gDALGlobals.gfx._16bpp      = (uint16 *)malloc(sizeof(uint16) * PALETTE_SIZE);
  gDALGlobals.gfx.palette     = (RGBQUAD *)malloc(sizeof(RGBQUAD) * PALETTE_SIZE);
  gDALGlobals.gfx.paletteDiff = (uint32 *)malloc(sizeof(uint32) * PALETTE_SIZE);
  memset(gDALGlobals.gfx._16bpp,      0, sizeof(uint16)  * PALETTE_SIZE);
  memset(gDALGlobals.gfx.palette,     0, sizeof(RGBQUAD) * PALETTE_SIZE);
  memset(gDALGlobals.gfx.paletteDiff, 0, sizeof(uint32)  * PALETTE_SIZE);

  //
  // device gfx initialization
  //

  // this is determined in DAL.c
#ifdef GFX_12BPP_CUSTOM
  gDALGlobals.gfx.depth = GetDeviceCaps(gDALGlobals.LCDScreen, BITSPIXEL);
  if (gDALGlobals.gfx.depth > 16) 
#endif
    gDALGlobals.gfx.depth = 16;   

  //
  // gfx initialization
  //

  gDALGlobals.gfx.window =
    _GfxCreateWindow((coord)(gDALGlobals.gfx.width), (coord)(gDALGlobals.gfx.height));
  _GfxSetDrawWindow(gDALGlobals.gfx.window);

  // prepare blitter function pointer (if needed)
  gDALGlobals.gfx.fnCopyRegion = NULL;
  deviceP->win              = NULL;
  switch (gDALGlobals.gfx.depth)
  {
#ifdef GFX_12BPP_CUSTOM
    case 12:
         deviceP->win = _GfxCreateWindowX((coord)(gDALGlobals.gfx.width), (coord)(gDALGlobals.gfx.height), 12);
         gDALGlobals.gfx.fnCopyRegion = _GfxCopyToDisplay12bpp;
         break;
#endif
    case 16:
         deviceP->win = _GfxCreateWindowX((coord)(gDALGlobals.gfx.width), (coord)(gDALGlobals.gfx.height), 16);
         gDALGlobals.gfx.fnCopyRegion = _GfxCopyToDisplay16bpp;
         break;

    default:
         break;
  }

  // initialize the "font" resources 
  fontP->winFonts   = (_WindowsWindow **)malloc(sizeof(_WindowsWindow *) * MAX_FONT);
  fontP->fontHeight = (coord *)malloc(sizeof(coord) * MAX_FONT);
  fontP->fontWidth  = (uint8 **)malloc(sizeof(uint8 *) * MAX_FONT);
  fontP->fontOffset = (coord **)malloc(sizeof(coord *) * MAX_FONT);
  for (i=0; i<MAX_FONT; i++)
  {
    fontP->winFonts[i]   = NULL;
    fontP->fontWidth[i]  = NULL;
    fontP->fontOffset[i] = NULL;
  }

  // default to system palette
  _GfxSetPalette(sys_palette);
}

void
_GfxTerminate()
{
  _Gfx_DeviceInfo *deviceP;
  _FontInfo       *fontP;
  int              i;

  deviceP = (_Gfx_DeviceInfo *)gDALGlobals.gfx.info;
  fontP   = (_FontInfo *)gDALGlobals.gfx.font;

  //
  // gfx shutdown
  //

  _GfxSetDrawWindow(NULL);
  _GfxDeleteWindow(gDALGlobals.gfx.window);
  switch (gDALGlobals.gfx.depth)
  {
    case 16:
         _GfxDeleteWindow(deviceP->win);
         break;

    default:
         break;
  }

  free(gDALGlobals.gfx._16bpp);
  free(gDALGlobals.gfx.palette);
  free(gDALGlobals.gfx.paletteDiff);
  gDALGlobals.gfx._16bpp      = NULL;
  gDALGlobals.gfx.palette     = NULL;
  gDALGlobals.gfx.paletteDiff = NULL;

  // clean up "font" resources
  for (i=0; i<MAX_FONT; i++)
    _FntReleaseFont((font)i);
  free(fontP->winFonts);
  free(fontP->fontHeight);
  free(fontP->fontWidth);
  free(fontP->fontOffset);

  free(gDALGlobals.gfx.info);
  free(gDALGlobals.gfx.font);
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

gfx_window *
_GfxCreateWindowX(coord width, coord height, int16 depth)
{
  void             *ptr;
  BITMAPINFO       *pbmi;
  _WindowsWindow   *winPtr;

  winPtr = (_WindowsWindow *)malloc(sizeof(_WindowsWindow));
  if (winPtr != NULL)
  {
    memset(winPtr, 0, sizeof(_WindowsWindow));

    pbmi = (BITMAPINFO *)winPtr->bmi;
    pbmi->bmiHeader.biClrImportant    = 0;
    pbmi->bmiHeader.biClrUsed         = 0;
    pbmi->bmiHeader.biCompression     = (depth == 8) ? BI_RGB : BI_BITFIELDS;
    pbmi->bmiHeader.biPlanes          = 1;
    pbmi->bmiHeader.biBitCount        = (WORD)(depth);
    pbmi->bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biHeight          = -(LONG)(height);
    pbmi->bmiHeader.biWidth           = (LONG)(width);
    pbmi->bmiHeader.biSizeImage       = ((((((LONG)(width) * pbmi->bmiHeader.biBitCount) + 31) & ~(31)) / 8) * (LONG)(height));
    pbmi->bmiHeader.biXPelsPerMeter   = 0;
    pbmi->bmiHeader.biYPelsPerMeter   = 0;
  
    // configure the bitmap to use the currently active palette (important)
    switch (depth)
    {
      case 8:  memmove(&pbmi->bmiColors[0], &gDALGlobals.gfx.palette[0], (uint16)(sizeof(RGBQUAD) * PALETTE_SIZE));
               break;

      case 16: {
                 DWORD *mask = (DWORD *)pbmi->bmiColors;
                 *mask++ = 0x0000f800;
                 *mask++ = 0x000007e0;
                 *mask++ = 0x0000001f;
               }
               break;

      default: _SysDebugMessage("SHARK does not support this display depth!");
               break;
    }

    winPtr->dc  = CreateCompatibleDC(gDALGlobals.LCDScreen);
    winPtr->bmp = CreateDIBSection(winPtr->dc, pbmi, 
      DIB_RGB_COLORS, ((void **)(&ptr)), NULL, 0);
    SelectObject(winPtr->dc, winPtr->bmp);
    winPtr->bits = ptr;

    winPtr->width  = (UINT)(width);
    winPtr->height = (UINT)(height);

    if ((winPtr->dc == NULL) || (winPtr->bmp == NULL) || (winPtr->bits == NULL))
    {
      free(winPtr);
      return NULL;
    }
  }

  return (gfx_window *)(winPtr);
}

gfx_window *
_GfxCreateWindow(coord width, coord height)
{
  return _GfxCreateWindowX(width, height, GFX_WINDOW_BPP);
}

gfx_window *
_GfxCreateWindowFromBuffer(coord width, coord height, uint8 *bits)
{
  _WindowsWindow *winPtr;

  winPtr = (_WindowsWindow *)malloc(sizeof(_WindowsWindow));
  if (winPtr != NULL)
  {
    memset(winPtr, 0, sizeof(_WindowsWindow));

    winPtr->width  = width;
    winPtr->height = height;
    winPtr->bits   = bits;
  }

  return (gfx_window *)(winPtr);
}

void
_GfxDeleteWindow(gfx_window *win)
{
  if (win != NULL)
  {
    _WindowsWindow *winPtr = (_WindowsWindow *)win;

    if (winPtr->bmp != NULL) DeleteObject(winPtr->bmp);
    if (winPtr->dc != NULL)  DeleteDC(winPtr->dc);

    free(win);
  }
}

void
_GfxCopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  _WindowsWindow *window = (_WindowsWindow *)win;

  // do we have a function pointer for customized blitting?
  if (gDALGlobals.gfx.fnCopyRegion == NULL)
  {
#ifndef _WIN32_WCE
    StretchBlt(gDALGlobals.LCDScreen,
               (int)(scr_x * gDALGlobals.gfx.stretch_x), (int)(scr_y * gDALGlobals.gfx.stretch_y), (int)(rect->width * gDALGlobals.gfx.stretch_x), (int)(rect->height * gDALGlobals.gfx.stretch_y),
               window->dc, 
               (int)(rect->x), (int)(rect->y), (int)(rect->width), (int)(rect->height),
               SRCCOPY);
#else
    BitBlt(gDALGlobals.LCDScreen,
           (int)(rect->x), (int)(rect->y), (int)(rect->width), (int)(rect->height),
           window->dc, (int)(scr_x), (int)(scr_y), SRCCOPY);
#endif
  }

  // customized blitter (function pointer)
  else
    gDALGlobals.gfx.fnCopyRegion(win, rect, scr_x, scr_y);
}

/*************************************************************************
 *
 * Library Routines
 *
 *************************************************************************/

#include "DAL-gfx-lib.inc"
#include "DAL-gfx-fonts.inc"

/*************************************************************************
 *
 * Custom Device Blitters
 *
 *************************************************************************/

#ifdef GFX_12BPP_CUSTOM

void 
_GfxCopyToDisplay12bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  _SysDebugMessage("SHARK: unsupported screen depth [12bpp]");
  // THIS NEEDS TO BE WRITTEN, 2 pixels for 3 bytes (packed 12 bit)
}

#endif

void 
_GfxCopyToDisplay16bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  _Gfx_DeviceInfo   *deviceP;
  _WindowsWindow    *window = (_WindowsWindow *)win;
  _WindowsWindow    *winLCD;

  deviceP = (_Gfx_DeviceInfo *)gDALGlobals.gfx.info;

  //
  // translation: 8bpp -> 16bpp
  //

  winLCD = (_WindowsWindow *)deviceP->win;
  {
    uint16 *ptrLCD, *pal;
    uint8  *ptrWin;
    int     cnt, pix, i;
    int32   win_delta, scr_delta;
    uint32  win1, win2;

    // determine the "pointer" value, and how many bytes to blit
    pal     = gDALGlobals.gfx._16bpp;
    ptrWin  =  (uint8 *)window->bits;
    ptrLCD  = (uint16 *)winLCD->bits;
    ptrWin += (((uint32)rect->y * window->width) + rect->x);
    ptrLCD += (((uint32)rect->y * winLCD->width) + rect->x);

    win_delta = window->width - rect->width;
    scr_delta = winLCD->width - rect->width;

    cnt = rect->height;
    pix = rect->width;

    // do each line...
    do
    {
      i = pix;

      win2 = (4 - ((uint32)(ptrWin))) & 0x3;
      if (win2)
      {
        i -= win2;
        do
        {
          win1 = *ptrWin++;
          *ptrLCD++ = *(pal + win1);
        }
        while (--win2);
      }

      // chunks of 8 writes (faster)
      win2 = i & 0x7;
      if (i >>= 3)
      {
        do 
        {
		      win1 = ((uint32 *)(ptrWin))[0];
          ((uint32 *)(ptrLCD))[0] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 7) & 0x1fe)) << 16)  +
            *(uint16 *)((uint8 *)(pal) + ((win1 & 0xff) << 1));
          ((uint32 *)(ptrLCD))[1] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 23) & 0x1fe)) << 16) +
            *(uint16 *)((uint8 *)(pal) + ((win1 >> 15) & 0x1fe));
		      win1 = ((uint32 *)(ptrWin))[1];
          ptrWin += 8;
          ((uint32 *)(ptrLCD))[2] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 7) & 0x1fe))  << 16) +
            *(uint16 *)((uint8 *)(pal) + ((win1 & 0xff) << 1));
          ((uint32 *)(ptrLCD))[3] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 23) & 0x1fe)) << 16) +
            *(uint16 *)((uint8 *)(pal) + ((win1 >> 15) & 0x1fe));
          ptrLCD += 8;
        }
        while (--i);
      }

      if (win2)
      {
        do
        {
          win1 = *ptrWin++;
          *ptrLCD++ = *(pal + win1);
        }
        while (--win2);
      }

      ptrWin += win_delta;
      ptrLCD += scr_delta;
    }
    while (--cnt);
  }

  //
  // copy:        16bpp -> LCD
  //

#ifndef _WIN32_WCE
  StretchBlt(gDALGlobals.LCDScreen,
             (int)(scr_x * gDALGlobals.gfx.stretch_x), (int)(scr_y * gDALGlobals.gfx.stretch_y), (int)(rect->width * gDALGlobals.gfx.stretch_x), (int)(rect->height * gDALGlobals.gfx.stretch_y),
             winLCD->dc, 
             (int)(rect->x), (int)(rect->y), (int)(rect->width), (int)(rect->height),
             SRCCOPY);
#else
  BitBlt(gDALGlobals.LCDScreen,
         (int)(rect->x), (int)(rect->y), (int)(rect->width), (int)(rect->height),
         winLCD->dc, (int)(scr_x), (int)(scr_y), SRCCOPY);
#endif
}

/********************************* EOF ***********************************/
