/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_UIMenuSetFont.c
 */

#include "../SHARK-prv.h"

void
_UIMenuSetFont(font font_id)
{
  GLOBALS_ACCESS;

  // we can only do this if a font has been defined
  if (g->gfx.font.winFonts[font_id] != NULL)
  {
    g->ui.menu.font_id = font_id;

    // adjust menu spacing based on font selected
    if (font_id != font_undefined)
    {
      _FntSetFont(g->ui.menu.font_id);
      g->ui.menu.spacing = (((_FntGetFontHeight() * 3) / 5) + 1) & ~0x01;
    }
  }
}

/********************************* EOF ***********************************/
