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
_PenAvailable()
{
  boolean result;
  GLOBALS_ACCESS;

  // default return value
  result = g->device.input.stylus;

  return result;
}

/********************************* EOF ***********************************/
