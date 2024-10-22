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
  int32  theKeys[4];

  // default return value
  result = 0;

  // DIGITAL KEYS: 
  //   http://developer.apple.com/documentation/Carbon/Reference/
  //      Event_Manager/event_mgr_ref/chapter_1.2_section_5.html

  // lets get the key mapping
  GetKeys(theKeys);

  // documented keys
  if (theKeys[3] & 0x00000008) result |= KeyPadLeft;
  if (theKeys[3] & 0x00000010) result |= KeyPadRight;
  if (theKeys[3] & 0x00000040) result |= KeyPadUp;
  if (theKeys[3] & 0x00000020) result |= KeyPadDown;
  if (theKeys[1] & 0x10000000) result |= KeyPadSelect;
  if (theKeys[0] & 0x01000000) result |= KeyActionA;   // 'A'
  if (theKeys[0] & 0x00080000) result |= KeyActionB;   // 'B' 
  if (theKeys[0] & 0x00010000) result |= KeyActionC;   // 'C' - for games

  // alternative (standard keyboard)
  if (theKeys[1] & 0x00000008) result |= KeyActionA;   // 'CTL'
  if (theKeys[1] & 0x00000001) result |= KeyActionB;   // 'SHIFT' 
  if (theKeys[1] & 0x00000100) result |= KeyActionC;   // 'TAB'
  if (theKeys[1] & 0x00000200) result |= KeyPadSelect; // ' '
  
  // alternative (keypad)

  // other action keys
  if (theKeys[3] & 0x00000004) result |= KeyOption;    // 'F1'

  // ANALOG KEYS: not supported in macosx

  return result;
}

// get the current state of the pen
void
_PenState(coord *penX, coord *penY, boolean *penDown)
{
  Point pt;
  coord tmp;

  // entry requirements
  if (penDown == NULL) return;

  // do we actually have input support for the pen?
  if ((_InputSupported() & INPUT_STYLUS))
  {
    *penDown = Button();
    if (*penDown)
    {
      GetMouse(&pt);

      // lets ensure the pen is within the screen boundaries
      if (((pt.h > 0) && (pt.h < g_width) &&
           (pt.v > 0) && (pt.v < g_height)))
      {
        if (penY != NULL) *penX = (coord)pt.h;
        if (penY != NULL) *penY = (coord)pt.v;
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
    }
  }
  else
    *penDown = false;

  // there is no concept of a pen event unless the pen is down
  if (!*penDown)
  {
    if (penX != NULL) *penX = (coord)0;
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
  coord   tmp;
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
    case 53: // 'ESC' 
         _SysEnqueueQuitEvent();
         QuitApplicationEventLoop();
         processed = false;
         break;

    case 123:  // 'LEFT'
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_padleft;
         processed = true;
         break;

    case 124:  // 'RIGHT'
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_padright;
         processed = true;
         break;

    case 126: // 'UP'
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_padup;
         processed = true;
         break;

    case 125: // 'DOWN'
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_paddown;
         processed = true;
         break;

    case 36:  // 'RETURN'
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_padselect;
         processed = true;
         break;

    case 122: // 'F1'
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_option;
         processed = true;
         break;

    case 120: // 'F2'
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_menu;
         processed = true;
         break;

    case 99:  // 'F3'
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_acknowledge;
         processed = true;
         break;

    case 118: // 'F4'
         // special hack - rotate through orientations (we know of 4) :)
         _LCDSetOrientation((g_lcd_orientation + 1) & 0x03);
         processed = false;
         break;

    case 96: // 'F5'
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_actionA;
         processed = true;
         break;

    case 97: // 'F6'
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_actionB;
         processed = true;
         break;

    case 98: // 'F7'
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_actionC;
         processed = true;
         break;

    case 51:  // 'DELETE'
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_backspace;
         processed = true;
         break;

    default:

//
// NOTE: if we want to add more keys, the keycode can be obtained here
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
