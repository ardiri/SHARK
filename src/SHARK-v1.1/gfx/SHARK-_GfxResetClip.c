/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxResetClip.c
 */

#include "../SHARK-prv.h"

void
_GfxResetClip()
{
  _gfx_window *win;
  GLOBALS_ACCESS;

  // entry condition, cannot have a NULL active window
  if (g->gfx.activeWindow == NULL) return;
  win = (_gfx_window *)g->gfx.activeWindow;

  g->gfx.clip.x      = 0;
  g->gfx.clip.y      = 0;
  g->gfx.clip.width  = win->width;
  g->gfx.clip.height = win->height;
}

/********************************* EOF ***********************************/
