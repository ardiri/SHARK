/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SysDebugMessage.c
 */

#include "../DAL-prv.h"

void
DAL_SysDebugMessage(uint8 *msg, boolean dialog)
{
  Win32_PlatformInfo *platform;
  int                 i;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // do we need to show a dialog message?
    if (dialog)
      MessageBox(platform->windows.LCDWnd, (const uint8 *)msg, TEXT("SHARK: DEBUG MESSAGE"), MB_OK);

    // LOGGING EVENT
    else
    {
      uint8 str_log[256];
      sprintf(str_log, ">> SysDebugMessage: %s", msg);

      // we dont want \n's everywhere
      for (i=0; i<_StrLen(str_log); i++)
        if (str_log[i] == '\n') str_log[i] = ':';

      LOG_ENTRY(str_log);
    }
  }
}

/********************************* EOF ***********************************/
