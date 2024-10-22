/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_KeyGetMask.c
 */

#include "../SHARK-prv.h"

uint32
_KeyGetMask()
{
  uint32 result;
  GLOBALS_ACCESS;

  // default return value
  result = g->input.keyMask;

  return result;
}

/********************************* EOF ***********************************/
