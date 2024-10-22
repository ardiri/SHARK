/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)series60/DAL-gfx.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 * Platform Implementation Structures
 *
 *************************************************************************/

typedef struct _Gfx_DeviceInfo
{
  int dummy;
} _Gfx_DeviceInfo;

typedef struct _SymbianWindow
{
  boolean     allocated;                  // memory is allocated?

  void       *bits;                       // offscreen buffer pointer
  coord       width;                      // window width
  coord       height;                     // window height
} _SymbianWindow;

typedef struct _FontInfo
{
  _SymbianWindow **winFonts;
  coord           *fontHeight;
  uint8          **fontWidth;
  coord          **fontOffset;            // font resources
} _FontInfo;

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
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  gDALGlobals->gfx.info = (void *)malloc(sizeof(_Gfx_DeviceInfo));
  memset(gDALGlobals->gfx.info, 0, sizeof(_Gfx_DeviceInfo));
  deviceP = (_Gfx_DeviceInfo *)gDALGlobals->gfx.info;

  gDALGlobals->gfx.font_info = (void *)malloc(sizeof(_FontInfo));
  memset(gDALGlobals->gfx.font_info, 0, sizeof(_FontInfo));
  fontP = (_FontInfo *)gDALGlobals->gfx.font_info;

  //
  // device gfx initialization
  //

  // this is determined in DAL.cpp

  gDALGlobals->gfx.palette        = (uint8 *)malloc(sizeof(uint8) * PALETTE_SIZE * 4);
  gDALGlobals->gfx.paletteDiff    = (uint32 *)malloc(sizeof(uint32) * PALETTE_SIZE);
  gDALGlobals->gfx.iPaletteLookup = (uint16 *)malloc(sizeof(uint16) * PALETTE_SIZE);
  memset(gDALGlobals->gfx.palette,        0, sizeof(uint8) * PALETTE_SIZE * 4);
  memset(gDALGlobals->gfx.paletteDiff,    0, sizeof(uint32) * PALETTE_SIZE);
  memset(gDALGlobals->gfx.iPaletteLookup, 0, sizeof(uint16) * PALETTE_SIZE);

  //
  // gfx initialization
  //

  gDALGlobals->gfx.window =
    _GfxCreateWindow((coord)(gDALGlobals->gfx.width), (coord)(gDALGlobals->gfx.height));
  _GfxSetDrawWindow(gDALGlobals->gfx.window);

  // initialize the "font" resources
  fontP->winFonts   = (_SymbianWindow **)malloc(sizeof(_SymbianWindow *) * MAX_FONT);
  fontP->fontHeight = (coord *)malloc(sizeof(coord) * MAX_FONT);
  fontP->fontWidth  = (uint8 **)malloc(sizeof(uint8 *) * MAX_FONT);
  fontP->fontOffset = (coord **)malloc(sizeof(coord *) * MAX_FONT);
  for (i=0; i<MAX_FONT; i++)
  {
    fontP->winFonts[i]   = NULL;
    fontP->fontWidth[i]  = NULL;
    fontP->fontOffset[i] = NULL;
  }
}

void
_GfxTerminate()
{
  _Gfx_DeviceInfo *deviceP;
  _FontInfo       *fontP;
  int              i;
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  deviceP = (_Gfx_DeviceInfo *)gDALGlobals->gfx.info;
  fontP   = (_FontInfo *)gDALGlobals->gfx.font_info;

  //
  // gfx shutdown
  //

  _GfxSetDrawWindow(NULL);
  _GfxDeleteWindow(gDALGlobals->gfx.window);

  free(gDALGlobals->gfx.palette);
  free(gDALGlobals->gfx.paletteDiff);
  free(gDALGlobals->gfx.iPaletteLookup);
  gDALGlobals->gfx.palette        = NULL;
  gDALGlobals->gfx.paletteDiff    = NULL;
  gDALGlobals->gfx.iPaletteLookup = NULL;

  // clean up "font" resources
  for (i=0; i<MAX_FONT; i++)
    _FntReleaseFont((font)i);
  free(fontP->winFonts);
  free(fontP->fontHeight);
  free(fontP->fontWidth);
  free(fontP->fontOffset);

  free(gDALGlobals->gfx.info);
  free(gDALGlobals->gfx.font_info);
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

gfx_window *
_GfxCreateWindow(coord width, coord height)
{
  _SymbianWindow *winPtr;

  winPtr = (_SymbianWindow *)malloc(sizeof(_SymbianWindow));
  if (winPtr != NULL)
  {
    memset(winPtr, 0, sizeof(_SymbianWindow));

    winPtr->width  = width;
    winPtr->height = height;

    uint32 size = (uint32)width * (uint32)height;
    winPtr->bits = malloc(size);
    if (winPtr->bits  != NULL) memset(winPtr->bits, 0, size);
    winPtr->allocated = true;

    // we must have bit access
    if (winPtr->bits == NULL)
    {
      free(winPtr);
      winPtr = NULL;
    }
  }

  return (gfx_window *)(winPtr);
}

gfx_window *
_GfxCreateWindowFromBuffer(coord width, coord height, uint8 *bits)
{
  _SymbianWindow *winPtr;

  winPtr = (_SymbianWindow *)malloc(sizeof(_SymbianWindow));
  if (winPtr != NULL)
  {
    memset(winPtr, 0, sizeof(_SymbianWindow));

    winPtr->width     = width;
    winPtr->height    = height;
    winPtr->bits      = bits;
    winPtr->allocated = false;
  }

  return (gfx_window *)(winPtr);
}

void
_GfxDeleteWindow(gfx_window *win)
{
  if (win != NULL)
  {
    _SymbianWindow *winPtr = (_SymbianWindow *)win;

    if ((winPtr->bits != NULL) && (winPtr->allocated)) free(winPtr->bits);
    winPtr->bits    = NULL;

    free(win);
  }
}

void
_GfxCopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  _SymbianWindow *window;
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  window = (_SymbianWindow *)win;

  if (window != NULL)
  {
    // communicate with the appview (user interface) object, update the display
    gDALGlobals->appThread->DrawToScreen(TPoint(scr_x, scr_y), window->bits, window->width, window->height,
                                      TRect(rect->x, rect->y,
                                            rect->x + rect->width, rect->y + rect->height));
  }
}

/*************************************************************************
 *
 * Library Routines
 *
 *************************************************************************/

#include "DAL-gfx-lib.inc"
#include "DAL-gfx-fonts.inc"

/********************************* EOF ***********************************/
