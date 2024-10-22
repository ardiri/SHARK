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
#ifdef _M_ARM
  PALM_PlatformInfo *platform;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    if (platform->gfx.bmp != NULL)   BmpDelete(platform->gfx.bmp);
    if (platform->gfx.bmpV3 != NULL) BmpDelete((BitmapType *)platform->gfx.bmpV3);
    if (platform->gfx.win != NULL)   WinDeleteWindow(platform->gfx.win, false);
    platform->gfx.bmp   = NULL;
    platform->gfx.bmpV3 = NULL;
    platform->gfx.win   = NULL;
  }
#endif
}

/********************************* EOF ***********************************/
