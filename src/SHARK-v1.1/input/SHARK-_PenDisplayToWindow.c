/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_PenDisplayToWindow.c
 */

#include "../SHARK-prv.h"

boolean
_PenDisplayToWindow(coord *penX, coord *penY)
{
  boolean result;
  GLOBALS_ACCESS;

  // default return value
  result = false;

  // access device information block
  if (_PenAvailable())
  {
    result = ((*penX > 0) &&
              (*penY > 0) &&
              (*penX < (coord)g->device.display.width) &&
              (*penY < (coord)g->device.display.height));
  }

  return result;
}

/********************************* EOF ***********************************/
