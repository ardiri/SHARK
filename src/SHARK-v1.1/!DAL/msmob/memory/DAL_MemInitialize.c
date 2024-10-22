/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_MemInitialize.c
 */

#include "../DAL-prv.h"

boolean
DAL_MemInitialize()
{
  boolean init_ok;
  GLOBALS_ACCESS;

  // assume everything is ok
  init_ok = true;

  // allocate the information block (not needed on ppc)
  g->memory.platform = NULL;

  // allocate the memory chunk information blocks
  g->memory.count = 0;
  g->memory.info  = (MemChunkInfo *)SYSTEM_MemPtrNew(sizeof(MemChunkInfo) * (MAX_MEM+1));  // one more (safety)
  init_ok = (g->memory.info != NULL); if (!init_ok) goto MEM_INIT_ABORT;
  _MemSet(g->memory.info, (uint16)(sizeof(MemChunkInfo) * (MAX_MEM+1)), 0);

  g->memory.max     = MAX_MEM;
  g->memory.max_set = false;

  // everything ok?
  goto MEM_INIT_SUCCESS;

MEM_INIT_ABORT:

  // clean up memory usage within memory management system
  if (g->memory.info != NULL) SYSTEM_MemPtrFree(g->memory.info); 
  g->memory.info = (MemChunkInfo *)NULL;

MEM_INIT_SUCCESS:

  return init_ok;
}

/********************************* EOF ***********************************/
