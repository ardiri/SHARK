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
  Win32_PlatformInfo *platform;
  uint32              size, i, j, cnt;
  uint8              *pSrc;
  uint16             *pDst, *palette, *p, val;
  GLOBALS_ACCESS;

  // update the screen
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    size = (g->device.display.width * g->device.display.height / (16 / g->device.display.depth)) * 2;

    // direct color  (direct memory copy)
    if (g->device.display.depth == 16)
    {
      // dont do anything, its already copied :)
    }

    // 12bit color   (bit shifting)
    else 
    if (g->device.display.depth == 12)
    {
      size = g->device.display.width * g->device.display.height;

      p    = g->device.display.bits;
      pDst = (uint16 *)platform->display.bits;

      // blit using bit shifing
      i = size;
      do
      {
        val = *p++;

        *pDst++ = (((val & 0x0f00) << 4) |  // 1111100000000000
                   ((val & 0x00f0) << 3) |  // 0000011111100000
                   ((val & 0x000f) << 1));  // 0000000000011111

      } while (--i);
    }

    // indexed color (color lookup table: 8bpp -> 16bpp)
    else
    {
      pSrc    = (uint8 *)g->device.display.bits;
      pDst    = (uint16 *)platform->display.bits;
      palette = (uint16 *)g->device.display.palette;
      cnt     = (8 / g->device.display.depth);

      // blit using lookup table
      i = size;
      do
      {
        p = palette + ((uint16)*pSrc++ * cnt);

        j = cnt;
        do
        { 
          *pDst++ = *p++;
        } while (--j);
      } while (--i);
    }

    // ensure boundary clipping
    if  (rect->x > g->device.display.width)                  rect->x      = g->device.display.width;
    if  (rect->y > g->device.display.height)                 rect->y      = g->device.display.height;
    if ((rect->x + rect->width)  > g->device.display.width)  rect->width  = g->device.display.width  - rect->x;
    if ((rect->y + rect->height) > g->device.display.height) rect->height = g->device.display.height - rect->y;

    // draw screen buffer to the display (just the area we are interested in)
    BitBlt(platform->windows.LCDScreen,
           rect->x, rect->y, rect->width, rect->height, platform->display.dc, rect->x, rect->y, SRCCOPY);
  }
}

/********************************* EOF ***********************************/
