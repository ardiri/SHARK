/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_LCDGetProperties.c
 */

#include "../DAL-prv.h"

void
DAL_LCDGetProperties(void **bits, coord *width, coord *height, uint8 *depth)
{
  Win32_PlatformInfo *platform;
  GLOBALS_ACCESS;

  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    if (bits   != NULL) *bits   = (g->device.display.bits == NULL) 
      ? (void *)platform->display.bits   // the base display window
      : (void *)g->device.display.bits;  // the non 16bpp display window
    if (width  != NULL) *width  = g->device.display.width;
    if (height != NULL) *height = g->device.display.height;
    if (depth  != NULL) *depth  = g->device.display.depth;
  }
}

/********************************* EOF ***********************************/
