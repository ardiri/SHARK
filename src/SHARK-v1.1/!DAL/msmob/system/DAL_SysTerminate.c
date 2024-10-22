/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SysTerminate.c
 */

#include "../DAL-prv.h"

void
DAL_SysTerminate()
{
  MSmob_PlatformInfo *platform;
  GLOBALS_ACCESS;

  // clean up memory usage within memory management system
  if (g->system.events.queue != NULL) _MemPtrFree(g->system.events.queue); 
  g->system.events.queue = (event *)NULL;

  // terminate the high resolution timer
  platform = (MSmob_PlatformInfo *)g->platform;
  if (platform != NULL)
    KillTimer(platform->windows.LCDWnd, platform->system.timerID);
}

/********************************* EOF ***********************************/
