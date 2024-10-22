/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxGetPixel.c
 */

#include "../SHARK-prv.h"

color
_GfxGetPixel(coord x, coord y)
{
  _gfx_window *win;
  uint8       *ptr;
  color        result;
  GLOBALS_ACCESS;

  // default return value
  result = 0;

  // can only do this on an active window
  if (g->gfx.activeWindow != NULL)
  {
    win = (_gfx_window *)g->gfx.activeWindow;

    if ((x >= g->gfx.clip.x) &&
        (x < (g->gfx.clip.x + g->gfx.clip.width)) &&
        (y >= g->gfx.clip.y) &&
        (y < (g->gfx.clip.y + g->gfx.clip.height)))
    {
      ptr  = win->bits;
      ptr += ((uint32)y * win->width) + x;
  
      result = *ptr;
    }
  }

  return result;
}

/********************************* EOF ***********************************/
