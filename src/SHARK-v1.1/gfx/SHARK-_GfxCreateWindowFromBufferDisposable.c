/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxCreateWindowFromBufferDisposable.c
 */

#include "../SHARK-prv.h"

gfx_window *
_GfxCreateWindowFromBufferDisposable(coord width, coord height, uint8 *bits)
{
  _gfx_window *win;

  win = (_gfx_window *)_GfxCreateWindowFromBuffer(width, height, bits);
  if (win != NULL) 
  {
    _MemSemaphore(true);
    win->allocated = true;
    _MemSemaphore(false);
  }

  return (gfx_window *)win;
}

/********************************* EOF ***********************************/
