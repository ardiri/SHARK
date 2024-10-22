/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_UIMenuSetButton.c
 */

#include "../SHARK-prv.h"

void
_UIMenuSetButton(uint8 pos, uint8 *label, uint8 key_code)
{
  GLOBALS_ACCESS;

  // no font defined? lets get outa here
  if (g->ui.menu.font_id == font_undefined) return;

  // only valid under these conditions
  if (!_PenAvailable())
  {
    if ((pos == 0) || (pos == 1))
    {
      _MemSet(g->ui.menu.button[pos].label, 32, 0);
      _StrNCopy(g->ui.menu.button[pos].label, label, 31);  // max 31 chars
      g->ui.menu.button[pos].key_code = key_code;
    }
  }

  // DEVELOPER ERROR: warn them
  else
    _SysDebugMessage("_UIMenuSetButton() cannot be used when stylus exists", true);
}

/********************************* EOF ***********************************/
