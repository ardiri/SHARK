/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SysGetGlobals.c
 */

#include "../SHARK-prv.h"

void *
_SysGetGlobals()
{
  void    *result;
  GLOBALS_ACCESS;

  // default return value
  result = g->system.app_globals;

  return result;
}

/********************************* EOF ***********************************/
