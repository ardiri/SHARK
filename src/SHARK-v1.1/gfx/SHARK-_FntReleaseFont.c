/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FntReleaseFont.c
 */

#include "../SHARK-prv.h"

void
_FntReleaseFont(font id)
{
  GLOBALS_ACCESS;

  // lets make sure is defined.
  if (g->gfx.font.winFonts[id] != NULL)
  {
    if (g->gfx.font.winFonts[id]   != NULL) _GfxDeleteWindow(g->gfx.font.winFonts[id]);
    if (g->gfx.font.fontOffset[id] != NULL) _MemPtrFree(g->gfx.font.fontOffset[id]);

    _MemSemaphore(true);
    g->gfx.font.winFonts[id]       = NULL;
    g->gfx.font.fontOffset[id]     = NULL;
    g->gfx.font.fontWidth[id]      = NULL;
    _MemSemaphore(false);
  }
}

/********************************* EOF ***********************************/
