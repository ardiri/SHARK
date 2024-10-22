/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxGetPalette.c
 */

#include "../SHARK-prv.h"

void
_GfxGetPalette(uint8 *rgb)
{
  GLOBALS_ACCESS;

  // entry conditions, cannot have NULL pointers
  if ((rgb == NULL) || (g->gfx.palette == NULL)) return;

  // copy over palette entries
  _MemMove(rgb, g->gfx.palette, (PALETTE_SIZE * 3));
}

/********************************* EOF ***********************************/
