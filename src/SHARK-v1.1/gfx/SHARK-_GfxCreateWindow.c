/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxCreateWindow.c
 */

#include "../SHARK-prv.h"

gfx_window *
_GfxCreateWindow(coord width, coord height)
{
  _gfx_window *win;
  uint32       size;
  void        *bits;

  // default return value
  win = NULL;

  size = (uint32)width * (uint32)height;
  bits = (void *)_MemPtrNewX(size, false);
  if (bits != NULL)
  {
    win = (_gfx_window *)_GfxCreateWindowFromBuffer(width, height, bits);
    if (win != NULL)
    {
      _MemSemaphore(true);
      win->allocated = true;
      _MemSemaphore(false);
    }
  }

  return (gfx_window *)win;
}

/********************************* EOF ***********************************/
