/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxDisplayInfo.c
 */

#include "../SHARK-prv.h"

void
_GfxDisplayInfo(coord *width, coord *height)
{
  GLOBALS_ACCESS;

  if (width  != NULL) *width  = g->device.display.width;
  if (height != NULL) *height = g->device.display.height;
}

/********************************* EOF ***********************************/
