/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_LCDRepaintRegion.c
 */

#include "../DAL-prv.h"

void
DAL_LCDRepaintRegion(rectangle *rect)
{
  MSmob_PlatformInfo *platform;
  GLOBALS_ACCESS;

  // update the screen
  platform = (MSmob_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // ensure boundary clipping
    if  (rect->x > g->device.display.width)                  rect->x      = g->device.display.width;
    if  (rect->y > g->device.display.height)                 rect->y      = g->device.display.height;
    if ((rect->x + rect->width)  > g->device.display.width)  rect->width  = g->device.display.width  - rect->x;
    if ((rect->y + rect->height) > g->device.display.height) rect->height = g->device.display.height - rect->y;

    // draw screen buffer to the display (just the area we are interested in)
#ifdef GAPI_SUPPORT
    if ((platform->gapi.GXBeginDraw != NULL) && (platform->gapi.GXEndDraw != NULL))
    {
      // start actual drawing
      uint16 *pLCD, *pBMP;
      int     i, j, offset, rb;

      offset = (rect->y * g->device.display.width) + rect->x;
      rb     = g->device.display.width - rect->width;

      pBMP = (g->device.display.bits == NULL) ? platform->display.bits : g->device.display.bits;
      pLCD = (uint16 *)platform->gapi.GXBeginDraw();

      pLCD += offset; 
      pBMP += offset;

      j = rect->height;
      while (j--)
      {
        i = rect->width;
        while (i--)
          *pLCD++ = *pBMP++;       // this assumes 5:6:5 display

        pLCD += rb;
        pBMP += rb;
      }

      platform->gapi.GXEndDraw();
    }
    else
#endif
      BitBlt(platform->windows.LCDScreen,
             rect->x, rect->y, rect->width, rect->height, platform->display.dc, rect->x, rect->y, SRCCOPY);
  }
}

/********************************* EOF ***********************************/
