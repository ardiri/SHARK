/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_LCDTerminate.c
 */

#include "../DAL-prv.h"

void
DAL_LCDTerminate()
{
  Win32_PlatformInfo *platform;
  GLOBALS_ACCESS;

  // LOGGING EVENT
  LOG_ENTRY(">> LCDTerminate");

  // lets free memory for our device window
  if (g->device.display.bits    != NULL) _MemPtrFree(g->device.display.bits);
  if (g->device.display.palette != NULL) _MemPtrFree(g->device.display.palette);
  g->device.display.bits    = NULL;
  g->device.display.palette = NULL;

  // remove screen buffer
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    if (platform->display.bmp != NULL) DeleteObject(platform->display.bmp);
    if (platform->display.dc  != NULL) DeleteDC(platform->display.dc);
    platform->display.bmp = NULL;
    platform->display.dc  = NULL;
  }
}

/********************************* EOF ***********************************/
