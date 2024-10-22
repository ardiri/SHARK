/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_UIMenuGetHeight.c
 */

#include "../SHARK-prv.h"

coord
_UIMenuGetHeight()
{
  coord height;
  GLOBALS_ACCESS;

  // no font defined? lets get outa here
  if (g->ui.menu.font_id == font_undefined) return 0;

  _FntSetFont(g->ui.menu.font_id);
  height = (_FntGetFontHeight() + MENU_SPACE_Y + 1) & ~0x01; // ensure even (round up)

  return height;
}

/********************************* EOF ***********************************/
