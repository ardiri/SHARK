/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_UIMenuReset.c
 */

#include "../SHARK-prv.h"

void
_UIMenuReset()
{
  GLOBALS_ACCESS;

  // reset all the menu variables
  g->ui.menu.visible            = false;
  g->ui.menu.special            = false;
  g->ui.menu.index              = _key_unknown;
  g->ui.menu.count              = 0;
  _StrCopy(g->ui.menu.button[0].label, "");
  _StrCopy(g->ui.menu.button[1].label, "");
  g->ui.menu.button[0].key_code = _key_unknown;
  g->ui.menu.button[1].key_code = _key_unknown;
  g->ui.menu.rect.x             = 0;
  g->ui.menu.rect.y             = 24; // 0;
  g->ui.menu.rect.width         = 0;
  g->ui.menu.rect.height        = 2;

  // no font defined? lets get outa here
  if (g->ui.menu.font_id == font_undefined) return;

  // this is the 'pen response' area
  if (_PenAvailable())
  {
    g->ui.menu.button[0].rect.x      = 0;
    g->ui.menu.button[0].rect.y      = 0;
    g->ui.menu.button[0].rect.width  = g->device.display.width >> 1;
    g->ui.menu.button[0].rect.height = _UIMenuGetHeight();  // top right corner
  }
}

/********************************* EOF ***********************************/
