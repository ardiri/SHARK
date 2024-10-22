/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)windows/DAL-io.inc
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
  int dummy;
} _IO_DeviceInfo;

// local functions
static boolean _KeyHandleEvent(UINT key, event *_e);
static boolean _PenHandleEvent(event *_e);

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

void
_IOInitialize()
{
  gDALGlobals.io.info = (void *)malloc(sizeof(_IO_DeviceInfo));
  memset(gDALGlobals.io.info, 0, sizeof(_IO_DeviceInfo));

  // initialize defaults
  gDALGlobals.io.keyMask = ~KeyAllMask;
}

void
_IOTerminate()
{
  // clean up
  free(gDALGlobals.io.info);
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

void
_KeySetMask(uint32 mask)
{
  gDALGlobals.io.keyMask = mask;
}

uint32
_KeyGetMask()
{
  return gDALGlobals.io.keyMask;
}

uint32
_KeyCurrentState()
{
  uint32          result     = 0;

  // standard keys
  if (GetAsyncKeyState(VK_LEFT))      result |= KeyPadLeft;
  if (GetAsyncKeyState(VK_RIGHT))     result |= KeyPadRight;
  if (GetAsyncKeyState(VK_UP))        result |= KeyPadUp;
  if (GetAsyncKeyState(VK_DOWN))      result |= KeyPadDown;
#ifdef _WIN32_WCE
  if (GetAsyncKeyState(0x86))         result |= KeyPadSelect;           // special select?
  if (GetAsyncKeyState(0xC1) ||
      GetAsyncKeyState(0xC2) ||
      GetAsyncKeyState(0xC3) ||
      GetAsyncKeyState(0xC4) ||
      GetAsyncKeyState(0x72) ||                                         // reject phone (O2)
      GetAsyncKeyState(0x73))         result |= KeyAction;              // accept phone (O2)
#else
  if (GetAsyncKeyState(VK_RETURN))    result |= KeyPadSelect;
  if (GetAsyncKeyState(VK_F1) ||
      GetAsyncKeyState(VK_F2) ||
      GetAsyncKeyState(VK_F3) ||
      GetAsyncKeyState(VK_F4))        result |= KeyAction;
  if (GetAsyncKeyState(VK_PRIOR))     result |= KeyScrollUp;
  if (GetAsyncKeyState(VK_NEXT))      result |= KeyScrollDown;
#endif

  // key pad [on phone]
#ifdef _WIN32_WCE
  if (GetAsyncKeyState('4'))          result |= KeyPadLeft;
  if (GetAsyncKeyState('6'))          result |= KeyPadRight;
  if (GetAsyncKeyState('2'))          result |= KeyPadUp;
  if (GetAsyncKeyState('8'))          result |= KeyPadDown;
  if (GetAsyncKeyState('5'))          result |= KeyPadSelect;
#endif

  if (GetAsyncKeyState('1'))          result |= KeyOption1;
  if (GetAsyncKeyState('3'))          result |= KeyOption2;
  if (GetAsyncKeyState('0'))          result |= KeyOption2;

#ifdef _WIN32_WCE
  if (GetAsyncKeyState(0x77))         result |= KeyPause;
#else
  if (GetAsyncKeyState(VK_F5))        result |= KeyPause;
#endif

  {
    MSG msg;

    PeekMessage(&msg, gDALGlobals.LCDWnd, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE);
  }

  return result;
}

boolean
_PenAvailable()
{
#ifdef _WIN32_SPV
  return false;  // no stylus on smartphone 2002
#else
  return true;   // pocket pc / windows have a stylus
#endif
}

void
_PenState(coord *penX, coord *penY, boolean *penDown)
{
#if defined(_WIN32_WCE) && !defined(_WIN32_WCE_EMULATION)
  *penDown = (boolean)(GetAsyncKeyState(VK_LBUTTON));
#else
  *penDown = (boolean)(GetKeyState(VK_LBUTTON) < 0);
#endif
  if (*penDown)
  {
    POINT pt;

#ifdef _WIN32_WCE
    DWORD pos;

    pos = GetMessagePos();
    pt.x = LOWORD(pos);
    pt.y = HIWORD(pos);
#else
    GetCursorPos(&pt);
#endif

    ScreenToClient(gDALGlobals.LCDWnd, &pt);

    if (penX != NULL) *penX = (coord)pt.x;
    if (penY != NULL) *penY = (coord)pt.y;

    // flush the pen queue
    {
      MSG msg;
      PeekMessage(&msg, gDALGlobals.LCDWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE);
    }

#ifndef _WIN32_WCE
    *penX = (coord)(*penX / gDALGlobals.gfx.stretch_x);
    *penY = (coord)(*penY / gDALGlobals.gfx.stretch_y);
#endif
  }
}

static boolean
_KeyHandleEvent(UINT key, event *_e)
{
  boolean processed = false;

  // special case #1: hard keys
  {
    switch (key)
    {
#ifdef _WIN32_WCE
      case 0xC1:    // VK_APP1 (in later PPC build environments)
      case 0xC2:    // VK_APP2 (in later PPC build environments)
      case 0xC3:    // VK_APP3 (in later PPC build environments)
      case 0xC4:    // VK_APP4 (in later PPC build environments)
      case 0x72:
      case 0x73:
#else
      case VK_F1:
      case VK_F2:
      case VK_F3:
      case VK_F4:
#endif
           if ((gDALGlobals.io.keyMask & KeyAction) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_action;
             processed = true;
           }
           break;

      case VK_UP:
      case '2':
           if ((gDALGlobals.io.keyMask & KeyPadUp) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_padup;
             processed = true;
           }
           break;

      case VK_DOWN:
      case '8':
           if ((gDALGlobals.io.keyMask & KeyPadDown) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_paddown;
             processed = true;
           }
           break;

      case VK_LEFT:
      case '4':
           if ((gDALGlobals.io.keyMask & KeyPadLeft) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_padleft;
             processed = true;
           }
           break;

      case VK_RIGHT:
      case '6':
           if ((gDALGlobals.io.keyMask & KeyPadRight) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_padright;
             processed = true;
           }
           break;

      case VK_ESCAPE:
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_unknown;

             _SysEnqueueQuitEvent();
             processed = true;
           }
           break;

      case '1':
           if ((gDALGlobals.io.keyMask & KeyOption1) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_option1;
             processed = true;
           }
           break;

      case '3':
      case '0':
           if ((gDALGlobals.io.keyMask & KeyOption2) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_option2;
             processed = true;
           }
           break;

      case VK_RETURN:
#ifndef _WIN32_WCE
      case 0x86:    // VK_F23 - both keys get broadcast, VK_RETURN = guarenteed
#endif
      case '5':
           if ((gDALGlobals.io.keyMask & KeyPadSelect) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_padselect;
             processed = true;
           }
           break;

#ifndef _WIN32_WCE
      case VK_PRIOR:
           if ((gDALGlobals.io.keyMask & KeyScrollUp) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_scrollup;
             processed = true;
           }
           break;

      case VK_NEXT:
           if ((gDALGlobals.io.keyMask & KeyScrollDown) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_scrolldown;
             processed = true;
           }
           break;
#endif

#ifdef _WIN32_WCE
      case 0x77:
#else
      case VK_F5:
#endif
           if ((gDALGlobals.io.keyMask & KeyPause) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_pause;
             processed = true;
           }
           break;

      default:
           if ((key >= 0x20) && (key < MAX_FONT_CHAR))
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = key;
             processed = true;
           }
           break;
    }
  }

  return processed;
}

static boolean
_PenHandleEvent(event *_e)
{
  coord   penX, penY;
  boolean penDown;

  _PenState(&penX, &penY, &penDown);
  if (penDown)
  {
    _e->eType   = _penEvent;
    _e->penDown = penDown;
    _e->screenX = _e->data.pen.x = penX;
    _e->screenY = _e->data.pen.y = penY;
  }

  // only valid if within range of drawable area :)
  return ((penDown) &&
          (_e->screenX < (coord)gDALGlobals.gfx.width) &&
          (_e->screenY < (coord)gDALGlobals.gfx.height));
}

boolean
_PenDisplayToWindow(coord *penX, coord *penY)
{
  // only valid if within range of drawable area :)
  return ((*penX > 0) &&
          (*penY > 0) &&
          (*penX < (coord)gDALGlobals.gfx.width) &&
          (*penY < (coord)gDALGlobals.gfx.height));
}

/********************************* EOF ***********************************/
