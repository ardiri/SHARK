/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SysEnqueueEvent.c
 */

#include "../SHARK-prv.h"

void
_SysEnqueueEvent(event *e)
{
  GLOBALS_ACCESS;

  // save event in queue at 'max' index
  _MemMove(&g->system.events.queue[g->system.events.max], e, sizeof(event));

  // move index along, or wrap around if needed
  g->system.events.max++;
  if (g->system.events.max >= QUEUE_SIZE) g->system.events.max = 0;
}

/********************************* EOF ***********************************/
