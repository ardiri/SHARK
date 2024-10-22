/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxRepaint.c
 */

#include "../SHARK-prv.h"

void
_GfxRepaint()
{
  rectangle rect;
  GLOBALS_ACCESS;

  // entry condition (screen access disabled)
  if (g->gfx.window == NULL) return;

  rect.x      = 0;
  rect.y      = 0;
  rect.width  = (coord)g->device.display.width;
  rect.height = (coord)g->device.display.height;

  _GfxCopyToDisplay(g->gfx.window, &rect, 0, 0);
}

/********************************* EOF ***********************************/
