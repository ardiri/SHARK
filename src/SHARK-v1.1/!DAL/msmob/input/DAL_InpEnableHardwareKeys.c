/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_InpEnableHardwareKeys.c
 */

#include "../DAL-prv.h"

void 
DAL_InpEnableHardwareKeys(HWND hWnd)
{
#ifdef INP_HANDLE_KEY_UP_DN_MASK
  MSmob_PlatformInfo *platform;
  GLOBALS_ACCESS;
#endif

  // application keys
  UnregisterHotKey(hWnd, VK_APP1);
  UnregisterHotKey(hWnd, VK_APP2);
  UnregisterHotKey(hWnd, VK_APP3);
  UnregisterHotKey(hWnd, VK_APP4);
  UnregisterHotKey(hWnd, VK_APP5);
  UnregisterHotKey(hWnd, VK_APP6);

  // miscellaneous keys
  UnregisterHotKey(hWnd, 0x86);
  UnregisterHotKey(hWnd, VK_TTALK);
  UnregisterHotKey(hWnd, VK_TEND);

#ifdef INP_HANDLE_KEY_UP_DN_MASK
  // access device information block
  platform = (MSmob_PlatformInfo *)g->platform;
  if (platform != NULL)
    platform->input.appKeyState = 0;
#endif
}

/********************************* EOF ***********************************/
