/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-display.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define LCD_GLOBALS           g_lcd
#define GLOBALS_UNINITIALIZED (LCD_GLOBALS == NULL)
#define GLOBALS_DEFINITION    LCDGlobals *g_lcd = NULL

// LCDGlobals::   platform additions
#define DAL_LCD_GLOBALS \
  HDC              lcd_dc;           \
                                     \
  HBITMAP          lcd_bmp;          \
  BITMAPINFOHEADER lcd_bmiHeader;    \
  DWORD            lcd_bmiColors[4];
                                     
#define g_lcd_dc               (LCD_GLOBALS) -> lcd_dc

#define g_lcd_bmp              (LCD_GLOBALS) -> lcd_bmp
#define g_lcd_bmiHeader        (LCD_GLOBALS) -> lcd_bmiHeader
#define g_lcd_bmiColors        (LCD_GLOBALS) -> lcd_bmiColors

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-display.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
DAL_LCDInitialize(uint16 width, uint16 height)
{
  boolean init_ok;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(LCD_GLOBALS, 0, sizeof(LCDGlobals));

  g_lcd_type      = lcd_direct;
  g_lcd_depth     = 16;
  g_lcd_depth_bus = (uint16)((g_lcd_depth + 15) & ~0xf);
  g_lcd_a         = 0;
  g_lcd_r         = 5;
  g_lcd_g         = 6;
  g_lcd_b         = 5;
  g_lcd_a_mask    = 0;
  g_lcd_r_mask    = 0xf800;
  g_lcd_g_mask    = 0x07e0;
  g_lcd_b_mask    = 0x001f;

  g_lcd_inverted  = false;

  // lets determine the DPI of the display
  g_lcd_dpi       = lcd_dpi96;

  return init_ok;
}

static void
DAL_LCDInitialize_RESIZE(uint16 width, uint16 height)
{
  BITMAPINFO *pbmi;
  uint32      size;

  // now we know the size of the display
  g_lcd_width     = width;
  g_lcd_width_rb  = (((g_lcd_width  + 3) & ~3) * (g_lcd_depth_bus >> 3));
  g_lcd_height    = height;
  g_lcd_height_rb = (((g_lcd_height + 3) & ~3) * (g_lcd_depth_bus >> 3));

  // calculate the size of the window
  size = ((uint32)g_lcd_width_rb * g_lcd_height);

  // initialize the bitmap palette
  g_lcd_bmiColors[0] = g_lcd_r_mask;
  g_lcd_bmiColors[1] = g_lcd_g_mask;
  g_lcd_bmiColors[2] = g_lcd_b_mask;

  // verify the device + platform structures are valid
  pbmi = (BITMAPINFO *)&g_lcd_bmiHeader;
  pbmi->bmiHeader.biClrImportant  = 0;
  pbmi->bmiHeader.biClrUsed       = 0;
  pbmi->bmiHeader.biCompression   = BI_BITFIELDS;
  pbmi->bmiHeader.biPlanes        = 1;
  pbmi->bmiHeader.biBitCount      = g_lcd_depth;
  pbmi->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
  pbmi->bmiHeader.biWidth         =  (LONG)g_lcd_width;
  pbmi->bmiHeader.biHeight        = -(LONG)g_lcd_height;
  pbmi->bmiHeader.biSizeImage     =  (LONG)size;
  pbmi->bmiHeader.biXPelsPerMeter = 3760;
  pbmi->bmiHeader.biYPelsPerMeter = 3760;

  g_lcd_dc  = CreateCompatibleDC(GetDC(g_hWnd));
  g_lcd_bmp = CreateDIBSection(g_lcd_dc, pbmi, DIB_RGB_COLORS,
                               ((void **)(&g_lcd_framebuffer_dir)), NULL, 0);
  SelectObject(g_lcd_dc, g_lcd_bmp);

  // we have a static LCD address and it is double buffering?
  g_lcd_doublebuffered  = true;
  g_lcd_requirelock     = false;
}

static void
DAL_LCDTerminate_RESIZE()
{
  // remove screen buffer
  if (g_lcd_bmp != NULL) DeleteObject(g_lcd_bmp);
  if (g_lcd_dc  != NULL) DeleteDC(g_lcd_dc);
  g_lcd_bmp = NULL;
  g_lcd_dc  = NULL;
}

static void
DAL_LCDFlipDisplay()
{
  HDC hWnd_dc;

  // entry requirements
  if (g_hWnd == NULL) return;

  // get the drawing context
  hWnd_dc = GetDC(g_hWnd);

  // draw screen buffer to the display 
  SetStretchBltMode(hWnd_dc, HALFTONE);
  StretchBlt(hWnd_dc,  0, 0, 
                       APPLICATION_WINDOW_WIDTH, APPLICATION_WINDOW_HEIGHT, 
             g_lcd_dc, 0, 0, 
                       g_lcd_width, g_lcd_height,
             SRCCOPY);

  // release the drawing context
  ReleaseDC(g_hWnd, hWnd_dc);
}

static void *
DAL_LCDLock()
{
  return NULL;
}

static void
DAL_LCDResume()
{
  // windows - no resume options, handled by window manager
}

static void
DAL_LCDSuspend()
{
  // windows - no suspend options, handled by window manager
}

static void
DAL_LCDUnlock()
{
}

static void 
DAL_LCDTerminate()
{
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef LCD_GLOBALS

/********************************* EOF ***********************************/
