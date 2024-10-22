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
  RGBColorType  sys_pal[256];
  uint8        *_p;
  int           i;
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
         for (i=0; i<256; i++)
         {
           sys_pal[i].index = i;
           sys_pal[i].r     = *_p++;
           sys_pal[i].g     = *_p++;
           sys_pal[i].b     = *_p++;
         }

         // set system palette
         WinPalette(winPaletteSet, 0, 256, (RGBColorType *)sys_pal);
         break;  
  }
}

/********************************* EOF ***********************************/
