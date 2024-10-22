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
  GLOBALS_ACCESS;

  if (bits   != NULL) *bits   = g->device.display.bits;
  if (width  != NULL) *width  = g->device.display.width;
  if (height != NULL) *height = g->device.display.height;
  if (depth  != NULL) *depth  = g->device.display.depth;
}

/********************************* EOF ***********************************/
