/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxTerminate.c
 */

#include "../SHARK-prv.h"

void
_GfxTerminate()
{
  int i;
  GLOBALS_ACCESS;

  // clean up "font" resources
  for (i=0; i<MAX_FONT; i++)
    _FntReleaseFont((font)i);

  if (g->gfx.font.fontOffset  != NULL) _MemPtrFree(g->gfx.font.fontOffset);
  if (g->gfx.font.fontWidth   != NULL) _MemPtrFree(g->gfx.font.fontWidth);
  if (g->gfx.font.fontHeight  != NULL) _MemPtrFree(g->gfx.font.fontHeight);
  if (g->gfx.font.winFonts    != NULL) _MemPtrFree(g->gfx.font.winFonts);
  if (g->gfx._indexed         != NULL) _MemPtrFree(g->gfx._indexed);
  if (g->gfx._direct          != NULL) _MemPtrFree(g->gfx._direct);
  if (g->gfx.paletteDiff      != NULL) _MemPtrFree(g->gfx.paletteDiff);
  if (g->gfx.palette          != NULL) _MemPtrFree(g->gfx.palette);
  if (g->gfx.window           != NULL) _GfxDeleteWindow(g->gfx.window);
  g->gfx.font.fontOffset  = NULL;
  g->gfx.font.fontWidth   = NULL;
  g->gfx.font.fontHeight  = NULL;
  g->gfx.font.winFonts    = NULL;
  g->gfx._indexed         = NULL;
  g->gfx._direct          = NULL;
  g->gfx.paletteDiff      = NULL;
  g->gfx.palette          = NULL;
  g->gfx.window           = NULL;
}

/********************************* EOF ***********************************/
