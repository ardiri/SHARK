/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxSetDrawWindow.c
 */

#include "../SHARK-prv.h"

void
_GfxSetDrawWindow(gfx_window *win)
{
  _gfx_window *_win = (_gfx_window *)win;
  GLOBALS_ACCESS;
  
  // entry conditions, NULL window or bits doesn't make sense
  if ((_win == NULL) || (_win->bits == NULL)) return;

  g->gfx.activeWindow = win;
  _GfxResetClip();
}

/********************************* EOF ***********************************/
