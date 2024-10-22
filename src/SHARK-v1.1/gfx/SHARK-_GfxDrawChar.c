/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxDrawChar.c
 */

#include "../SHARK-prv.h"

void
_GfxDrawChar(uint8 c, coord x, coord y, draw_operation mode)
{
  gfx_window *winSrc, *winDst;
  rectangle   rect;
  GLOBALS_ACCESS;

  // entry condition, cannot have a NULL active window/font
  if ((g->gfx.activeWindow == NULL) || (g->gfx.font.winFonts[g->gfx.activeFont] == NULL)) return;

  // configure
  winSrc = g->gfx.font.winFonts[g->gfx.activeFont];
  winDst = g->gfx.activeWindow;

  rect.x      = g->gfx.font.fontOffset[g->gfx.activeFont][c];
  rect.y      = 0;
  rect.width  = g->gfx.font.fontWidth[g->gfx.activeFont][c];
  rect.height = g->gfx.font.fontHeight[g->gfx.activeFont];

  // copy over the character bitmap!
  _GfxCopyRegion(winSrc, winDst, &rect, x, y, mode);
}

/********************************* EOF ***********************************/
