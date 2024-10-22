/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SysTerminate.c
 */

#include "../DAL-prv.h"

void
DAL_SysTerminate()
{
  GLOBALS_ACCESS;

  // clean up memory usage within memory management system
  if (g->system.events.queue != NULL) _MemPtrFree(g->system.events.queue); 
  g->system.events.queue = (event *)NULL;
}

/********************************* EOF ***********************************/
