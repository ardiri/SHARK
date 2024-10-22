/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_UIMenuAddItem.c
 */

#include "../SHARK-prv.h"

void
_UIMenuAddItem(uint8 *label, uint8 key_code, uint8 *param)
{
  int    i, cnt, tmp_width;
  uint8 *pStr;
  GLOBALS_ACCESS;

  // no font defined? lets get outa here
  if (g->ui.menu.font_id == font_undefined) return;

  // sorry :)
  if ((label[0] == '-') && (g->ui.menu.count == 0)) return;

  if ((!g->ui.menu.visible) && (g->ui.menu.count < MAX_MENUS))
  {
    tmp_width = 0;

    _MemSet(g->ui.menu.item[g->ui.menu.count].label, 32, 0);
    _MemSet(g->ui.menu.item[g->ui.menu.count].label, 64, 0);
    _StrNCopy(g->ui.menu.item[g->ui.menu.count].label, label, 31); // max 32 chars
    if (param == NULL)
      g->ui.menu.item[g->ui.menu.count].nested.count = 0;
    else
    if ((param[1] == ':') && (param[3] == ':')) // validation
    {
      g->ui.menu.special = true;
      g->ui.menu.item[g->ui.menu.count].nested.count = (uint8)_StrAToI(param);
      param += 2; // skip the ":"
      g->ui.menu.item[g->ui.menu.count].nested.index = (uint8)_StrAToI(param);
      param += 2; // skip the ":"

      cnt       = 0; 
      pStr      = param;
      for (i=0; i<g->ui.menu.item[g->ui.menu.count].nested.count; i++)
      {
        _StrNCopy(&g->ui.menu.item[g->ui.menu.count].nested.labels[cnt], pStr, (int16)(63 - cnt));  // max 64 chars
        if (tmp_width < (_FntGetCharsWidth(pStr, _StrLen(pStr))))
          tmp_width = _FntGetCharsWidth(pStr, _StrLen(pStr));
        cnt  += _StrLen(pStr) + 1; 
        pStr += _StrLen(pStr) + 1;
      }
      tmp_width += _FntGetCharWidth(' ');
    }
    g->ui.menu.item[g->ui.menu.count].key_code = key_code;

    _FntSetFont(g->ui.menu.font_id);
    g->ui.menu.rect.x = 0;
    if (g->ui.menu.rect.width < (tmp_width + _FntGetCharsWidth(g->ui.menu.item[g->ui.menu.count].label, _StrLen(g->ui.menu.item[g->ui.menu.count].label)) + MENU_SPACE_X))
      g->ui.menu.rect.width = (tmp_width + _FntGetCharsWidth(g->ui.menu.item[g->ui.menu.count].label, _StrLen(g->ui.menu.item[g->ui.menu.count].label)) + MENU_SPACE_X);
    if (g->ui.menu.item[g->ui.menu.count].label[0] == '-')
      g->ui.menu.rect.height += _FntGetFontHeight() / 2;
    else
      g->ui.menu.rect.height += (_FntGetFontHeight() + 1);

    // PEN BASED SYSTEMS: menu comes from the top of the screen
    if (_PenAvailable())
      g->ui.menu.rect.y = 24; // 0;
    else
      g->ui.menu.rect.y = g->device.display.height - (g->ui.menu.rect.height + _UIMenuGetHeight());

    // we have one more menu item
    g->ui.menu.count++;
  }
}

/********************************* EOF ***********************************/
