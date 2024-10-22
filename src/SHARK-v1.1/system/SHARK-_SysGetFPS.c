/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SysGetFPS.c
 */

#include "../SHARK-prv.h"

uint32
_SysGetFPS()
{
  uint32 result;
  GLOBALS_ACCESS;

  // default return value
  result = g->system.fps;

  return result;
}

/********************************* EOF ***********************************/
