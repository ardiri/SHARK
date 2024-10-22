/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_TimerProc.c
 */

#include "../DAL-prv.h"

void CALLBACK 
DAL_TimerProc(UINT id, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
  Win32_PlatformInfo *platform;
  GLOBALS_ACCESS;

  // post a WM_TIMER event for the event loop to identify
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
    PostMessage(platform->windows.LCDWnd, WM_TIMER, 0, 0);
}

/********************************* EOF ***********************************/
