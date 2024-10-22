/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxGetPaletteIndex.c
 */

#include "../SHARK-prv.h"

color
_GfxGetPaletteIndex(uint8 _r, uint8 _g, uint8 _b)
{
  uint8  *pal_ptr;
  int     index, i;
  uint32  lowValue, *diff, diffValue;
  int     rd, gd, bd;
  GLOBALS_ACCESS;

  // default value
  index = 0;

  // cannot have null pointers
  if ((g->gfx.palette != NULL) && (g->gfx.paletteDiff != NULL))
  {
    pal_ptr = (uint8 *)g->gfx.palette;
  
    // generate the color "differences" for all colors in the palette
    _MemSemaphore(true);
    diff = g->gfx.paletteDiff;
    i    = PALETTE_SIZE;
    do
    {
      bd = ((int)*pal_ptr++ - _r);
      gd = ((int)*pal_ptr++ - _g);
      rd = ((int)*pal_ptr++ - _b);
  
      *diff++ = ((int32)rd * rd) + ((int32)gd * gd) + ((int32)bd * bd);
    }
    while (--i);
    _MemSemaphore(false);
  
    // find the palette index that has the smallest color "difference"
    lowValue = 0xffffffff;
  
    i     = PALETTE_SIZE;
    index = i-1;
    do
    {
      diff--;
      diffValue = *diff;
      if (diffValue < lowValue)
      {
        lowValue = diffValue;
        index = (i-1);
      }
    }
    while (--i);
  }

  // special purpose cases
  if ((_r == 255) && (_g == 255) && (_b == 255)) index = 0;
  if ((_r ==   0) && (_g ==   0) && (_b ==   0)) index = 255;

  return (color)index;
}

/********************************* EOF ***********************************/
