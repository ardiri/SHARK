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
  MSmob_PlatformInfo *platform;
  GLOBALS_ACCESS;

  platform = (MSmob_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    if (bits   != NULL) *bits   = (void *)platform->display.bits; // the base display window
    if (width  != NULL) *width  = g->device.display.width;
    if (height != NULL) *height = g->device.display.height;
    if (depth  != NULL) *depth  = 16;
  }
}

/********************************* EOF ***********************************/
