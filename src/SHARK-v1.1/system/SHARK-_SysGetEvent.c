/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SysGetEvent.c
 */

#include "../SHARK-prv.h"

void
_SysGetEvent(event *e)
{
  GLOBALS_ACCESS;

  // get event from queue at current index
  _MemMove(e, &g->system.events.queue[g->system.events.index], sizeof(event));

  // move index along, or wrap around if needed
  g->system.events.index++;
  if (g->system.events.index >= QUEUE_SIZE) g->system.events.index = 0;
}

/********************************* EOF ***********************************/
