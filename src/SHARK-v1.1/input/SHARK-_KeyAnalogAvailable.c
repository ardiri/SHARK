/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_PenAvailable.c
 */

#include "../SHARK-prv.h"

boolean
_KeyAnalogAvailable()
{
  boolean result;
  GLOBALS_ACCESS;

  // default return value
  result = g->device.input.analog;

  return result;
}

/********************************* EOF ***********************************/
