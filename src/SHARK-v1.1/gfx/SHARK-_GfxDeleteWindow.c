/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxDeleteWindow.c
 */

#include "../SHARK-prv.h"

void
_GfxDeleteWindow(gfx_window *win)
{
  _gfx_window *_win = (_gfx_window *)win;

  // entry condition - no NULL windows please
  if (_win == NULL) return;

  // clean up!
  if (_win->allocated)
  {
    if (_win->bits != NULL) _MemPtrFree(_win->bits);
    _MemSemaphore(true);
    _win->bits = NULL;
    _MemSemaphore(false);
  }

  _MemPtrFree(_win);
}

/********************************* EOF ***********************************/
