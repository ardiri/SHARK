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
  MSmob_PlatformInfo *platform;
  uint32              result;
  MSG                 msg;
  GLOBALS_ACCESS;

  // default return value
  result = 0;

  // access device information block
  platform = (MSmob_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // standard d-pad key simulation
    if (GetAsyncKeyState(VK_TLEFT))   result |= KeyPadLeft;
    if (GetAsyncKeyState(VK_TRIGHT))  result |= KeyPadRight;
    if (GetAsyncKeyState(VK_TUP))     result |= KeyPadUp;
    if (GetAsyncKeyState(VK_TDOWN))   result |= KeyPadDown;
    if (GetAsyncKeyState(VK_TACTION)) result |= KeyPadSelect;

    // numerical keypad key values
    if (GetAsyncKeyState(VK_T1))      result |= KeyOption1;
    if (GetAsyncKeyState(VK_T2))      result |= KeyPadUp;
    if (GetAsyncKeyState(VK_T3))      result |= KeyOption2;
    if (GetAsyncKeyState(VK_T4))      result |= KeyPadLeft;
    if (GetAsyncKeyState(VK_T5))      result |= KeyPadSelect;
    if (GetAsyncKeyState(VK_T6))      result |= KeyPadRight;
    if (GetAsyncKeyState(VK_T7))      result |= KeyAction;
    if (GetAsyncKeyState(VK_T8))      result |= KeyPadDown;
    if (GetAsyncKeyState(VK_T9))      result |= KeyAction;
    if (GetAsyncKeyState(VK_T0))      result |= KeyAction;
    if (GetAsyncKeyState(VK_TSTAR))   result |= KeyPause;
    if (GetAsyncKeyState(VK_TPOUND))  result |= KeyAction;

    // other action keys
    if (GetAsyncKeyState(0x86))       result |= KeyPadSelect;
#ifndef INP_BUTTONS_AS_ACTIONS
    if (GetAsyncKeyState(VK_APP1))    result |= KeyOption1;
    if (GetAsyncKeyState(VK_APP2))    result |= KeyOption2;
    if (GetAsyncKeyState(VK_APP3))    result |= KeyPause;
    if (GetAsyncKeyState(VK_APP4)  ||
#else
    if (GetAsyncKeyState(VK_APP1)  ||
        GetAsyncKeyState(VK_APP2)  ||
        GetAsyncKeyState(VK_APP3)  ||
        GetAsyncKeyState(VK_APP4)  ||
#endif
        GetAsyncKeyState(VK_APP5)  ||
        GetAsyncKeyState(VK_APP6)  ||
        GetAsyncKeyState(VK_TTALK) ||
        GetAsyncKeyState(VK_TEND))    result |= KeyAction;

#ifdef INP_HANDLE_KEY_UP_DN_MASK
#ifndef INP_BUTTONS_AS_ACTIONS
    if (platform->input.appKeyState & KEY_STATE_APP1)   result |= KeyOption1;  // APP1
    if (platform->input.appKeyState & KEY_STATE_APP2)   result |= KeyOption2;
    if (platform->input.appKeyState & KEY_STATE_APP3)   result |= KeyPause;
#else
    if ((platform->input.appKeyState & KEY_STATE_APP1) ||
        (platform->input.appKeyState & KEY_STATE_APP2) ||
        (platform->input.appKeyState & KEY_STATE_APP3)) result |= KeyAction;
#endif
    if (platform->input.appKeyState & KEY_STATE_APP4)   result |= KeyAction;
    if (platform->input.appKeyState & KEY_STATE_APP5)   result |= KeyAction;
    if (platform->input.appKeyState & KEY_STATE_APP6)   result |= KeyAction;
    if (platform->input.appKeyState & KEY_STATE_TTALK)  result |= KeyAction;
    if (platform->input.appKeyState & KEY_STATE_TEND)   result |= KeyAction;
#endif

    // flush the key queue
    PeekMessage(&msg, platform->windows.LCDWnd, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE);
  }

  return result;
}

/********************************* EOF ***********************************/
