/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_LCDSetPalette.c
 */

#include "../DAL-prv.h"

void
DAL_LCDSetPalette(uint8 *palette)
{
  uint32  i;
  uint8   _r, _g, _b;
  uint8  *_p;
  uint16 * p;
  GLOBALS_ACCESS;

  // entry condition - no null pointers
  if (palette == NULL) return;

  // generate lookup tables (only valid for 8bpp)
  switch (g->device.display.depth)
  {
    case 1:
    case 2:
    case 4:
    case 12:
    case 16:
    default:
         break;  

    case 8:
         _p = palette;
          p = g->device.display.palette;
         for (i=0; i<256; i++)
         {
           _r = *_p++;
           _g = *_p++;
           _b = *_p++;

           *p++ = (((uint16)(_r & 0xF8) << 8) |   // 1111100000000000
                   ((uint16)(_g & 0xFC) << 3) |   // 0000011111100000
                   ((uint16)(_b & 0xF8) >> 3));   // 0000000000011111
         }
         break;  
  }
}

/********************************* EOF ***********************************/
