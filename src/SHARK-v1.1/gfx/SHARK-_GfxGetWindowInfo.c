/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxGetWindowInfo.c
 */

#include "../SHARK-prv.h"

void
_GfxGetWindowInfo(gfx_window *win, uint8 **bits, coord *width, coord *height)
{
  _gfx_window *_win = (_gfx_window *)win;

  // entry conditions, NULL window doesn't make sense
  if (_win == NULL) return;

  if (width  != NULL) *width  = _win->width;
  if (height != NULL) *height = _win->height;
  if (bits   != NULL) *bits   = _win->bits;
}

/********************************* EOF ***********************************/
