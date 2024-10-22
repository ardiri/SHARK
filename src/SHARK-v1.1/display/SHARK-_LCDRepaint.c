/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_LCDRepaint.c
 */

#include "../SHARK-prv.h"

void
_LCDRepaint()
{
  rectangle rect;
  GLOBALS_ACCESS;

  rect.x      = 0;
  rect.y      = 0;
  rect.width  = (coord)g->device.display.width;
  rect.height = (coord)g->device.display.height;

  _LCDRepaintRegion(&rect);
} 

/********************************* EOF ***********************************/
