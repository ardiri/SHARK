/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxClearWindow.c
 */

#include "../SHARK-prv.h"

void
_GfxClearWindow()
{
  GLOBALS_ACCESS;

  _GfxFillRegion(&g->gfx.clip, _GfxGetPaletteIndex(255, 255, 255));
}

/********************************* EOF ***********************************/
