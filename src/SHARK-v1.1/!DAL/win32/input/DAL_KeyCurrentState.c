/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_KeyCurrentState.c
 */

#include "../DAL-prv.h"

uint32
DAL_KeyCurrentState()
{
  Win32_PlatformInfo *platform;
  uint32              result;
  MSG                 msg;
  GLOBALS_ACCESS;

  // default return value
  result = 0;

  // access device information block
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // standard d-pad key simulation
    if (GetAsyncKeyState(VK_LEFT))      result |= KeyPadLeft;
    if (GetAsyncKeyState(VK_RIGHT))     result |= KeyPadRight;
    if (GetAsyncKeyState(VK_UP))        result |= KeyPadUp;
    if (GetAsyncKeyState(VK_DOWN))      result |= KeyPadDown;
    if (GetAsyncKeyState(VK_RETURN))    result |= KeyPadSelect;
    if (GetAsyncKeyState(VK_CONTROL))   result |= KeyAction;
  
    // numerical keypad key values
    if (GetAsyncKeyState('1'))          result |= KeyOption1;
    if (GetAsyncKeyState('2'))          result |= KeyPadUp;
    if (GetAsyncKeyState('3'))          result |= KeyOption2;
    if (GetAsyncKeyState('4'))          result |= KeyPadLeft;
    if (GetAsyncKeyState('5'))          result |= KeyPadSelect;
    if (GetAsyncKeyState('6'))          result |= KeyPadRight;
    if (GetAsyncKeyState('7'))          result |= KeyAction;
    if (GetAsyncKeyState('8'))          result |= KeyPadDown;
    if (GetAsyncKeyState('9'))          result |= KeyAction;
    if (GetAsyncKeyState('0'))          result |= KeyAction;
    if (GetAsyncKeyState('*'))          result |= KeyPause;

    // other action keys
    if (GetAsyncKeyState(VK_F1))        result |= KeyAction;
    if (GetAsyncKeyState(VK_F2))        result |= KeyOption1;
    if (GetAsyncKeyState(VK_F3))        result |= KeyOption2;
    if (GetAsyncKeyState(VK_F4))        result |= KeyPause;
  
    // flush the key queue
    PeekMessage(&msg, platform->windows.LCDWnd, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE);
  }

  return result;
}

/********************************* EOF ***********************************/
