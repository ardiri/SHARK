/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-mnu.c
 */

#include "../../SHARK.h"

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

#define MNU_LIBRARY_ID  0x5F6D6E75 // '_mnu'

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

#define MAX_MENUS     13
#define MAX_BUTTONS   2
#define MENU_SPACE_X  10
#define MENU_SPACE_Y  3
#define MENU_ARROWS_X (2 + g_mnu->spacing)

typedef struct _MnuGlobals
{
  void       *extension;

  boolean   (*fnMnuInitialize)(void);
  void      (*fnMnuTerminate)(void);

  boolean     visible;              // is the menu visible?
  boolean     special;              // special menu type?
  rectangle   rect;
  gfx_window *win;

  font        font_id;              // the font to draw the menu text
  coord       spacing;

  int16       index;
  int16       count;

  boolean     pen_support;

  union
  {
    struct
    {
      boolean   enabled;
      rectangle rect;
    } trigger;

    struct
    {
      rectangle rect;
      char      label[32];
      uint8     key_code;           // the rect/text/codes for screen buttons
    } button[MAX_BUTTONS];
  } data;

  struct
  {
    char      label[32];
    uint8     key_code;             // the text/codes for menu items
    struct
    {
      uint8   count;
      uint8   index, prev;
      char    labels[64];
    } nested;

  } item[MAX_MENUS];

} _MnuGlobals;

#define MNU_GLOBALS_ACCESS \
  _MnuGlobals *g_mnu = (_MnuGlobals *)_LibGetGlobals(MNU_LIBRARY_ID);

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibGetGlobals() API.
//------------------------------------------------------------------------

/*************************************************************************
 *
 * Implemnutation
 *
 *************************************************************************/

static void    _MnuInitialize_RESIZE();
static void    _MnuTerminate_RESIZE();

static boolean _MnuDriverConfiguration(void);
static boolean _MnuHandleEvent(event *e);

boolean
_MnuInitialize()
{
  boolean      init_ok;
  _MnuGlobals *g_mnu;

  // assume everything is ok
  init_ok = true;

  // lets see if we have already been initialized
  g_mnu = (_MnuGlobals *)_LibGetGlobals(MNU_LIBRARY_ID);

  // entry condition
  if (g_mnu != NULL) return false;

  // register the library
  if (_LibRegister(MNU_LIBRARY_ID, "_MnuLibrary", (void *)_MnuHandleEvent))
  {
    // allocate globals memory for the library
    g_mnu = (_MnuGlobals *)_MemPtrNew(sizeof(_MnuGlobals), false);
    init_ok &= (g_mnu != NULL); if (!init_ok) goto MNU_INIT_ABORT;
    _LibSetGlobals(MNU_LIBRARY_ID, (void *)g_mnu);

    // initialize the _MNU driver
    init_ok = _MnuDriverConfiguration();
    if (!init_ok) goto MNU_INIT_ABORT;

    // initialize _Fnt internally
    _MnuInitialize_RESIZE();
  }
  else
    init_ok = false;

  // define the default font id (we hope it'll exist)
  g_mnu->font_id = INVALID_FONT;
  g_mnu->pen_support = ((_InputSupported() & INPUT_STYLUS) != 0);

  // initialize
  _MnuSetFont(g_mnu->font_id);
  if (g_mnu->pen_support) g_mnu->data.trigger.enabled = true;

MNU_INIT_ABORT:

  return init_ok;
}

void
_MnuReset()
{
  int16 i;
  MNU_GLOBALS_ACCESS;

  // entry requirement
  if ((g_mnu == NULL) || (g_mnu->font_id == INVALID_FONT)) return;
 
  // reset all the menu variables
  g_mnu->visible            = false;
  g_mnu->special            = false;
  g_mnu->index              = _key_unknown;
  g_mnu->count              = 0;
  g_mnu->rect.x             = 0;
  g_mnu->rect.y             = 0;
  g_mnu->rect.width         = 0;
  g_mnu->rect.height        = 2;

  if (!g_mnu->pen_support)
  {
    for (i=0; i<MAX_BUTTONS; i++)
    {
      _StrCopy(g_mnu->data.button[i].label, "");
      g_mnu->data.button[i].key_code = _key_unknown;
    }
  }
}

uint32
_MnuGetHeight()
{
  uint32 result; 
  MNU_GLOBALS_ACCESS;

  // default return value
  result = 0;

  // entry requirement
  if ((g_mnu == NULL) || (g_mnu->font_id == INVALID_FONT)) return result;

  _FntSetFont(g_mnu->font_id);
  result = (_FntGetFontHeight() + MENU_SPACE_Y + 1) & ~0x01; // ensure even 

  return result;
}

void
_MnuSetFont(font font_id)
{
  MNU_GLOBALS_ACCESS;

  // entry requirement
  if ((g_mnu == NULL) || (font_id == INVALID_FONT)) return;

  // adjust menu spacing based on font selected
  g_mnu->font_id = font_id;

  _FntSetFont(g_mnu->font_id);
  g_mnu->spacing = (int16)(((_FntGetFontHeight() * 3) / 5) + 1) & ~0x01;
  _MnuReset();
}

void
_MnuSetButton(int8 pos, char *label, uint8 key_code)
{
  MNU_GLOBALS_ACCESS;

  // entry requirement
  if ((g_mnu == NULL) || (g_mnu->font_id == INVALID_FONT)) return;

  // only valid under these conditions
  if (!g_mnu->pen_support)
  {
    if ((pos >= 0) && (pos < MAX_BUTTONS))
    {
      _MemSet(g_mnu->data.button[pos].label, 32, 0);
      _StrNCopy(g_mnu->data.button[pos].label, label, 31);  // max 31 chars
      g_mnu->data.button[pos].key_code = key_code;
    }
  }
}

void
_MnuSetMenuTriggerRegion(rectangle *rect)
{
  MNU_GLOBALS_ACCESS;

  // entry requirement
  if ((g_mnu == NULL) || (g_mnu->font_id == INVALID_FONT)) return;

  // only valid under these conditions
  if (g_mnu->pen_support)
  {
    if (rect == NULL)
    {
      g_mnu->data.trigger.enabled = false;

      g_mnu->data.trigger.rect.x      = -1;
      g_mnu->data.trigger.rect.y      = -1;
      g_mnu->data.trigger.rect.width  = 0;
      g_mnu->data.trigger.rect.height = 0;   // there is no trigger region 
    }
    else
    {
      g_mnu->data.trigger.enabled = true;

      g_mnu->data.trigger.rect.x      = rect->x;
      g_mnu->data.trigger.rect.y      = rect->y;
      g_mnu->data.trigger.rect.width  = rect->width;
      g_mnu->data.trigger.rect.height = rect->height;
    }
  }
}

void
_MnuAddItem(char *label, uint8 key_code, char *param)
{
  char  *pStr;
  int32  i, cnt, tmp_width;
  int16  height;
  MNU_GLOBALS_ACCESS;

  // entry requirement
  if ((g_mnu == NULL) || (g_mnu->font_id == INVALID_FONT)) return;

  // sorry :)
  if ((label[0] == '-') && (g_mnu->count == 0)) return;

  // get the display size
  _GfxGetWindowInfo(_GfxGetDisplayWindow(), NULL, NULL, &height, NULL);

  if ((!g_mnu->visible) && (g_mnu->count < MAX_MENUS))
  {
    tmp_width = 0;

    _MemSet(g_mnu->item[g_mnu->count].label, 32, 0);
    _MemSet(g_mnu->item[g_mnu->count].nested.labels, 64, 0);
    _StrNCopy(g_mnu->item[g_mnu->count].label, label, 31); // max 32 chars
    if (param == NULL)
      g_mnu->item[g_mnu->count].nested.count = 0;
    else
    if ((param[1] == ':') && (param[3] == ':')) // validation
    {
      g_mnu->special = true;
      g_mnu->item[g_mnu->count].nested.count = (uint8)_StrAToI(param);
      param += 2; // skip the ":"
      g_mnu->item[g_mnu->count].nested.index = (uint8)_StrAToI(param);
      param += 2; // skip the ":"

      cnt       = 0;
      pStr      = param;
      for (i=0; i<g_mnu->item[g_mnu->count].nested.count; i++)
      {
        _StrNCopy(&g_mnu->item[g_mnu->count].nested.labels[cnt], 
                  pStr, (int16)(63 - cnt));  // max 64 chars
        if (tmp_width < (int32)(_FntGetCharsWidth(pStr, _StrLen(pStr))))
          tmp_width = _FntGetCharsWidth(pStr, _StrLen(pStr));
        cnt  += _StrLen(pStr) + 1;
        pStr += _StrLen(pStr) + 1;
      }
      tmp_width += _FntGetCharsWidth(" ", 1);
    }
    g_mnu->item[g_mnu->count].key_code = key_code;

    _FntSetFont(g_mnu->font_id);
    g_mnu->rect.x = 0;
    if (g_mnu->rect.width < 
          (int16)(tmp_width + MENU_SPACE_X + 
                  _FntGetCharsWidth(g_mnu->item[g_mnu->count].label, 
                                    _StrLen(g_mnu->item[g_mnu->count].label))))
      g_mnu->rect.width = 
        (int16)(tmp_width + MENU_SPACE_X + 
                _FntGetCharsWidth(g_mnu->item[g_mnu->count].label, 
                                  _StrLen(g_mnu->item[g_mnu->count].label)));
    if (g_mnu->item[g_mnu->count].label[0] == '-')
      g_mnu->rect.height += (int16)_FntGetFontHeight() / 2;
    else
      g_mnu->rect.height += (int16)(_FntGetFontHeight() + 1);

    // PEN BASED SYSTEMS: menu comes from the top of the screen
    if (g_mnu->pen_support)
      g_mnu->rect.y = 0;
    else
      g_mnu->rect.y = height - (int16)(g_mnu->rect.height + _MnuGetHeight());

    // we have one more menu item
    g_mnu->count++;
  }
}

void
_MnuAddSeparator()
{
  _MnuAddItem("-", _key_unknown, NULL);
}

void
_MnuComplete()
{
  rectangle   rect;
  uint16      spacing;
  int16       width, height;
  coord       x, y;
  int         i;
  gfx_window *win;
  MNU_GLOBALS_ACCESS;

  // entry requirement
  if ((g_mnu == NULL) || (g_mnu->font_id == INVALID_FONT)) return;

  // get the display size
  _GfxGetWindowInfo(_GfxGetDisplayWindow(), NULL, &width, &height, NULL);

  // make adjustments (just in case of error)
  if (g_mnu->item[g_mnu->count-1].label[0] == '-')
  {
    g_mnu->count--;
    _FntSetFont(g_mnu->font_id);
    g_mnu->rect.height -= (int16)_FntGetFontHeight() / 2;
  }

  if (g_mnu->special)
    g_mnu->rect.width += (MENU_ARROWS_X * 2);

  // draw the menu
  if (!g_mnu->pen_support)
  {
    rect.x      = 0;
    rect.y      = height - (int16)_MnuGetHeight();
    rect.width  = width;
    rect.height = (int16)_MnuGetHeight();
    spacing     = rect.width / 2;

    win = _GfxGetDrawWindow();
    _GfxSetDrawWindow(_GfxGetDisplayWindow());

    _GfxFillRegion(&rect, _GfxGetPaletteIndex(255, 255, 255));
    _GfxDrawLine(rect.x, rect.y, 
                 (coord)(rect.x + rect.width), rect.y, 
                 _GfxGetPaletteIndex(0, 0, 0));
    _GfxDrawLine((coord)(rect.x + spacing), rect.y,
                 (coord)(rect.x + spacing), (coord)(rect.y + rect.height), 
                 _GfxGetPaletteIndex(0, 0, 0));

    _FntSetFont(g_mnu->font_id);
    y = rect.y + (MENU_SPACE_Y-1);
    x = rect.x + ((spacing - 
          (int16)(_FntGetCharsWidth(g_mnu->data.button[0].label, 
                            _StrLen(g_mnu->data.button[0].label)))) / 2);
    _FntDrawString(g_mnu->data.button[0].label, 
                   _StrLen(g_mnu->data.button[0].label), x, y, gfxPaint);
    x = rect.x + spacing + 
          (int16)((spacing - 
                  _FntGetCharsWidth(g_mnu->data.button[1].label, 
                                    _StrLen(g_mnu->data.button[1].label))) / 2);
    _FntDrawString(g_mnu->data.button[1].label, 
                   _StrLen(g_mnu->data.button[1].label), x, y, gfxPaint);

    // repaint the region where the menu is
    _GfxRepaintRegion(&rect);
    _GfxSetDrawWindow(win);

    // save the selected indexes for future use
    for (i=0;i<g_mnu->count; i++)
      g_mnu->item[i].nested.prev = g_mnu->item[i].nested.index;
  }
}

void
_MnuTerminate()
{
  MNU_GLOBALS_ACCESS;

  // entry requiremnut (must call _MnuInitialize())
  if (g_mnu == NULL) return;

  // shutdown the sub-MNU layer
  if (g_mnu->fnMnuTerminate != NULL)
    g_mnu->fnMnuTerminate();

  // terminate _MNU internally
  _MnuTerminate_RESIZE();

  // free the globals used
  _MemPtrFree(g_mnu); g_mnu = NULL;
  _LibSetGlobals(MNU_LIBRARY_ID, (void *)g_mnu);

  // shutdown the library
  _LibShutdown(MNU_LIBRARY_ID);
}

static void
_MnuInitialize_RESIZE()
{
  int16 width;
  MNU_GLOBALS_ACCESS;

  // get the display size
  _GfxGetWindowInfo(_GfxGetDisplayWindow(), NULL, &width, NULL, NULL);

  // this is the 'pen response' area
  if ((g_mnu->pen_support) && (g_mnu->data.trigger.enabled))
  {
    g_mnu->data.trigger.rect.x      = 0;
    g_mnu->data.trigger.rect.y      = 0;
    g_mnu->data.trigger.rect.width  = width >> 2;
    g_mnu->data.trigger.rect.height = (int16)_MnuGetHeight(); // top right 
  }
}

static void
_MnuTerminate_RESIZE()
{
}

static boolean
_MnuDriverConfiguration(void)
{
  boolean     result;
  MNU_GLOBALS_ACCESS;

  // default return value
  result = false;

  // initialize the sub-mnu layer
  if (g_mnu->fnMnuInitialize != NULL)
    result = g_mnu->fnMnuInitialize();

  // FORCE::
  result = true;

  return result;
}

static boolean
_MnuHandleEvent(event *e)
{
  boolean processed;
  boolean     menu_vis, menu_redraw;  
  rectangle   rect;  
  coord       x, y, _x, _y;  
  color       c;  
  int         i, j;
  char       *pStr;
  gfx_window *win;
  event       _e;
  MNU_GLOBALS_ACCESS;

  // assume the following  
  processed   = false;  
  menu_vis    = g_mnu->visible;
  menu_redraw = false;

  // swallow all menu events if it is visible
  processed = g_mnu->visible;

  switch (e->eType)
  {
    case _keyEvent:
         switch (e->data.key.chr)
         {
           case _key_gfx_change:
                _MnuTerminate_RESIZE();
                _MnuInitialize_RESIZE();
                break;

           case _key_menu:

                // if we have a menu, pop it up (or, close it)
                if (g_mnu->count > 0)
                {
                  // is the menu open right now?
                  if (g_mnu->visible)
                  {
                    // possible nested menu items changed - inform application 
                    for (i=0; i<g_mnu->count; i++)
                    {
                      if (g_mnu->item[i].nested.prev != 
                          g_mnu->item[i].nested.index)
                      {
                        _MemSet(&_e, sizeof(event), 0);
                        _e.eType        = _keyEvent;
                        _e.data.key.chr = g_mnu->item[i].key_code;
                        if (_e.data.key.chr != _key_unknown)
                        {
                          if (g_mnu->item[i].nested.count != 0)
                            _e.data.key.chr += g_mnu->item[i].nested.index;
                          _SysEnqueueEvent(&_e);
                        }
                        g_mnu->item[i].nested.prev = 
                          g_mnu->item[i].nested.index;
                      }
                    }
                  }

                  g_mnu->visible = !g_mnu->visible;
                  g_mnu->index   = 0;
                  menu_redraw    = g_mnu->visible;
                }

                if (g_mnu->data.button[0].key_code != _key_unknown)
                {
                  _MemSet(&_e, sizeof(event), 0);
                  _e.eType        = _keyEvent;
                  _e.data.key.chr = g_mnu->data.button[0].key_code;
                  _SysEnqueueEvent(&_e);
                }

                processed = true;  
                break;

           case _key_acknowledge:
                // if we have a menu, close it
                if (g_mnu->visible)
                {
                  if (g_mnu->count > 0)
                  {
                    // possible nested menu items changed - inform application 
                    for (i=0; i<g_mnu->count; i++)
                    {
                      if (g_mnu->item[i].nested.prev != 
                          g_mnu->item[i].nested.index)
                      {
                        _MemSet(&_e, sizeof(event), 0);
                        _e.eType        = _keyEvent;
                        _e.data.key.chr = g_mnu->item[i].key_code;
                        if (_e.data.key.chr != _key_unknown)
                        {
                          if (g_mnu->item[i].nested.count != 0)
                            _e.data.key.chr += g_mnu->item[i].nested.index;
                          _SysEnqueueEvent(&_e);
                        }

                        g_mnu->item[i].nested.prev = 
                          g_mnu->item[i].nested.index;
                      }
                    }
                  }

                  g_mnu->visible = false;
                }

                // send the event (after nested events)
                if (g_mnu->data.button[1].key_code != _key_unknown)
                {
                  _MemSet(&_e, sizeof(event), 0);
                  _e.eType        = _keyEvent;
                  _e.data.key.chr = g_mnu->data.button[1].key_code;
                  _SysEnqueueEvent(&_e);

                  processed = true;  
                }
                break;

           case _key_padup:
                if (g_mnu->visible)
                {
                  // can we move down?
                  if (g_mnu->index > 0)
                  {
                    g_mnu->index--;
                    while ((g_mnu->index > 0) && 
                           (g_mnu->item[g_mnu->index].label[0] == '-'))
                      g_mnu->index--;

                    menu_redraw = true;
                  }
                  else
                  {
                    g_mnu->index = g_mnu->count-1;
                    menu_redraw = true;
                  }

                  processed = true;  // override!
                }
                break;

           case _key_paddown:
                if (g_mnu->visible)
                {
                  // can we move down?
                  if (g_mnu->index < (g_mnu->count-1))
                  {
                    g_mnu->index++;
                    while ((g_mnu->index < (g_mnu->count-1)) && 
                           (g_mnu->item[g_mnu->index].label[0] == '-'))
                      g_mnu->index++;

                    menu_redraw = true;
                  }
                  else
                  {
                    g_mnu->index = 0;
                    menu_redraw = true;
                  }

                  processed = true;  
                }
                break;

           case _key_padleft:
                if ((g_mnu->special) && 
                    (g_mnu->item[g_mnu->index].nested.count != 0))
                {
                  if (g_mnu->item[g_mnu->index].nested.index > 0)
                  {
                    // save the selected indexes for future use
                    g_mnu->item[g_mnu->index].nested.prev = 
                      g_mnu->item[g_mnu->index].nested.index;

                    // make the change
                    g_mnu->item[g_mnu->index].nested.index--;
                    menu_redraw = true;
                  }

                  processed = true;  
                }
                break;

           case _key_padright:
                if ((g_mnu->special) && 
                    (g_mnu->item[g_mnu->index].nested.count != 0))
                {
                  if (g_mnu->item[g_mnu->index].nested.index < 
                      (g_mnu->item[g_mnu->index].nested.count-1))
                  {
                    // save the selected indexes for future use
                    g_mnu->item[g_mnu->index].nested.prev = 
                      g_mnu->item[g_mnu->index].nested.index;

                    // make the change
                    g_mnu->item[g_mnu->index].nested.index++;
                    menu_redraw = true;
                  }

                  processed = true;  
                }
                break;

           case _key_padselect:
                if (g_mnu->visible)
                {
                  // need to enqueue an event to the system? (selected item)
                  if (g_mnu->index != -1)
                  {
                    // possible nested menu items changed - inform application 
                    for (i=0; i<g_mnu->count; i++)
                    {
                      if (g_mnu->item[i].nested.prev != 
                          g_mnu->item[i].nested.index)
                      {
                        _MemSet(&_e, sizeof(event), 0);
                        _e.eType        = _keyEvent;
                        _e.data.key.chr = g_mnu->item[i].key_code;
                        if (_e.data.key.chr != _key_unknown)
                        {
                          if (g_mnu->item[i].nested.count != 0)
                            _e.data.key.chr += g_mnu->item[i].nested.index;
                          _SysEnqueueEvent(&_e);
                        }
                        g_mnu->item[i].nested.prev = 
                          g_mnu->item[i].nested.index;
                      }
                    }

                    _MemSet(&_e, sizeof(event), 0);
                    _e.eType        = _keyEvent;
                    _e.data.key.chr = g_mnu->item[g_mnu->index].key_code;
                    if (_e.data.key.chr != _key_unknown)
                    {
                      if (g_mnu->item[g_mnu->index].nested.count != 0)
                        _e.data.key.chr += 
                          g_mnu->item[g_mnu->index].nested.index;
                      _SysEnqueueEvent(&_e);
                    }
                  }
                  g_mnu->visible = false;

                  processed = true;  
                }
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }
  
  // was the menu opened or closed in this instance?
  win = _GfxGetDrawWindow();
  _GfxSetDrawWindow(_GfxGetDisplayWindow());
  if (menu_vis != g_mnu->visible)
  {
    // menu visibility state?
    if (g_mnu->visible)
    {
      // possible memory mishap, lets clean up before we clobber this pointer
      if (g_mnu->win != NULL) _GfxDeleteWindow(g_mnu->win);
      g_mnu->win = NULL;

      g_mnu->win = 
        _GfxCreateWindow(g_mnu->rect.width, g_mnu->rect.height);
      if (g_mnu->win != NULL)
      {
        _GfxCopyRegion(_GfxGetDrawWindow(), 
                       g_mnu->win, &g_mnu->rect, 0, 0, gfxPaint);
      }

      menu_redraw = true;
    }
    else
    {
      // close menu and restore
      if (g_mnu->win != NULL)
      {
        rect.x      = 0;
        rect.y      = 0;
        rect.width  = g_mnu->rect.width;
        rect.height = g_mnu->rect.height;
        _GfxCopyRegion(g_mnu->win, _GfxGetDrawWindow(), 
                       &rect, g_mnu->rect.x, g_mnu->rect.y, gfxPaint);
        _GfxRepaint();

        _GfxDeleteWindow(g_mnu->win);
        g_mnu->win = NULL;
      }
    }
  }

  // do we need to redraw the menu?
  if ((g_mnu->visible) && (menu_redraw))
  {
    // clear the region behind the menu
    _GfxFillRegion(&g_mnu->rect, _GfxGetPaletteIndex(255, 255, 255));
    _GfxDrawRectangle(&g_mnu->rect, _GfxGetPaletteIndex(0, 0, 0));
    _FntSetFont(g_mnu->font_id);

    y = g_mnu->rect.y + 1;
    for (i=0; i<g_mnu->count; i++)
    {
      x = 0;
      if (g_mnu->special) x += MENU_ARROWS_X;

      if (g_mnu->item[i].label[0] == '-')
      {
        _GfxDrawLine(0, (coord)(y + (_FntGetFontHeight() / 4)),
                     (coord)(g_mnu->rect.width - 1), 
                     (coord)(y + (_FntGetFontHeight() / 4)), 
                     _GfxGetPaletteIndex(0, 0, 0));
      }
      else
        _FntDrawString(g_mnu->item[i].label, _StrLen(g_mnu->item[i].label), 
                       (coord)(x + (MENU_SPACE_X / 2)), y, gfxPaint);

      // special case - multi value items
      if (g_mnu->item[i].nested.count != 0)
      {
        _x = (MENU_ARROWS_X - (g_mnu->spacing >> 1)) / 2;
        _y = y + (int16)((_FntGetFontHeight() - g_mnu->spacing) / 2);
        _y += (g_mnu->spacing >> 1);

        if (g_mnu->item[i].nested.index > 0)
          c = _GfxGetPaletteIndex(0, 0, 0);
        else
          c = _GfxGetPaletteIndex(192, 192, 192);

        for (j=0; j<(g_mnu->spacing >> 1); j++, _x++)
          _GfxDrawLine(_x, (coord)(_y - j), _x, (coord)(_y + j), c);

        _x = g_mnu->rect.width - ((MENU_ARROWS_X - (g_mnu->spacing >> 1)) / 2);
        if (g_mnu->item[i].nested.index < (g_mnu->item[i].nested.count-1))
          c = _GfxGetPaletteIndex(0, 0, 0);
        else
          c = _GfxGetPaletteIndex(192, 192, 192);
        for (j=0; j<(g_mnu->spacing >> 1); j++, _x--)
          _GfxDrawLine(_x, (coord)(_y - j), _x, (coord)(_y + j), c); _x--;

        x += (int16)_FntGetCharsWidth(g_mnu->item[i].label, 
                                      _StrLen(g_mnu->item[i].label));
        x += (MENU_SPACE_X / 2);
        x += (int16)_FntGetCharsWidth(" ", 1);

        pStr = g_mnu->item[i].nested.labels;
        for (j=0; j<g_mnu->item[i].nested.index; j++)
          pStr += _StrLen(pStr) + 1;

        _FntDrawString(pStr, _StrLen(pStr), x, y, gfxPaint);
      }

      // draw the highlighted menu item
      if (i == g_mnu->index)
      {
        rect.x      = 1;
        rect.y      = y;
        rect.width  = g_mnu->rect.width-2;
        rect.height = (int16)(_FntGetFontHeight() + 1);
        _GfxInvertRegion(&rect);
      }

      if (g_mnu->item[i].label[0] == '-') y += (int16)(_FntGetFontHeight() / 2);
      else                                y += (int16)(_FntGetFontHeight() + 1);
    }
    _GfxRepaint();
  }
  _GfxSetDrawWindow(win);

  return processed;
}

/********************************* EOF ***********************************/
