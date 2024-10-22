/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SysSetGlobals.c
 */

#include "../SHARK-prv.h"

void
_SysSetGlobals(void *gbls)
{
  GLOBALS_ACCESS;

  g->system.app_globals = gbls;
}

/********************************* EOF ***********************************/
