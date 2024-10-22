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
  Win32_PlatformInfo *platform;
  UINT                timerRes;
  TIMECAPS            tc;
  uint32              ticksPerFrame;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    if (fps == 0)
      timeKillEvent(platform->system.timerID);
    else
    {
      ticksPerFrame = (DEFAULT_TICKS_PER_SEC * 10) / fps;
  
      if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) == TIMERR_NOERROR)
      {
        timerRes = MIN(MAX(tc.wPeriodMin, ticksPerFrame), tc.wPeriodMax);
        timeBeginPeriod(timerRes);
        platform->system.timerID = 
          timeSetEvent(ticksPerFrame, timerRes, DAL_TimerProc, (DWORD)NULL, TIME_PERIODIC);
      }
    }
  }
}

/********************************* EOF ***********************************/
