/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SysFlushEventQueue.c
 */

#include "../SHARK-prv.h"

void
_SysFlushEventQueue()
{
  GLOBALS_ACCESS;

  g->system.events.index = 0;
  g->system.events.max   = 0;
}

/********************************* EOF ***********************************/
