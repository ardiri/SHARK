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
  PALM_PlatformInfo *platform;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    g->system.fps     = fps;
    g->system.fps_set = (g->system.fps != 0);

    if (fps == 0)
      platform->system.evtTimeOut     = evtWaitForever;            // no animations
    else
    {
      platform->system.evtTimeOut     = 1;
      platform->system.ticksPerSecond = SysTicksPerSecond();
      platform->system.ticksPerFrame  = (platform->system.ticksPerSecond * 10) / fps;
      platform->system.ticksPerFrameE = (platform->system.ticksPerSecond % (fps / 10));
      if (platform->system.ticksPerFrameE != 0)
        platform->system.ticksPerFrameE = ((fps / 10) - platform->system.ticksPerFrameE);
    }
  }
}

/********************************* EOF ***********************************/
