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
#ifdef _M_ARM
  PALM_PlatformInfo *platform;
  RectangleType      _rect;
  GLOBALS_ACCESS;

  // configure region to repaint
  _rect.topLeft.x = rect->x;
  _rect.topLeft.y = rect->y;
  _rect.extent.x  = rect->width;
  _rect.extent.y  = rect->height;

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    switch (platform->device.type)
    {
      case DEVICE_GENERIC:
           WinCopyRectangle(platform->gfx.win, WinGetDisplayWindow(),
                            &_rect, (Coord)rect->x, (Coord)rect->y, winPaint);
           break;
 
      case DEVICE_PALM_HDD:
           WinSetCoordinateSystem(kCoordinatesNative);
           WinCopyRectangle(platform->gfx.win, WinGetDisplayWindow(),
                            &_rect, (Coord)rect->x, (Coord)rect->y, winPaint);
           WinSetCoordinateSystem(kCoordinatesStandard);
           break;

      case DEVICE_SONY:
           HRWinCopyRectangle(platform->device.libRef[0],
                              platform->gfx.win, WinGetDisplayWindow(),
                              &_rect, (Coord)rect->x, (Coord)rect->y, winPaint);
           break;

      default:
           break;
    }
  }
#endif
}

/********************************* EOF ***********************************/
