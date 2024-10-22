/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_InpDisableHardwareKeys.c
 */

#include "../DAL-prv.h"

void 
DAL_InpDisableHardwareKeys(HWND hWnd)
{
#ifdef INP_HANDLE_KEY_UP_DN_MASK
  MSmob_PlatformInfo *platform;
#endif
  typedef BOOL (__stdcall *UnregisterFunc1Proc)( UINT, UINT);
  HINSTANCE hCoreDll;
  UINT      modifiers;
  UnregisterFunc1Proc procUnregisterFunc;
#ifdef INP_HANDLE_KEY_UP_DN_MASK
  GLOBALS_ACCESS;
#endif

  hCoreDll = LoadLibrary(_T("coredll.dll"));
  procUnregisterFunc = (UnregisterFunc1Proc)GetProcAddress(hCoreDll, _T("UnregisterFunc1"));

#ifdef INP_HANDLE_KEY_UP_DN_MASK
  modifiers = MOD_WIN | MOD_KEYUP;
#else
  modifiers = MOD_WIN;
#endif

  // application keys
  procUnregisterFunc(MOD_WIN, VK_APP1);    RegisterHotKey(hWnd, VK_APP1,  modifiers, VK_APP1);
  procUnregisterFunc(MOD_WIN, VK_APP2);    RegisterHotKey(hWnd, VK_APP2,  modifiers, VK_APP2);
  procUnregisterFunc(MOD_WIN, VK_APP3);    RegisterHotKey(hWnd, VK_APP3,  modifiers, VK_APP3);
  procUnregisterFunc(MOD_WIN, VK_APP4);    RegisterHotKey(hWnd, VK_APP4,  modifiers, VK_APP4);
  procUnregisterFunc(MOD_WIN, VK_APP5);    RegisterHotKey(hWnd, VK_APP5,  modifiers, VK_APP5);
  procUnregisterFunc(MOD_WIN, VK_APP6);    RegisterHotKey(hWnd, VK_APP6,  modifiers, VK_APP6);

  // miscellaneous keys
  procUnregisterFunc(MOD_WIN, 0x86);       RegisterHotKey(hWnd, 0x86,     modifiers, 0x86);
  procUnregisterFunc(MOD_WIN, VK_TTALK);   RegisterHotKey(hWnd, VK_TTALK, modifiers, VK_TTALK);
  procUnregisterFunc(MOD_WIN, VK_TEND);    RegisterHotKey(hWnd, VK_TEND,  modifiers, VK_TEND);

#ifdef INP_HANDLE_KEY_UP_DN_MASK
  // access device information block
  platform = (MSmob_PlatformInfo *)g->platform;
  if (platform != NULL)
    platform->input.appKeyState = 0;
#endif

  FreeLibrary(hCoreDll);
}

/********************************* EOF ***********************************/
