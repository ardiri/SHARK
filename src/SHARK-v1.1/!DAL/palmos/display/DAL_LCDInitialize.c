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
  boolean            success = true;
#ifdef _M_ARM
  PALM_PlatformInfo *platform;
  Err                err;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    switch (platform->device.type)
    {
      case DEVICE_GENERIC:
           // create the base bitmap
           platform->gfx.bmp =
             BmpCreate((Coord)g->device.display.width, (Coord)g->device.display.height, (UInt8)g->device.display.depth, NULL, &err);

           // create the bitmap window
           if ((platform->gfx.bmp != NULL) && (err == errNone))
             platform->gfx.win = WinCreateBitmapWindow(platform->gfx.bmp, &err);

           // lets verify everything worked out ok
           success &= ((platform->gfx.bmp != NULL) && (platform->gfx.win != NULL) && (err == errNone));
           break;

      case DEVICE_HANDERA:

           // there was never any device made with ARM processor
           success = false; 

           break;

      case DEVICE_SONY:

           // create the base bitmap
           platform->gfx.bmp =
             HRBmpCreate(platform->device.libRef[0],
                         (Coord)g->device.display.width, (Coord)g->device.display.height, (UInt8)g->device.display.depth, NULL, &err);

           // create the bitmap window
           if (err == errNone)
             platform->gfx.win =
               HRWinCreateBitmapWindow(platform->device.libRef[0], platform->gfx.bmp, &err);

           // lets verify everything worked out ok
           success &= ((platform->gfx.bmp != NULL) && (platform->gfx.win != NULL) && (err == errNone));
           break;

      case DEVICE_PALM_HDD:
           // create the base bitmap
           platform->gfx.bmp =
             BmpCreate((Coord)g->device.display.width, (Coord)g->device.display.height, (UInt8)g->device.display.depth, NULL, &err);

           if ((platform->gfx.bmp != NULL) && (err == errNone))
           {
             UInt16 density;

             // we need to ensure we create with the right density
             density = kDensityLow; // default
             switch (platform->device.density)
             {
               case DENSITY_SINGLE:      density = kDensityLow;         break;
               case DENSITY_ONEANDHALF:  density = kDensityOneAndAHalf; break;
               case DENSITY_DOUBLE:      density = kDensityDouble;      break;
               default:
                    break;
             } 

             // create the V3 bitmap (needed for hires)
             platform->gfx.bmpV3 =
               BmpCreateBitmapV3(platform->gfx.bmp, density,
                                 BmpGetBits(platform->gfx.bmp), NULL);
             err = (platform->gfx.bmpV3 == NULL);

             // create the bitmap window
             if (err == errNone)
               platform->gfx.win =
                 WinCreateBitmapWindow((BitmapType *)platform->gfx.bmpV3, &err);
           }

           // lets verify everything worked out ok
           success &= ((platform->gfx.bmp != NULL) && (platform->gfx.bmpV3 != NULL) && (platform->gfx.win != NULL) && (err == errNone));
           break;

      default:
           success = false; // WTF? (unit doesn't exist)
           break;
    }

    // did we suceed?
    if (success)
      g->device.display.bits = BmpGetBits(WinGetBitmap(platform->gfx.win));
  }
#endif

  return success;
}

/********************************* EOF ***********************************/
