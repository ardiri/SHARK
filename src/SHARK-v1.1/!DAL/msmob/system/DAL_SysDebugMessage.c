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
  MSmob_PlatformInfo *platform;
  uint16             *buf;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (MSmob_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // do we need to show a dialog message?
    if (dialog)
    {
      buf = DAL_SysCreateUnicodeString(msg);
      MessageBox(platform->windows.LCDWnd, (const uint16 *)buf, TEXT("SHARK: DEBUG MESSAGE"), MB_OK);
      _MemPtrFree(buf);
    }
  }
}

/********************************* EOF ***********************************/
