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
  MSmob_PlatformInfo *platform;
  GLOBALS_ACCESS;

  // remove screen buffer
  platform = (MSmob_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    if (platform->display.bmp != NULL) DeleteObject(platform->display.bmp);
    if (platform->display.dc  != NULL) DeleteDC(platform->display.dc);
    platform->display.bmp = NULL;
    platform->display.dc  = NULL;
//  this is allocated by the bitmap
//  if (platform->display.bits != NULL) SYSTEM_MemPtrFree(platform->display.bits);
    platform->display.bits = NULL;

#ifdef GAPI_SUPPORT
    // terminate GAPI
    if (platform->gapi.GXCloseDisplay != NULL)
      platform->gapi.GXCloseDisplay();
#endif
  }
}

/********************************* EOF ***********************************/
