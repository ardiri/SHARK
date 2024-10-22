/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxCreateWindowFromBuffer.c
 */

#include "../SHARK-prv.h"

gfx_window *
_GfxCreateWindowFromBuffer(coord width, coord height, uint8 *bits)
{
  _gfx_window *win;

  // default return value
  win = NULL;

  if (bits != NULL)
  {
    win = (_gfx_window *)_MemPtrNew(sizeof(_gfx_window), true);
    if (win != NULL)
    {
      _MemSemaphore(true);
      _MemSet(win, sizeof(_gfx_window), 0);
      win->width     = width;
      win->height    = height;
      win->bits      = bits;
      win->allocated = false;
      _MemSemaphore(false);
    }
  }

  return (gfx_window *)win;
}

/********************************* EOF ***********************************/
