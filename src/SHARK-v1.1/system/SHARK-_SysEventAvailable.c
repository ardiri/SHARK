/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SysEventAvailable.c
 */

#include "../SHARK-prv.h"

boolean
_SysEventAvailable()
{
  boolean result;
  GLOBALS_ACCESS;

  // default return value
  result = (g->system.events.index != g->system.events.max);

  return result;
}

/********************************* EOF ***********************************/
