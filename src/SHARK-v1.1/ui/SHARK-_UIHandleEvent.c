/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_UIHandleEvent.c
 */

#include "../SHARK-prv.h"

boolean
_UIHandleEvent(event *e)
{
  boolean     processed;
  boolean     menu_vis, menu_redraw;
  rectangle   rect;
  coord       penX, penY;
  boolean     penDown;
  coord       x, y, _x, _y;
  color       c;
  int         i, j;
  uint8      *pStr;
  gfx_window *win;
  event       _e;
  GLOBALS_ACCESS;

  // no font defined? lets get outa here
  if (g->ui.menu.font_id == font_undefined) return false;

  // assume the following
  processed   = false;
  menu_vis    = g->ui.menu.visible;
  menu_redraw = false;

  // swallow all events if the menu is visible
  processed = g->ui.menu.visible;

  switch (e->eType)
  {
    case _appStartEvent:
    case _appStopEvent:
         processed = false;  // override these events - application MUST get them
         break;

    case _keyEvent:
         switch (e->data.key.chr)
         {
           case _key_menu:

                // if we have a menu, pop it up (or, close it)
                if (g->ui.menu.count > 0)
                {
                  // is the menu open right now?
                  if (g->ui.menu.visible)
                  {
                    // maybe some nested menu items changed - we need to inform application of these
                    for (i=0; i<g->ui.menu.count; i++)
                    {
                      if (g->ui.menu.item[i].nested.prev != g->ui.menu.item[i].nested.index)
                      {
                        _MemSet(&_e, sizeof(event), 0);
                        _e.eType        = _keyEvent;
                        _e.data.key.chr = g->ui.menu.item[i].key_code;
                        if (_e.data.key.chr != _key_unknown) 
                        {
                          if (g->ui.menu.item[i].nested.count != 0)
                            _e.data.key.chr += g->ui.menu.item[i].nested.index;
                          _SysEnqueueEvent(&_e);
                        }

                        g->ui.menu.item[i].nested.prev = g->ui.menu.item[i].nested.index;
                      }
                    }
                  }

                  g->ui.menu.visible = !g->ui.menu.visible;
                  g->ui.menu.index   = 0;
                  menu_redraw        = g->ui.menu.visible;
                }

                if (g->ui.menu.button[0].key_code != _key_unknown)
                {
                  _MemSet(&_e, sizeof(event), 0);
                  _e.eType        = _keyEvent;
                  _e.data.key.chr = g->ui.menu.button[0].key_code;
                  _SysEnqueueEvent(&_e);
                }

                processed = true;  // override!
                break;

           case _key_acknowledge:

                // if we have a menu, close it
                if (g->ui.menu.visible)
                {
                  if (g->ui.menu.count > 0)
                  {
                    // maybe some nested menu items changed - we need to inform application of these
                    for (i=0; i<g->ui.menu.count; i++)
                    {
                      if (g->ui.menu.item[i].nested.prev != g->ui.menu.item[i].nested.index)
                      {
                        _MemSet(&_e, sizeof(event), 0);
                        _e.eType        = _keyEvent;
                        _e.data.key.chr = g->ui.menu.item[i].key_code;
                        if (_e.data.key.chr != _key_unknown) 
                        {
                          if (g->ui.menu.item[i].nested.count != 0)
                            _e.data.key.chr += g->ui.menu.item[i].nested.index;
                          _SysEnqueueEvent(&_e);
                        }

                        g->ui.menu.item[i].nested.prev = g->ui.menu.item[i].nested.index;
                      }
                    }
                  }

                  g->ui.menu.visible = false;
                }

                // send the event (after nested events)
                if (g->ui.menu.button[1].key_code != _key_unknown)
                {
                  _MemSet(&_e, sizeof(event), 0);
                  _e.eType        = _keyEvent;
                  _e.data.key.chr = g->ui.menu.button[1].key_code;
                  _SysEnqueueEvent(&_e);

                  processed = true;  // override!
                }
                break;

           case _key_padup:
                if (g->ui.menu.visible)
                {
                  // can we move down?
                  if (g->ui.menu.index > 0)
                  {
                    g->ui.menu.index--;
                    while ((g->ui.menu.index > 0) && (g->ui.menu.item[g->ui.menu.index].label[0] == '-'))
                      g->ui.menu.index--;
 
                    menu_redraw = true;
                  }
                  else
                  {
                    g->ui.menu.index = g->ui.menu.count-1;
                    menu_redraw = true;
                  }

                  processed = true;  // override!
                }
                break;

           case _key_paddown:
                if (g->ui.menu.visible)
                {
                  // can we move down?
                  if (g->ui.menu.index < (g->ui.menu.count-1))
                  {
                    g->ui.menu.index++;
                    while ((g->ui.menu.index < (g->ui.menu.count-1)) && (g->ui.menu.item[g->ui.menu.index].label[0] == '-'))
                      g->ui.menu.index++;

                    menu_redraw = true;
                  }
                  else
                  {
                    g->ui.menu.index = 0;
                    menu_redraw = true;
                  }

                  processed = true;  // override!
                }
                break;

           case _key_padleft:
                if (g->ui.menu.special && (g->ui.menu.item[g->ui.menu.index].nested.count != 0))
                {
                  if (g->ui.menu.item[g->ui.menu.index].nested.index > 0)
                  {
                    // save the selected indexes for future use
                    g->ui.menu.item[g->ui.menu.index].nested.prev = g->ui.menu.item[g->ui.menu.index].nested.index;

                    // make the change
                    g->ui.menu.item[g->ui.menu.index].nested.index--;
                    menu_redraw = true;
                  }

                  processed = true;  // override!
                }
                break;

           case _key_padright:
                if (g->ui.menu.special && (g->ui.menu.item[g->ui.menu.index].nested.count != 0))
                {
                  if (g->ui.menu.item[g->ui.menu.index].nested.index < (g->ui.menu.item[g->ui.menu.index].nested.count-1))
                  {
                    // save the selected indexes for future use
                    g->ui.menu.item[g->ui.menu.index].nested.prev = g->ui.menu.item[g->ui.menu.index].nested.index;

                    // make the change
                    g->ui.menu.item[g->ui.menu.index].nested.index++;
                    menu_redraw = true;
                  }

                  processed = true;  // override!
                }
                break;

           case _key_action:
           case _key_padselect:
                if (g->ui.menu.visible)
                {
                  // need to enqueue an event to the system? (selected item)
                  if (g->ui.menu.index != -1)
                  {
                    // maybe some nested menu items changed - we need to inform application of these
                    for (i=0; i<g->ui.menu.count; i++)
                    {
                      if (g->ui.menu.item[i].nested.prev != g->ui.menu.item[i].nested.index)
                      {
                        _MemSet(&_e, sizeof(event), 0);
                        _e.eType        = _keyEvent;
                        _e.data.key.chr = g->ui.menu.item[i].key_code;
                        if (_e.data.key.chr != _key_unknown) 
                        {
                          if (g->ui.menu.item[i].nested.count != 0)
                            _e.data.key.chr += g->ui.menu.item[i].nested.index;
                          _SysEnqueueEvent(&_e);
                        }

                        g->ui.menu.item[i].nested.prev = g->ui.menu.item[i].nested.index;
                      }
                    }

                    _MemSet(&_e, sizeof(event), 0);
                    _e.eType        = _keyEvent;
                    _e.data.key.chr = g->ui.menu.item[g->ui.menu.index].key_code;
                    if (_e.data.key.chr != _key_unknown)
                    {
                      if (g->ui.menu.item[g->ui.menu.index].nested.count != 0)
                        _e.data.key.chr += g->ui.menu.item[g->ui.menu.index].nested.index;
                      _SysEnqueueEvent(&_e);
                    }
                  }
                  g->ui.menu.visible = false;

                  processed = true;  // override!
                }
                break;

           default:
                break;
         }
         break;

    case _penEvent:

         penX = e->data.pen.x;
         penY = e->data.pen.y;
         if (_PenDisplayToWindow(&penX, &penY))
         {
           // if menu is not visible - bring up menu?
           if (!g->ui.menu.visible)
           {
             // menu button
             if ((penX >  g->ui.menu.button[0].rect.x) &&
                 (penX < (g->ui.menu.button[0].rect.x + g->ui.menu.button[0].rect.width)) &&
                 (penY >  g->ui.menu.button[0].rect.y) &&
                 (penY < (g->ui.menu.button[0].rect.y + g->ui.menu.button[0].rect.height)))
             {
               _MemSet(&_e, sizeof(event), 0);
               _e.eType        = _keyEvent;
               _e.data.key.chr = _key_menu;
               _SysEnqueueEvent(&_e);

               processed = true; // override the event
             }
           }

           // menu is visible
           else
           {
             // the menu itself?
             if ((penX > g->ui.menu.rect.x) &&
                 (penX < (g->ui.menu.rect.x + g->ui.menu.rect.width)) &&
                 (penY > g->ui.menu.rect.y) &&
                 (penY < (g->ui.menu.rect.y + g->ui.menu.rect.height)))
             {
               x = 0;
               y = g->ui.menu.rect.y + 1;
               for (i=0; i<g->ui.menu.count; i++)
               {
                 rect.x      = 1;
                 rect.y      = y;
                 rect.width  = g->ui.menu.rect.width-2;
                 rect.height = (_FntGetFontHeight() + 1);
  
                 // are we within a specific item?       
                 if ((penY > rect.y) && (penY < (rect.y + rect.height)) && (g->ui.menu.item[i].label[0] != '-'))  
                 {
                   // clicking on the arrows (nested cases only)
                   if (g->ui.menu.special && (g->ui.menu.item[i].nested.count != 0))
                   {
                     if (penX < (rect.x + MENU_ARROWS_X)) 
                     {
                       if (g->ui.menu.item[i].nested.index > 0)
                       {
                         // save the selected indexes for future use
                         g->ui.menu.item[i].nested.prev = g->ui.menu.item[i].nested.index;
  
                         // make the change
                         g->ui.menu.item[i].nested.index--;
                       }
                     }
                     else
                     if (penX > (rect.x + rect.width - MENU_ARROWS_X)) 
                     {
                       if (g->ui.menu.item[i].nested.index < (g->ui.menu.item[i].nested.count-1))
                       {
                         // save the selected indexes for future use
                         g->ui.menu.item[i].nested.prev = g->ui.menu.item[i].nested.index;
  
                         // make the change
                         g->ui.menu.item[i].nested.index++;
                       }
                     }
  
                     // is the item already highlighted? (select it)
                     else
#ifdef SHARK_UI_DOUBLE_TAP
                     if (i == g->ui.menu.index)
#endif
                     {
#ifndef SHARK_UI_DOUBLE_TAP
                       g->ui.menu.index = i;
#endif  
                       // maybe some nested menu items changed - we need to inform application of these
                       for (i=0; i<g->ui.menu.count; i++)
                       {
                         if (g->ui.menu.item[i].nested.prev != g->ui.menu.item[i].nested.index)
                         {
                           _MemSet(&_e, sizeof(event), 0);
                           _e.eType        = _keyEvent;
                           _e.data.key.chr = g->ui.menu.item[i].key_code;
                           if (_e.data.key.chr != _key_unknown) 
                           {
                             if (g->ui.menu.item[i].nested.count != 0)
                               _e.data.key.chr += g->ui.menu.item[i].nested.index;
                             _SysEnqueueEvent(&_e);
                           }

                           g->ui.menu.item[i].nested.prev = g->ui.menu.item[i].nested.index;
                         }
                       }
  
                       _MemSet(&_e, sizeof(event), 0);
                       _e.eType        = _keyEvent;
                       _e.data.key.chr = g->ui.menu.item[g->ui.menu.index].key_code;
                       if (_e.data.key.chr != _key_unknown) 
                       {
                         if (g->ui.menu.item[g->ui.menu.index].nested.count != 0)
                           _e.data.key.chr += g->ui.menu.item[g->ui.menu.index].nested.index;
                         _SysEnqueueEvent(&_e);
                       }
  
                       // wait for the user to list the pen (if using pen system)
#ifndef SHARK_UI_DOUBLE_TAP
                       win = _GfxGetDrawWindow();
                       _GfxSetDrawWindow(g->gfx.window);

                       _GfxInvertRegion(&rect);
                       _GfxRepaint();

                       // wait for the key to be lifted
                       _PenState(&penX, &penY, &penDown);
                       while (penDown)
                         _PenState(&penX, &penY, &penDown);
 
                       _GfxInvertRegion(&rect);
                       _GfxRepaint();

                       _GfxSetDrawWindow(win);
#endif
    
                       g->ui.menu.visible = false;
                     }
                   }
  
                   // is the item already highlighted? (select it)
                   else
#ifdef SHARK_UI_DOUBLE_TAP
                   if (i == g->ui.menu.index)
#endif
                   {
#ifndef SHARK_UI_DOUBLE_TAP
                     g->ui.menu.index = i;
#endif  
       
                     // maybe some nested menu items changed - we need to inform application of these
                     for (i=0; i<g->ui.menu.count; i++)
                     {
                       if (g->ui.menu.item[i].nested.prev != g->ui.menu.item[i].nested.index)
                       {
                         _MemSet(&_e, sizeof(event), 0);
                         _e.eType        = _keyEvent;
                         _e.data.key.chr = g->ui.menu.item[i].key_code;
                         if (_e.data.key.chr != _key_unknown) 
                         {
                           if (g->ui.menu.item[i].nested.count != 0)
                             _e.data.key.chr += g->ui.menu.item[i].nested.index;
                           _SysEnqueueEvent(&_e);
                         }

                         g->ui.menu.item[i].nested.prev = g->ui.menu.item[i].nested.index;
                       }
                     }
  
                     _MemSet(&_e, sizeof(event), 0);
                     _e.eType        = _keyEvent;
                     _e.data.key.chr = g->ui.menu.item[g->ui.menu.index].key_code;
                     if (_e.data.key.chr != _key_unknown) 
                     {
                       if (g->ui.menu.item[g->ui.menu.index].nested.count != 0)
                         _e.data.key.chr += g->ui.menu.item[g->ui.menu.index].nested.index;
                       _SysEnqueueEvent(&_e);
                     }
  
                     // wait for the user to list the pen (if using pen system)
#ifndef SHARK_UI_DOUBLE_TAP
                     win = _GfxGetDrawWindow();
                     _GfxSetDrawWindow(g->gfx.window);

                     _GfxInvertRegion(&rect);
                     _GfxRepaint();

                     // wait for the key to be lifted
                     _PenState(&penX, &penY, &penDown);
                     while (penDown)
                       _PenState(&penX, &penY, &penDown);

                     _GfxInvertRegion(&rect);
                     _GfxRepaint();

                     _GfxSetDrawWindow(win);
#endif
  
                     g->ui.menu.visible = false;
                   }
  
                   g->ui.menu.index = i;
                   goto PEN_ABORT;
                 }
  
                 if (g->ui.menu.item[i].label[0] == '-') y += _FntGetFontHeight() / 2;
                 else                                    y += (_FntGetFontHeight() + 1);
               }
  
PEN_ABORT:
  
               menu_redraw = true;
             }

             // tapped outside, close the menu
             else
             {
               // maybe some nested menu items changed - we need to inform application of these
               for (i=0; i<g->ui.menu.count; i++)
               {
                 if (g->ui.menu.item[i].nested.prev != g->ui.menu.item[i].nested.index)
                 {
                   _MemSet(&_e, sizeof(event), 0);
                   _e.eType        = _keyEvent;
                   _e.data.key.chr = g->ui.menu.item[i].key_code;
                   if (_e.data.key.chr != _key_unknown) 
                   {
                     if (g->ui.menu.item[i].nested.count != 0)
                       _e.data.key.chr += g->ui.menu.item[i].nested.index;
                     _SysEnqueueEvent(&_e);
                   }

                   g->ui.menu.item[i].nested.prev = g->ui.menu.item[i].nested.index;
                 }
               }

               g->ui.menu.visible = false;
             }

             processed = true;  // override!
           }
         }
         break;

    default:
         break;
  }

  // was the menu opened or closed in this instance?
  win = _GfxGetDrawWindow();
  _GfxSetDrawWindow(g->gfx.window);
  if (menu_vis != g->ui.menu.visible)
  {
    // menu visibility state? 
    if (g->ui.menu.visible)
    {
      // possible memory mishap, lets clean up before we clobber this pointer
      if (g->ui.menu.win != NULL) _GfxDeleteWindow(g->ui.menu.win);
      g->ui.menu.win = NULL;

      g->ui.menu.win = _GfxCreateWindow(g->ui.menu.rect.width, g->ui.menu.rect.height);
      if (g->ui.menu.win != NULL)
        _GfxCopyRegion(_GfxGetDrawWindow(), g->ui.menu.win, &g->ui.menu.rect, 0, 0, gfxPaint);

      menu_redraw = true;
    }
    else
    {
      // close menu and restore
      if (g->ui.menu.win != NULL)
      {
        rect.x      = 0; 
        rect.y      = 0;
        rect.width  = g->ui.menu.rect.width;
        rect.height = g->ui.menu.rect.height;
        _GfxCopyRegion(g->ui.menu.win, _GfxGetDrawWindow(), &rect, g->ui.menu.rect.x, g->ui.menu.rect.y, gfxPaint);
        _GfxRepaint();

        _GfxDeleteWindow(g->ui.menu.win);
        g->ui.menu.win = NULL;
      }
    }
  }

  // do we need to redraw the menu?
  if ((g->ui.menu.visible) && (menu_redraw))
  {
    // clear the region behind the menu
    _GfxFillRegion(&g->ui.menu.rect, _GfxGetPaletteIndex(255, 255, 255));
    _GfxDrawRectangle(&g->ui.menu.rect, _GfxGetPaletteIndex(0, 0, 0));
    _FntSetFont(g->ui.menu.font_id);

    y = g->ui.menu.rect.y + 1;
    for (i=0; i<g->ui.menu.count; i++)
    {
      x = 0;
      if (g->ui.menu.special) x += MENU_ARROWS_X;

      if (g->ui.menu.item[i].label[0] == '-')
        _GfxDrawLine(0, (coord)(y + (_FntGetFontHeight() / 4)), 
                     (coord)(g->ui.menu.rect.width - 1), (coord)(y + (_FntGetFontHeight() / 4)), _GfxGetPaletteIndex(0, 0, 0));
      else
        _GfxDrawString(g->ui.menu.item[i].label, _StrLen(g->ui.menu.item[i].label), (coord)(x + (MENU_SPACE_X / 2)), y, gfxPaint);

      // special case - multi value items
      if (g->ui.menu.item[i].nested.count != 0)
      {
        _x = (MENU_ARROWS_X - (g->ui.menu.spacing >> 1)) / 2;
        _y = y + ((_FntGetFontHeight() - g->ui.menu.spacing) / 2);
        _y += (g->ui.menu.spacing >> 1);

        if (g->ui.menu.item[i].nested.index > 0)
          c = _GfxGetPaletteIndex(0, 0, 0);
        else
          c = _GfxGetPaletteIndex(192, 192, 192);
        for (j=0; j<(g->ui.menu.spacing >> 1); j++, _x++)
          _GfxDrawLine(_x, (coord)(_y - j), _x, (coord)(_y + j), c);

        _x = g->ui.menu.rect.width - ((MENU_ARROWS_X - (g->ui.menu.spacing >> 1)) / 2);
        if (g->ui.menu.item[i].nested.index < (g->ui.menu.item[i].nested.count-1))
          c = _GfxGetPaletteIndex(0, 0, 0);
        else
          c = _GfxGetPaletteIndex(192, 192, 192);
        for (j=0; j<(g->ui.menu.spacing >> 1); j++, _x--)
          _GfxDrawLine(_x, (coord)(_y - j), _x, (coord)(_y + j), c); _x--;

        x += _FntGetCharsWidth(g->ui.menu.item[i].label, _StrLen(g->ui.menu.item[i].label));
        x += (MENU_SPACE_X / 2);
        x += _FntGetCharWidth(' ');        

        pStr = g->ui.menu.item[i].nested.labels;
        for (j=0; j<g->ui.menu.item[i].nested.index; j++) 
          pStr += _StrLen(pStr) + 1;

        _GfxDrawString(pStr, _StrLen(pStr), x, y, gfxPaint);
      }

      // draw the highlighted menu item
      if (i == g->ui.menu.index)
      {
        rect.x      = 1;
        rect.y      = y;
        rect.width  = g->ui.menu.rect.width-2;
        rect.height = (_FntGetFontHeight() + 1);
        _GfxInvertRegion(&rect);
      }

      if (g->ui.menu.item[i].label[0] == '-') y += _FntGetFontHeight() / 2;
      else                                    y += (_FntGetFontHeight() + 1);
    }
    _GfxRepaint();

#ifndef SHARK_UI_DOUBLE_TAP
    if (g->ui.menu.index != -1)
      _GfxInvertRegion(&rect);
#endif
  }
  _GfxSetDrawWindow(win);

  // wait for the user to lift the pen (if using pen system)
#ifndef SHARK_UI_DOUBLE_TAP
  if (!g->ui.menu.visible)  // special case
  {
    if ((e->eType == _penEvent) && (processed))  // must be responding to it
    {
      _PenState(&penX, &penY, &penDown);
      while (penDown)
        _PenState(&penX, &penY, &penDown);
    }
  }
#endif

  return processed;
}

/********************************* EOF ***********************************/
