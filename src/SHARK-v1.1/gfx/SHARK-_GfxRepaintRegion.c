/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxRepaintRegion.c
 */

#include "../SHARK-prv.h"

void
_GfxRepaintRegion(rectangle *rect)
{
  GLOBALS_ACCESS;

  // entry condition (screen access disabled)
  if (g->gfx.window == NULL) return;

  _GfxCopyToDisplay(g->gfx.window, rect, rect->x, rect->y);
}

/********************************* EOF ***********************************/
