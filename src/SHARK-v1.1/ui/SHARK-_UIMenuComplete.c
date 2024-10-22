/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_UIMenuComplete.c
 */

#include "../SHARK-prv.h"

void
_UIMenuComplete()
{
  rectangle   rect;
  uint16      spacing;
  coord       x, y;
  int         i;
  gfx_window *win;
  GLOBALS_ACCESS;

  // no font defined? lets get outa here
  if (g->ui.menu.font_id == font_undefined) return;

  // make adjustments (just in case of error)
  if (g->ui.menu.item[g->ui.menu.count-1].label[0] == '-') 
  {
    g->ui.menu.count--;
    _FntSetFont(g->ui.menu.font_id);
    g->ui.menu.rect.height -= _FntGetFontHeight() / 2;
  }

  if (g->ui.menu.special)
    g->ui.menu.rect.width += (MENU_ARROWS_X * 2);

  // draw the menu
  if (!_PenAvailable())
  {
    rect.x      = 0;
    rect.y      = g->device.display.height - _UIMenuGetHeight();
    rect.width  = g->device.display.width;
    rect.height = _UIMenuGetHeight();
    spacing     = rect.width / 2;
  
    win = _GfxGetDrawWindow();
    _GfxSetDrawWindow(g->gfx.window);
  
    _GfxFillRegion(&rect, _GfxGetPaletteIndex(255, 255, 255));
    _GfxDrawLine(rect.x, rect.y, (coord)(rect.x + rect.width), rect.y, _GfxGetPaletteIndex(0, 0, 0));
    _GfxDrawLine((coord)(rect.x + spacing), rect.y, 
                 (coord)(rect.x + spacing), (coord)(rect.y + rect.height), _GfxGetPaletteIndex(0, 0, 0));
  
    _FntSetFont(g->ui.menu.font_id);
    y = rect.y + (MENU_SPACE_Y-1);
    x = rect.x + ((spacing - _FntGetCharsWidth(g->ui.menu.button[0].label, _StrLen(g->ui.menu.button[0].label))) / 2);
    _GfxDrawString(g->ui.menu.button[0].label, _StrLen(g->ui.menu.button[0].label), x, y, gfxPaint);
    x = rect.x + spacing + ((spacing - _FntGetCharsWidth(g->ui.menu.button[1].label, _StrLen(g->ui.menu.button[1].label))) / 2);
    _GfxDrawString(g->ui.menu.button[1].label, _StrLen(g->ui.menu.button[1].label), x, y, gfxPaint);    
  
    // repaint the region where the menu is
    _GfxRepaintRegion(&rect);
    _GfxSetDrawWindow(win);
  
    // save the selected indexes for future use
    for (i=0;i<g->ui.menu.count; i++)
      g->ui.menu.item[i].nested.prev = g->ui.menu.item[i].nested.index;
  }
}

/********************************* EOF ***********************************/
