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

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct
{
  uint32 keyState;
} InpGlobalsType;

#define GLOBALS_DEFINE_INP InpGlobalsType *g_inp;
#define GLOBALS_ACCESS_INP GLOBALS_ACCESS; \
                           InpGlobalsType *g_inp = (InpGlobalsType *)g->inp;

#define g_keyState g_inp->keyState

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static void    _InputKeyStateSet(uint32 keyState);
static boolean _PenHandleEvent(coord x, coord y, event *_e);
static boolean _KeyHandleEvent(uint16 key, TEventCode type, event *_e);

// initialize the input manager
static boolean
_InputInitialize()
{
  boolean init_ok;
  GLOBALS_ACCESS;
  GLOBALS_DEFINE_INP;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _InputInitialize");
#endif

  // assume everything is ok
  init_ok = true;

  // globals:: allocate appropriate memory
  g_inp = (InpGlobalsType *)malloc(sizeof(InpGlobalsType));
  init_ok = (g_inp != NULL); if (!init_ok) goto INPUT_INIT_ABORT;
  memset(g_inp, 0, sizeof(InpGlobalsType));
  g->inp = g_inp;

  // initialize
  g_keyState = 0;

INPUT_INIT_ABORT:

  return init_ok;
}

// return the input mechanisms supported on the platform
uint32
_InputSupported()
{
  uint32 result;
//GLOBALS_ACCESS_INP;

  // default return value
  result = (INPUT_KEY);

  return result;
}

// get the current state of the digital and analog keyboard inputs
uint32
_KeyCurrentState(uint8 *x, uint8 *y)
{
  uint32 result;
  GLOBALS_ACCESS_INP;

  // default return value
  result = g_keyState;

  // symbian: no analog control
  if (x) *x = 0;
  if (y) *y = 0;
  
  return result;
}

// get the current state of the pen
void
_PenState(coord *penX, coord *penY, boolean *penDown)
{
//GLOBALS_ACCESS_INP;

  // entry requirements
  if (penDown == NULL) return;

  // do we actually have input support for the pen?
  if ((_InputSupported() & INPUT_STYLUS))
  {
    // TODO
  }
  else
  {
    *penDown = false;
    if (penX != NULL)    *penX = (coord)0;
    if (penY != NULL)    *penY = (coord)0;
  }
}

// terminate the input management
static void
_InputTerminate()
{
  GLOBALS_ACCESS_INP;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _InputTerminate");
#endif

  // globals:: clean up memory
  free(g_inp); g_inp = NULL;
  g->inp = g_inp;
}

static void
_InputKeyStateSet(uint32 keyState)
{
  GLOBALS_ACCESS_INP;

  g_keyState = keyState;
}

// handle a pen event from the platform
static boolean
_PenHandleEvent(coord x, coord y, event *_e)
{
  boolean result;
//GLOBALS_ACCESS_INP;

  // default return value
  result = false;

  // do we actually have input support for the pen?
  if ((_InputSupported() & INPUT_STYLUS))
  {
    // TODO
  }

  return result;
}

// handle a key event from the platform
static boolean
_KeyHandleEvent(uint16 key, TEventCode type, event *_e)
{
  boolean         processed;
  lcd_orientation orientation;
  GLOBALS_ACCESS_INP;

  // default return value
  processed = false;

// Nokia 3650:            Other / N-Gage:
//
//           ^               [1] [^] [-]       ^
//         < + >             [<] [5] [>]     < + >
//           v               [7] [v] [-]       v
//                           [A] [B] [C]    keys 5 and 7 = raised on n-gage
//

  // which key are we processing?
  switch (type)
  {
    // key pressed (before event)
    case EEventKeyDown:
         switch (key)
         {
           case '5':
           case EStdKeyDevice3: // dpad center
                g_keyState |= KeyPadSelect;
                processed = true;
                break;

           case '2':
           case EStdKeyUpArrow:
                g_keyState |= KeyPadUp;
                processed = true;
                break;

           case '8':
           case EStdKeyDownArrow:
                g_keyState |= KeyPadDown;
                processed = true;
                break;

           case '4':
           case EStdKeyLeftArrow:
                g_keyState |= KeyPadLeft;
                processed = true;
                break;

           case '6':
           case EStdKeyRightArrow:
                g_keyState |= KeyPadRight;
                processed = true;
                break;

           case '*':
                g_keyState |= KeyActionA;
                processed = true;
                break;

           case '0':
                g_keyState |= KeyActionB;
                processed = true;
                break;

           case EStdKeyHash:
                g_keyState |= KeyActionC;
                processed = true;
                break;

           case '3':
                g_keyState |= KeyOption;
                processed = true;
                break;

           default:
                break;
         }
         break;

    // key pressed (actual)
    case EEventKey:
         switch (key)
         {
           case '1':
                // special hack - rotate through orientations (we know of 4) :)
                _LCDGetProperties(NULL,NULL,NULL,NULL,NULL,&orientation,NULL);
                orientation = (lcd_orientation)((orientation + 1) & 0x03);
                _LCDSetOrientation(orientation);
                break;
     
           case EStdKeyLeftShift:  // shift button
           case EStdKeyRightShift: // shift button
                _SysEnqueueQuitEvent();
                break;

           case EStdKeyDevice0: // left softkey
                _e->eType        = _keyEvent;
                _e->data.key.chr = _key_menu;
                processed = true;
                break;

           case EStdKeyDevice1: // right softkey
                _e->eType        = _keyEvent;
                _e->data.key.chr = _key_acknowledge;
                processed = true;
                break;

           case EStdKeyBackspace: 
                _e->eType        = _keyEvent;
                _e->data.key.chr = _key_backspace;
                processed = true;
                break;

           case '5':
           case EStdKeyDevice3: // dpad center
                _e->eType        = _keyEvent;
                _e->data.key.chr = _key_padselect;
                processed = true;
                break;

           case '2':
           case EStdKeyUpArrow:
                _e->eType        = _keyEvent;
                _e->data.key.chr = _key_padup;
                processed = true;
                break;

           case '8':
           case EStdKeyDownArrow:
                _e->eType        = _keyEvent;
                _e->data.key.chr = _key_paddown;
                processed = true;
                break;

           case '4':
           case EStdKeyLeftArrow:
                _e->eType        = _keyEvent;
                _e->data.key.chr = _key_padleft;
                processed = true;
                break;

           case '6':
           case EStdKeyRightArrow:
                _e->eType        = _keyEvent;
                _e->data.key.chr = _key_padright;
                processed = true;
                break;

           case '*':
                _e->eType        = _keyEvent;
                _e->data.key.chr = _key_actionA;
                processed = true;
                break;

           case '0':
                _e->eType        = _keyEvent;
                _e->data.key.chr = _key_actionB;
                processed = true;
                break;

           case EStdKeyHash:
                _e->eType        = _keyEvent;
                _e->data.key.chr = _key_actionC;
                processed = true;
                break;

           case '3':
                _e->eType        = _keyEvent;
                _e->data.key.chr = _key_option;
                processed = true;
                break;

           default:
                break;
         }
         break;

    // key pressed (after event)
    case EEventKeyUp:
         switch (key)
         {
           case '5':
           case EStdKeyDevice3: // dpad center
                g_keyState &= ~KeyPadSelect;
                processed = true;
                break;

           case '2':
           case EStdKeyUpArrow:
                g_keyState &= ~KeyPadUp;
                processed = true;
                break;

           case '8':
           case EStdKeyDownArrow:
                g_keyState &= ~KeyPadDown;
                processed = true;
                break;

           case '4':
           case EStdKeyLeftArrow:
                g_keyState &= ~KeyPadLeft;
                processed = true;
                break;

           case '6':
           case EStdKeyRightArrow:
                g_keyState &= ~KeyPadRight;
                processed = true;
                break;

           case '*':
                g_keyState &= ~KeyActionA;
                processed = true;
                break;

           case '0':
                g_keyState &= ~KeyActionB;
                processed = true;
                break;

           case EStdKeyHash:
                g_keyState &= ~KeyActionC;
                processed = true;
                break;

           case '3':
                g_keyState &= ~KeyOption;
                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  // are we creating a key event? if so, need to orientate it
  if (_e->eType == _keyEvent)
  {
    _LCDGetProperties(NULL,NULL,NULL,NULL,NULL,&orientation,NULL);
    switch (orientation)
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

#undef GLOBALS_DEFINE_INP
#undef GLOBALS_ACCESS_INP

#undef g_keyState

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-input.inc"

/********************************* EOF ***********************************/
