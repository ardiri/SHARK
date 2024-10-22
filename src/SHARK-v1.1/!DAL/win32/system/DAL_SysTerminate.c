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
  Win32_PlatformInfo *platform;
  GLOBALS_ACCESS;

  // LOGGING EVENT
  LOG_ENTRY(">> SysTerminate");

  // clean up memory usage within memory management system
  if (g->system.events.queue != NULL) _MemPtrFree(g->system.events.queue); 
  g->system.events.queue = (event *)NULL;

  // terminate the high resolution timer
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
    timeKillEvent(platform->system.timerID);
}

/********************************* EOF ***********************************/
