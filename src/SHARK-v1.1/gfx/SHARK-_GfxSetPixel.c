/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxSetPixel.c
 */

#include "../SHARK-prv.h"

void
_GfxSetPixel(coord x, coord y, color c)
{
  _gfx_window *win;
  uint8       *ptr;
  GLOBALS_ACCESS;

  // entry condition, cannot have a NULL active window
  if (g->gfx.activeWindow == NULL) return;
  win = (_gfx_window *)g->gfx.activeWindow;

  if ((x >= g->gfx.clip.x) &&
      (x < (g->gfx.clip.x + g->gfx.clip.width)) &&
      (y >= g->gfx.clip.y) &&
      (y < (g->gfx.clip.y + g->gfx.clip.height)))
  {
    ptr  = win->bits;
    ptr += ((uint32)y * win->width) + x;

    *ptr = c;
  }
}

/********************************* EOF ***********************************/
