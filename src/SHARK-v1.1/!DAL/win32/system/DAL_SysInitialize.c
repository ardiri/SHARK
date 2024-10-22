/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SysInitialize.c
 */

#include "../DAL-prv.h"

boolean
DAL_SysInitialize()
{
  boolean init_ok;
  GLOBALS_ACCESS;

  // LOGGING EVENT
  LOG_ENTRY(">> SysInitialize");

  // assume everythign is ok
  init_ok = true;

  // allocate the memory chunk we need for event handling
  g->system.events.max   = 0;
  g->system.events.index = 0;
  g->system.events.queue = (event *)_MemPtrNew(QUEUE_SIZE * sizeof(event), false);
  init_ok = (g->system.events.queue != NULL); if (!init_ok) goto SYS_INIT_ABORT;
  _MemSet(g->system.events.queue, QUEUE_SIZE * sizeof(event), 0);

  g->system.fps_set = false;
  g->system.fps     = 0;
  _SysFlushEventQueue();
  _SysRandom(_SysGetTime());

  // everything ok?
  goto SYS_INIT_SUCCESS;

SYS_INIT_ABORT:

  // clean up memory usage within memory management system
  if (g->system.events.queue != NULL) _MemPtrFree(g->system.events.queue); 
  g->system.events.queue = (event *)NULL;

SYS_INIT_SUCCESS:

  return init_ok;
}

/********************************* EOF ***********************************/
