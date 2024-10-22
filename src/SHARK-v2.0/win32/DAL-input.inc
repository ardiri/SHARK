/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-input.c
 */

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

// -----------------------------------------------------------------------
// compile time options
// -----------------------------------------------------------------------

// PEN_USE_RAW_EVENT
//
// _PenHandleEvent(...)
//   -> flag to determine if the raw pen event from the device event
//      buffer should be used in the event generation.
//
//      = 0 - means use _PenState() to get the current pen location
//      = 1 - means use the (x,y) co-ordinate from the event

   #define PEN_USE_RAW_EVENT      1      // use the raw event

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static boolean _PenHandleEvent(coord x, coord y, event *_e);
static boolean _KeyHandleEvent(uint16 key, event *_e);

// initialize the input manager
static boolean
_InputInitialize()
{
  boolean  init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _InputInitialize");
#endif

  // assume everything is ok
  init_ok = true;

  return init_ok;
}

// return the input mechanisms supported on the platform
uint32
_InputSupported()
{
  uint32 result;

  // default return value
  result = (INPUT_STYLUS | INPUT_KEYS | INPUT_KEYBOARD | INPUT_PHONEPAD);

  return result;
}

// get the current state of the digital and analog keyboard inputs
uint32
_KeyCurrentState(uint8 *x, uint8 *y)
{
  uint32 result;
  MSG    msg;

  // default return value
  result = 0;

  // DIGITAL KEYS: keyboard mappings
  //   http://msdn.microsoft.com/library/default.asp?
  //     url=/library/en-us/winui/winui/WindowsUserInterface/UserInput/VirtualKeyCodes.asp

  // documented keys
  if (GetAsyncKeyState(VK_LEFT))      result |= KeyPadLeft;
  if (GetAsyncKeyState(VK_RIGHT))     result |= KeyPadRight;
  if (GetAsyncKeyState(VK_UP))        result |= KeyPadUp;
  if (GetAsyncKeyState(VK_DOWN))      result |= KeyPadDown;
  if (GetAsyncKeyState(VK_RETURN))    result |= KeyPadSelect;
  if (GetAsyncKeyState('A'))          result |= KeyActionA;
  if (GetAsyncKeyState('B'))          result |= KeyActionB;
  if (GetAsyncKeyState('C'))          result |= KeyActionC; // for games

  // alternative (standard keyboard)
  if (GetAsyncKeyState(VK_CONTROL))   result |= KeyActionA;
  if (GetAsyncKeyState(VK_SHIFT))     result |= KeyActionB;
  if (GetAsyncKeyState(VK_TAB))       result |= KeyActionC;
  if (GetAsyncKeyState(VK_SPACE))     result |= KeyPadSelect;

  // alternative (keypad)
  if (GetAsyncKeyState(VK_NUMPAD4))   result |= KeyPadLeft;
  if (GetAsyncKeyState(VK_NUMPAD6))   result |= KeyPadRight;
  if (GetAsyncKeyState(VK_NUMPAD8))   result |= KeyPadUp;
  if (GetAsyncKeyState(VK_NUMPAD2))   result |= KeyPadDown;
  if (GetAsyncKeyState(VK_NUMPAD5))   result |= KeyPadSelect;

  // other action keys
  if (GetAsyncKeyState(VK_F1))        result |= KeyOption;

  // flush the key queue
  PeekMessage(&msg, g_hWnd, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE);

  // ANALOG KEYS: not supported in win32

  return result;
}

// get the current state of the pen
void
_PenState(coord *penX, coord *penY, boolean *penDown)
{
  POINT pt;
  MSG   msg;
  coord tmp;

  // entry requirements
  if (penDown == NULL) return;

  // do we actually have input support for the pen?
  if ((_InputSupported() & INPUT_STYLUS))
  {
    *penDown = (boolean)(GetKeyState(VK_LBUTTON) < 0);
    if (*penDown)
    {
      GetCursorPos(&pt);
      ScreenToClient(g_hWnd, &pt);

      // lets ensure the pen is within the screen boundaries
      if (((pt.x > 0) && (pt.x < g_width) &&
           (pt.y > 0) && (pt.y < g_height)))
      {
        if (penY != NULL) *penX = (coord)pt.x;
        if (penY != NULL) *penY = (coord)pt.y;
      }
      else
        *penDown = false;

      // rotation required?
      if ((penY != NULL) && (penY != NULL))
      {
        switch (g_lcd_orientation)
        {
          case lcd_rotate90:
               tmp   = *penX;
               *penX = *penY;
               *penY = g_width  - tmp;
               break;

          case lcd_rotate180:
               *penX = g_width  - *penX;
               *penY = g_height - *penY;
               break;

          case lcd_rotate270:
               tmp   = *penX;
               *penX = g_height - *penY;
               *penY = tmp;
               break;

          default:
          case lcd_rotate0:
               break;
        }
      }

      // flush the pen queue
      PeekMessage(&msg, g_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE);
    }
  }
  else
    *penDown = false;

  // there is no concept of a pen unless the pen is down
  if (!*penDown)
  {
    if (penY != NULL) *penX = (coord)0;
    if (penY != NULL) *penY = (coord)0;
  }
}

// terminate the input management
static void
_InputTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _InputTerminate");
#endif
}

// handle a pen event from the platform
static boolean
_PenHandleEvent(coord x, coord y, event *_e)
{
  coord   penX, penY;
#if (PEN_USE_RAW_EVENT == 1)
  coord   tmp;
#endif
  boolean penDown;
  boolean result;

  // default return value
  result = false;

  // do we actually have input support for the pen?
  if ((_InputSupported() & INPUT_STYLUS))
  {
#if (PEN_USE_RAW_EVENT == 0)
    _PenState(&penX, &penY, &penDown);
#else
    penX = x; penY = y;
    _PenState(NULL, NULL, &penDown);

    // rotation required?
    switch (g_lcd_orientation)
    {
      case lcd_rotate90:
           tmp  = penX;
           penX = penY;
           penY = g_width  - tmp;
           break;

      case lcd_rotate180:
           penX = g_width  - penX;
           penY = g_height - penY;
           break;

      case lcd_rotate270:
           tmp  = penX;
           penX = g_height - penY;
           penY = tmp;
           break;

      default:
      case lcd_rotate0:
           break;
    }
#endif

    if (penDown)
    {
      _e -> eType   = _penEvent;
      _e -> penDown = penDown;
      _e -> screenX = _e -> data.pen.x = penX;
      _e -> screenY = _e -> data.pen.y = penY;
      result = true;
    }
  }

  return result;
}

// handle a key event from the platform
static boolean
_KeyHandleEvent(uint16 key, event *_e)
{
  boolean processed;

  // default return value
  processed = false;

  // which key are we processing?
  switch (key)
  {
    case VK_ESCAPE:
         _SysEnqueueQuitEvent();
         processed = false;
         break;

    case VK_LEFT:
    case VK_NUMPAD4:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_padleft;
         processed = true;
         break;

    case VK_RIGHT:
    case VK_NUMPAD6:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_padright;
         processed = true;
         break;

    case VK_UP:
    case VK_NUMPAD8:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_padup;
         processed = true;
         break;

    case VK_DOWN:
    case VK_NUMPAD2:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_paddown;
         processed = true;
         break;

    case VK_RETURN:
    case VK_NUMPAD5:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_padselect;
         processed = true;
         break;

    case VK_F1:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_option;
         processed = true;
         break;

    case VK_F2:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_menu;
         processed = true;
         break;

    case VK_F3:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_acknowledge;
         processed = true;
         break;

    case VK_F4:
         // special hack - rotate through orientations (we know of 4) :)
         _LCDSetOrientation((g_lcd_orientation + 1) & 0x03);
         processed = false;
         break;

    case VK_F5:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_actionA;
         processed = true;
         break;

    case VK_F6:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_actionB;
         processed = true;
         break;

    case VK_F7:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_actionC;
         processed = true;
         break;

    case VK_PRIOR:
         processed = true;
         break;

    case VK_NEXT:
         processed = true;
         break;

    case VK_BACK:
    case VK_DELETE:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_backspace;
         processed = true;
         break;

    default:

//
// NOTE: if we want to add more keys, the keycode can be obtained here:
//
// {
//   char str[32];
//   sprintf(str, "_KeyHandleEvent(e) : key = %d", key);
//   _SysDebugMessage(str, true); 
// }
//

         break;
  }

  // are we creating a key event? if so, need to orientate it
  if (_e->eType == _keyEvent)
  {
    switch (g_lcd_orientation)
    {
      case lcd_rotate90:

if (_e->data.key.chr == _key_padup)    _e->data.key.chr = _key_padleft;  else
if (_e->data.key.chr == _key_paddown)  _e->data.key.chr = _key_padright; else
if (_e->data.key.chr == _key_padleft)  _e->data.key.chr = _key_paddown;  else
if (_e->data.key.chr == _key_padright) _e->data.key.chr = _key_padup;

           break;

      case lcd_rotate180:

if (_e->data.key.chr == _key_padup)    _e->data.key.chr = _key_paddown;  else
if (_e->data.key.chr == _key_paddown)  _e->data.key.chr = _key_padup;    else
if (_e->data.key.chr == _key_padleft)  _e->data.key.chr = _key_padright; else
if (_e->data.key.chr == _key_padright) _e->data.key.chr = _key_padleft;

           break;

      case lcd_rotate270:

if (_e->data.key.chr == _key_padup)    _e->data.key.chr = _key_padright; else
if (_e->data.key.chr == _key_paddown)  _e->data.key.chr = _key_padleft;  else
if (_e->data.key.chr == _key_padleft)  _e->data.key.chr = _key_padup;    else
if (_e->data.key.chr == _key_padright) _e->data.key.chr = _key_paddown;

           break;

      default:
           break;
    }
  }

  return processed;
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-input.inc"

/********************************* EOF ***********************************/
