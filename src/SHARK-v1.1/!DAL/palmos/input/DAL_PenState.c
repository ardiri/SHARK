/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PenState.c
 */

#include "../DAL-prv.h"

void
DAL_PenState(coord *penX, coord *penY, boolean *penDown)
{
  PALM_PlatformInfo *platform;
  Coord              x, y;
  Boolean            down;
  GLOBALS_ACCESS;

  // default values
  if (penX != NULL) *penX = 0;
  if (penY != NULL) *penY = 0;

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    if (penDown != NULL) 
    {
      switch (platform->device.type)
      {
        case DEVICE_PALM_HDD:
             // get the pen state
             EvtGetPenNative(WinGetDisplayWindow(), &x, &y, &down);
             break;

        default:
             // get the pen state
             EvtGetPen(&x, &y, &down);
 
             // adjust as required
                  if (platform->device.density == DENSITY_DOUBLE)     { x = x << 1;       y = y << 1;       }
             else if (platform->device.density == DENSITY_ONEANDHALF) { x = (x * 3) >> 1; y = (y * 3) >> 1; }
             break;
      }

      *penDown = (boolean)down;
  
      // lets ensure the pen is within the screen boundaries 
      if ((*penDown) &&
          ((x > 0) &&
           (y > 0) &&
           (x < (coord)g->device.display.width) &&
           (y < (coord)g->device.display.height)))
      {
        if (penX != NULL) *penX = (coord)x;
        if (penY != NULL) *penY = (coord)y;
      }
      else
        *penDown = false;
    }
  }
}

/********************************* EOF ***********************************/
