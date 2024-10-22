/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxCopyToDisplay.c
 */

#include "../SHARK-prv.h"

void
_GfxCopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  GLOBALS_ACCESS;

  // entry condition (window exists)
  if (win == NULL) return;

  // do any conversion from 8bpp -> LCD depth (if required)
  if (g->gfx.fnCopyRegion != NULL)
    g->gfx.fnCopyRegion(win, rect, scr_x, scr_y);

  _LCDRepaintRegion(rect);
}

/********************************* EOF ***********************************/
