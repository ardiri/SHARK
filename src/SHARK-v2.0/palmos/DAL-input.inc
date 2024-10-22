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
  boolean init_ok;

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
  result = (INPUT_STYLUS | INPUT_KEYS);

  // now, lets check device by device level
  switch (g -> ftr.comID)
  {
    case 'palm':
    case 'Palm': // Palm
         switch (g -> ftr.devID)
         {
           case 'Frg1': // T|T
           case 'Frg2': // T|T2
           case 'Cct1': // T|E
           case 'Zir4': // T|E2
           case 'D051': // Zire 22
           case 'Zi22': // Zire 31
           case 'Zpth': // Zire 71
           case 'Zi72': // Zire 72

                // these devices have grafitti areas (always)
                result |= INPUT_KEYBOARD;
                break;

           case 'MT64': // T|C
                // these devices have a real keyboard (always)
                result |= INPUT_KEYBOARD;
                break;

           case 'Arz1': // T|T3
                // BUG: with DIA support, forced grafitti area
                result |= INPUT_KEYBOARD;
                break;

           case 'TnT5': // T|T5
           case 'TunX': // LifeDrive
           case 'D050': // T|X
                // these devices have full screen DIA support 
                break;

           default:
                // NEW DEVICE:: we need to add information here :)
                break;
         }
         break;

    case 'hspr':
         switch (g -> ftr.devID)
         {
           case 'H101': // Treo 600
           case 'H102': // Treo 650
                // these devices have a real keyboard (always)
                result |= INPUT_KEYBOARD;
                break;

           default:
                // NEW DEVICE:: we need to add information here :)
                break;
         }
         break;

    case 'grmn':
         switch (g -> ftr.devID)
         {
           case '3000': // iQue 3000
           case '3200': // iQue 3200
                // these devices have grafitti areas (always)
                result |= INPUT_KEYBOARD;
                break;

           case '3600': // iQue 3600
           case '3700': // iQue 3600a
                // these devices have full screen DIA support (via PACE)
                break;

           default:
                // NEW DEVICE:: we need to add information here :)
                break;
         }
         break;

    case 'sony':
         switch (g -> ftr.devID)
         {
           case 'goha': // TJ25
           case 'leia': // TJ27
           case 'goku': // TJ35
           case 'luke': // TJ37
                // these devices have grafitti areas (always)
                result |= INPUT_KEYBOARD;
                break;

           case 'amno': // UX40
           case 'prmr': // UX50
           case 'grnd': // NZ90
           case 'tldo': // NX73V
           case 'mdrd': // NX80V
           case 'crdb': // NX60/NX70
                // these devices have physical keyboards (always)
                result |= INPUT_KEYBOARD;
                break;

           case 'atom': // TH55
           case 'vrna': // TG50
           case 'ancy': // VZ90
                // until we implement DIA support, forced grafitti area
                result |= INPUT_KEYBOARD;
                break;

           default:
                // NEW DEVICE:: we need to add information here :)
                break;
         }
         break;

    default:
         // NEW DEVICE:: we need to add information here :)
         break;
  }

  // all devices with keyboards have a phonepad
  if (result & INPUT_KEYBOARD) result |= INPUT_PHONEPAD;

  return result;
}

// get the current state of the digital and analog keyboard inputs
uint32
_KeyCurrentState(uint8 *x, uint8 *y)
{
  uint32 result;
  UInt32 keyState;

  // default return value
  result = 0;

  // get the system key state
  keyState = KeyCurrentState();

// TODO

  // ANALOG KEYS: not supported in palmos (yet)

  return result;
}

// get the current state of the pen
void
_PenState(coord *penX, coord *penY, boolean *penDown)
{
  Coord x, y;
  coord tmp;

  // entry requirements
  if (penDown == NULL) return;

  // do we actually have input support for the pen?
  if ((_InputSupported() & INPUT_STYLUS))
  {
    EvtGetPenNative(WinGetDisplayWindow(), &x, &y, penDown);

    // lets ensure the pen is within the screen boundaries
    if (((x > 0) && (x < g_width) &&
         (y > 0) && (y < g_height)))
    {
      if (penY != NULL) *penX = (coord)x;
      if (penY != NULL) *penY = (coord)y;
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

    // we need to take into consideration screen density!
    penX = (penX * g -> ftr.scr_density) / 72;
    penY = (penY * g -> ftr.scr_density) / 72;

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

      // only valid if within range of drawable area :)
      result = ((_e -> screenX >= 0) &&
                (_e -> screenY >= 0) &&
                (_e -> screenX < g_width) &&
                (_e -> screenY < g_height));
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
    case chrBackspace:
         _e -> eType        = _keyEvent;
         _e -> data.key.chr = _key_backspace;

         processed = true;
         break;

    case chrMenu:
         _e -> eType        = _keyEvent;
         _e -> data.key.chr = _key_menu;
    
         processed = true;
         break;

    case chrPageUp:
    case chrRockerUp:
    case chrJogUp:
    case chrThumbWheelUp:
         _e -> eType        = _keyEvent;
         _e -> data.key.chr = _key_padup;
  
         processed = true;
         break;

    case chrPageDown:
    case chrRockerDown:
    case chrJogDown:
    case chrThumbWheelDown:
         _e -> eType        = _keyEvent;
         _e -> data.key.chr = _key_paddown;
  
         processed = true;
         break;

    case chrRockerLeft:
         _e -> eType        = _keyEvent;
         _e -> data.key.chr = _key_padleft;
  
         processed = true;
         break;

    case chrRockerRight:
         _e -> eType        = _keyEvent;
         _e -> data.key.chr = _key_padright;
  
         processed = true;
         break;

    case chrEnter:
    case chrRockerCenter:
    case chrHardRockerCenter:
    case chrThumbWheelPush:
    case chrJogPress:
         _e -> eType        = _keyEvent;
         _e -> data.key.chr = _key_padselect;
  
         processed = true;
         break;

    case chrNavChange:
         // handled in DAL.c (need keyCode)
         break;

    default:

         // NOTE: we support max 0xff latin-1
         if ((key >= 32) && (key < 0x100))
         {
           _e -> eType = _keyEvent;
           if (key < 0x80)
           {
             _e -> data.key.size      = 1;
             _e -> data.key.chr       = (uint8)key;
           }
           else
           {
             _e -> data.key.size      = 2;
             _e -> data.key.chr       = 0xc0 | (uint8)((key >> 6) & 0x03);
             _e -> data.key.chr_x[0]  = 0x80 | (uint8) (key       & 0x3f);
           }
           processed = true;
         }

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
  if (_e -> eType == _keyEvent)
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
