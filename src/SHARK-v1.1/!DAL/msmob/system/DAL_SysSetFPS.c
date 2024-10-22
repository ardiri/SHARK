/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SysSetFPS.c
 */

#include "../DAL-prv.h"

void
DAL_SysSetFPS(uint32 fps)
{
  MSmob_PlatformInfo *platform;
  uint32              ticksPerFrame;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (MSmob_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    if (fps == 0)
      KillTimer(platform->windows.LCDWnd, platform->system.timerID);
    else
    {
      ticksPerFrame = (DEFAULT_TICKS_PER_SEC * 10) / fps;
      platform->system.timerID = SetTimer(platform->windows.LCDWnd, DEFAULT_TIMER_ID, ticksPerFrame, NULL);
    }
  }
}

/********************************* EOF ***********************************/
