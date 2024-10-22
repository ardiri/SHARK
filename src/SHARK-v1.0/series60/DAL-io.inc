/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)series60/DAL-io.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 * Platform Implementation Structures
 *
 *************************************************************************/

typedef struct _IO_DeviceInfo
{
  struct
  {
    uint32 keyState;
  } raw_key;            // we dont have KeyCurrentState() - simulating it

} _IO_DeviceInfo;

// local functions
static boolean _KeyHandleEvent(TKeyEvent *e, TEventCode, event *_e);
static boolean _PenHandleEvent(TPointerEvent *e, event *_e);

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

void
_IOInitialize()
{
  _IO_DeviceInfo *deviceP;
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  gDALGlobals->io.info = (void *)malloc(sizeof(_IO_DeviceInfo));
  memset(gDALGlobals->io.info, 0, sizeof(_IO_DeviceInfo));
  deviceP = (_IO_DeviceInfo *)gDALGlobals->io.info;

  // initialize defaults
  gDALGlobals->io.keyMask = ~KeyAllMask;
  deviceP->raw_key.keyState = 0;
}

void
_IOTerminate()
{
  _IO_DeviceInfo *deviceP;
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  deviceP = (_IO_DeviceInfo *)gDALGlobals->io.info;

  // clean up
  free(gDALGlobals->io.info);
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

void
_KeySetMask(uint32 mask)
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  gDALGlobals->io.keyMask = mask;
}

uint32
_KeyGetMask()
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  return gDALGlobals->io.keyMask;
}

uint32
_KeyCurrentState()
{
  _IO_DeviceInfo *deviceP;
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  deviceP = (_IO_DeviceInfo *)gDALGlobals->io.info;
  return deviceP->raw_key.keyState;
}

boolean
_PenAvailable()
{
  return false;  // symbian series 60 = no stylus
}

void
_PenState(coord */*penX*/, coord */*penY*/, boolean *penDown)
{
  _IO_DeviceInfo *deviceP;
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  deviceP = (_IO_DeviceInfo *)gDALGlobals->io.info;

  // symbian doesn't support it, sorry!
  *penDown = false;
}

static boolean
_KeyHandleEvent(TKeyEvent *e, TEventCode type, event *_e)
{
  boolean         processed = false;
  _IO_DeviceInfo *deviceP;
  TUint           key;
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  deviceP = (_IO_DeviceInfo *)gDALGlobals->io.info;

// Nokia 3650:            Other / N-Gage:
//
//           ^               [1] [^] [3]       ^
//         < + >             [<] [5] [>]     < + >
//           v               [7] [v] [9]       v
//                               [0]      keys 5 and 7 = raised on n-gage

  // which key was pressed/released?
  key = e->iScanCode;
  switch (type)
  {
    // key pressed (before event)
    case EEventKeyDown:
         switch (key)
         {
           case '5':
           case EStdKeyDevice3:
                deviceP->raw_key.keyState |= KeyPadSelect;
                processed = true;
                break;

           case '2':
           case EStdKeyUpArrow:
                deviceP->raw_key.keyState |= KeyPadUp;
                processed = true;
                break;

           case '8':
           case EStdKeyDownArrow:
                deviceP->raw_key.keyState |= KeyPadDown;
                processed = true;
                break;

           case '4':
           case EStdKeyLeftArrow:
                deviceP->raw_key.keyState |= KeyPadLeft;
                processed = true;
                break;

           case '6':
           case EStdKeyRightArrow:
                deviceP->raw_key.keyState |= KeyPadRight;
                processed = true;
                break;

           case '1':
                deviceP->raw_key.keyState |= KeyOption1;
                processed = true;
                break;

           case '3':
           case '0':
                deviceP->raw_key.keyState |= KeyOption2;
                processed = true;
                break;

           case '7':
           case '9':
                deviceP->raw_key.keyState |= KeyAction;
                processed = true;
                break;

           case '*':
                deviceP->raw_key.keyState |= KeyPause;
                processed = true;
                break;

           default:
                break;
         }
         break;

    // key event (real)
    case EEventKey:
         switch (key)
         {
           case '5':
           case EStdKeyDevice3:
                if ((gDALGlobals->io.keyMask & KeyPadSelect) == 0)
                {
                  _e->eType        = _keyEvent;
                  _e->data.key.chr = _key_padselect;
                  processed = true;
                }
                break;

           case '2':
           case EStdKeyUpArrow:
                if ((gDALGlobals->io.keyMask & KeyPadUp) == 0)
                {
                  _e->eType        = _keyEvent;
                  _e->data.key.chr = _key_padup;
                  processed = true;
                }
                break;

           case '8':
           case EStdKeyDownArrow:
                if ((gDALGlobals->io.keyMask & KeyPadDown) == 0)
                {
                  _e->eType        = _keyEvent;
                  _e->data.key.chr = _key_paddown;
                  processed = true;
                }
                break;

           case '4':
           case EStdKeyLeftArrow:
                if ((gDALGlobals->io.keyMask & KeyPadLeft) == 0)
                {
                  _e->eType        = _keyEvent;
                  _e->data.key.chr = _key_padleft;
                  processed = true;
                }
                break;

           case '6':
           case EStdKeyRightArrow:
                if ((gDALGlobals->io.keyMask & KeyPadRight) == 0)
                {
                  _e->eType        = _keyEvent;
                  _e->data.key.chr = _key_padright;
                  processed = true;
                }
                break;

           case '1':
                if ((gDALGlobals->io.keyMask & KeyOption1) == 0)
                {
                  _e->eType        = _keyEvent;
                  _e->data.key.chr = _key_option1;
                  processed = true;
                }
                break;

           case '3':
           case '0':
                if ((gDALGlobals->io.keyMask & KeyOption2) == 0)
                {
                  _e->eType        = _keyEvent;
                  _e->data.key.chr = _key_option2;
                  processed = true;
                }
                break;

           case '7':
           case '9':
                if ((gDALGlobals->io.keyMask & KeyAction) == 0)
                {
                  _e->eType        = _keyEvent;
                  _e->data.key.chr = _key_action;
                  processed = true;
                }
                break;

           case '*':
                if ((gDALGlobals->io.keyMask & KeyPause) == 0)
                {
                  _e->eType        = _keyEvent;
                  _e->data.key.chr = _key_pause;
                  processed = true;
                }
                break;

           default:
                break;
         }
         break;

    // key released (after event)
    case EEventKeyUp:
         switch (key)
         {
           case '5':
           case EStdKeyDevice3:
                deviceP->raw_key.keyState &= ~KeyPadSelect;
                processed = true;
                break;

           case '2':
           case EStdKeyUpArrow:
                deviceP->raw_key.keyState &= ~KeyPadUp;
                processed = true;
                break;

           case '8':
           case EStdKeyDownArrow:
                deviceP->raw_key.keyState &= ~KeyPadDown;
                processed = true;
                break;

           case '4':
           case EStdKeyLeftArrow:
                deviceP->raw_key.keyState &= ~KeyPadLeft;
                processed = true;
                break;

           case '6':
           case EStdKeyRightArrow:
                deviceP->raw_key.keyState &= ~KeyPadRight;
                processed = true;
                break;

           case '1':
                deviceP->raw_key.keyState &= ~KeyOption1;
                processed = true;
                break;

           case '3':
           case '0':
                deviceP->raw_key.keyState &= ~KeyOption2;
                processed = true;
                break;

           case '7':
           case '9':
                deviceP->raw_key.keyState &= ~KeyAction;
                processed = true;
                break;

           case '*':
                deviceP->raw_key.keyState &= ~KeyPause;
                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}

static boolean
_PenHandleEvent(TPointerEvent *e, event *_e)
{
  _IO_DeviceInfo *deviceP;
  boolean processed = false;
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  deviceP = (_IO_DeviceInfo *)gDALGlobals->io.info;

  switch (e->iType)
  {
    // normal event (pen/mouse down)
    case TPointerEvent::EButton1Down:
         _e->eType   = _penEvent;
         _e->penDown = true;
         _e->screenX = _e->data.pen.x = (coord)e->iPosition.iX;
         _e->screenY = _e->data.pen.y = (coord)e->iPosition.iY;

         processed = true;
         break;

    // drag event (button 1)
    case TPointerEvent::EDrag:
         _e->eType   = _penEvent;
         _e->penDown = true;
         _e->screenX = _e->data.pen.x = (coord)e->iPosition.iX;
         _e->screenY = _e->data.pen.y = (coord)e->iPosition.iY;

         processed = true;
         break;

    // normal event (pen/mouse up)
    case TPointerEvent::EButton1Up:
         break;

    // not interested in these for now (ignore them for now)
    case TPointerEvent::EMove:              // mouse moved
    case TPointerEvent::EButton2Down:
    case TPointerEvent::EButton2Up:
    case TPointerEvent::EButton3Down:
    case TPointerEvent::EButton3Up:
    case TPointerEvent::ESwitchOn:          // power on via pointer event?
    case TPointerEvent::EButtonRepeat:
    default:
         break;
  }

  // only valid if within range of drawable area :)
  return (processed &&
          (_e->screenX < (coord)gDALGlobals->gfx.width) &&
          (_e->screenY < (coord)gDALGlobals->gfx.height));
}

boolean
_PenDisplayToWindow(coord */*penX*/, coord */*penY*/)
{
  // pointer is already relative to window - continue!
  return true;
}

/********************************* EOF ***********************************/
