/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_LCDInitialize.c
 */

#include "../DAL-prv.h"

boolean
DAL_LCDInitialize()
{
  MSmob_PlatformInfo *platform;
  BITMAPINFO         *pbmi;
  uint32              size;
  boolean             init_ok;
  GLOBALS_ACCESS;

  // assume everything is ok
  init_ok = true;

  // verify the device + platform structures are valid
  platform = (MSmob_PlatformInfo *)g->platform;
  init_ok = ((platform != NULL)); if (!init_ok) goto LCD_INIT_ABORT;

#ifdef GAPI_SUPPORT
  // open the GAPI display
  if (platform->gapi.GXOpenDisplay != NULL)
    platform->gapi.GXOpenDisplay(platform->windows.LCDWnd, 1);
#endif

  pbmi = (BITMAPINFO *)&platform->display.bmi.bmiHeader;
  pbmi->bmiHeader.biClrImportant  = 0;
  pbmi->bmiHeader.biClrUsed       = 0;
  pbmi->bmiHeader.biCompression   = BI_BITFIELDS;
  pbmi->bmiHeader.biPlanes        = 1;
  pbmi->bmiHeader.biBitCount      = 16;
  pbmi->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
  pbmi->bmiHeader.biHeight        = -(LONG)(g->device.display.height);
  pbmi->bmiHeader.biWidth         =  (LONG)(g->device.display.width);
  pbmi->bmiHeader.biSizeImage     = ((((((LONG)(g->device.display.width) * pbmi->bmiHeader.biBitCount) + 31) & ~(31)) / 8) * (LONG)(g->device.display.height));
  pbmi->bmiHeader.biXPelsPerMeter = 0;
  pbmi->bmiHeader.biYPelsPerMeter = 0;

  // configure the bitmap to use the currently active palette (important)
  platform->display.bmi.bmiColors[0] = (SHORT)0x1f << 11;   // 5:6:5
  platform->display.bmi.bmiColors[1] = (SHORT)0x3f << 5; 
  platform->display.bmi.bmiColors[2] = (SHORT)0x1f;

  platform->display.dc  = CreateCompatibleDC(platform->windows.LCDScreen);
  platform->display.bmp = CreateDIBSection(platform->display.dc, pbmi, DIB_RGB_COLORS, ((void **)(&platform->display.bits)), NULL, 0);
  SelectObject(platform->display.dc, platform->display.bmp);

  // initialize window to white (or, report error)
  if ((platform->display.dc == NULL) || (platform->display.bmp == NULL) || (platform->display.bits == NULL))
  {
    init_ok = false; goto LCD_INIT_ABORT;
  }
  
  // fill the window with white color
  size = g->device.display.width * g->device.display.height * 2; // 16bpp
  SYSTEM_MemSet(platform->display.bits, size, 0xff);

  // we know we are dealing with 16bpp - so, can assume the following
  g->device.display.bits    = NULL; 
  g->device.display.palette = NULL;
  
  // everything ok?
  goto LCD_INIT_SUCCESS;

LCD_INIT_ABORT:

  // clean up memory usage within memory management system
  if (platform->display.bmp != NULL) DeleteObject(platform->display.bmp);
  if (platform->display.dc != NULL)  DeleteDC(platform->display.dc);
  g->device.display.bits    = NULL;
  g->device.display.palette = NULL;
  platform->display.bmp     = NULL;
  platform->display.dc      = NULL;

LCD_INIT_SUCCESS:

  return init_ok;
}

/********************************* EOF ***********************************/
