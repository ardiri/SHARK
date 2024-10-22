/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SysRandom.c
 */

#include "../SHARK-prv.h"

uint16
_SysRandom(uint32 seed)
{
  uint16 result;
  GLOBALS_ACCESS;

  // default return value
  result = (uint16)seed;

  // platform independent random number generator
  if (seed != 0) g->system.seed = seed;
  g->system.seed = (0x015A4E35L * g->system.seed) + 1;
  result = (uint16)(g->system.seed >> 16);

  return result;
}

/********************************* EOF ***********************************/
