/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-display-GAPI.support
 */

// TODO: 
// -----
//
// 480x640 + 640x480
// - need to provide support for hi-res native GAPI environments
//
// GAPI DRAM 
// - use GXIsDisplayDRAMBuffer to avoid tripple buffering
//
// non-linear framebuffer
// - use GXGetDisplayProperties() 
// - gapi.props.cbxPitch / gapi.props.cbyPitch adjustments to code
// 

/*************************************************************************
 *
 * GAPI Interface
 *
 *************************************************************************/

#define GX_FULLSCREEN 0x01

HINSTANCE GXlibRef;

static int32     (*GXOpenDisplay)(HWND hWnd, DWORD dwFlags);
static int32     (*GXCloseDisplay)();
static void     *(*GXBeginDraw)();
static int32     (*GXSuspend)();
static int32     (*GXResume)();
static int32     (*GXEndDraw)();

#define GAPI_FN(fn) (void *)GetProcAddress(GXlibRef, fn)

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

/**
 ** /Program Files/Windows CE Tools/wce300/Pocket PC 2002/target/arm
 ** $ dumpbin /exports gx.dll
 **
 ** >> ?GXOpenDisplay@@YAHPAUHWND__@@K@Z
 ** >> ?GXCloseDisplay@@YAHXZ
 ** >> ?GXBeginDraw@@YAPAXXZ
 ** >> ?GXEndDraw@@YAHXZ
 ** >> ?GXSuspend@@YAHXZ
 ** >> ?GXResume@@YAHXZ
 **
 ** ?GXCloseInput@@YAHXZ
 ** ?GXGetDefaultKeys@@YA?AUGXKeyList@@H@Z
 ** ?GXIsDisplayDRAMBuffer@@YAHXZ
 ** ?GXOpenInput@@YAHXZ
 ** ?GXSetViewport@@YAHKKKK@Z
 ** ?GXGetDisplayProperties@@YA?AUGXDisplayProperties@@XZ
 **/

static void
_GAPIInitialize()
{
  GXlibRef = LoadLibrary(_T("gx.dll"));
  if (GXlibRef != NULL)
  {
    GXOpenDisplay  = GAPI_FN(_T("GXOpenDisplay@@YAHPAUHWND__@@K@Z"));
    GXCloseDisplay = GAPI_FN(_T("GXCloseDisplay@@YAHXZ"));
    GXBeginDraw    = GAPI_FN(_T("GXBeginDraw@@YAPAXXZ"));
    GXSuspend      = GAPI_FN(_T("GXSuspend@@YAHXZ"));
    GXResume       = GAPI_FN(_T("GXResume@@YAHXZ"));
    GXEndDraw      = GAPI_FN(_T("GXEndDraw@@YAHXZ"));
  }
  else
  {
    GXOpenDisplay  = NULL;
    GXCloseDisplay = NULL;
    GXBeginDraw    = NULL;
    GXSuspend      = NULL;
    GXResume       = NULL;
    GXEndDraw      = NULL;
  }

  // open the GAPI display
  if (GXOpenDisplay != NULL) GXOpenDisplay(g_hWnd, GX_FULLSCREEN);
}

static void
_GAPISuspend()
{
  if (GXSuspend != NULL) GXSuspend();
}

static void
_GAPIResume()
{
  if (GXResume != NULL) GXResume();
}

static void
_GAPIBitBlt16(coord x, coord y, int16 width, int16 height)
{
  int16   i, j;
  uint16 *pLCD, *pBMP;

  // initialize our pointers
  pBMP = (uint16 *)g_lcd_bits    + (((uint32)y * g_width) + x);
  pLCD = (uint16 *)GXBeginDraw() + (((uint32)y * g_width) + x);

  // do the inner blit of the rectangular area
  j = height;
  while (j--)
  {
    i = width;
    while (i--)
      *pLCD++ = *pBMP++;

    pLCD += (g_width - width);
    pBMP += (g_width - width);
  }

  GXEndDraw();
}

static void
_GAPIBitBlt32(coord x, coord y, int16 width, int16 height)
{
  int16   i, j;
  uint32 *pLCD, *pBMP;

  // initialize our pointers
  pBMP = (uint32 *)g_lcd_bits    + (((uint32)y * g_width) + x);
  pLCD = (uint32 *)GXBeginDraw() + (((uint32)y * g_width) + x);

  // do the inner blit of the rectangular area
  j = height;
  while (j--)
  {
    i = width;
    while (i--)
      *pLCD++ = *pBMP++;

    pLCD += (g_width - width);
    pBMP += (g_width - width);
  }

  GXEndDraw();
}

static void
_GAPITerminate()
{
  // close the GAPI display
  if (GXCloseDisplay != NULL) GXCloseDisplay();

  if (GXlibRef) FreeLibrary(GXlibRef);
  GXlibRef = NULL;
}

#undef GAPI_FN

/********************************* EOF ***********************************/
