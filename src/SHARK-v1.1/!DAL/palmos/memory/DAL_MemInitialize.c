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
  PALM_PlatformInfo *platform;
  boolean            init_ok;
  GLOBALS_ACCESS;

  // assume everything is ok
  init_ok = true;

  // get access to the platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  init_ok = (platform != NULL); if (!init_ok) goto MEM_INIT_ABORT;

  // create and open the cache database for DAL memory
  DmCreateDatabase(0, "SHARK_CACHE", ApplicationGetUniqueID(), 0x74656D70, true); // 'temp' = 0x74656D70
  platform->memory.dbID = DmFindDatabase(0, "SHARK_CACHE");
  init_ok = (platform->memory.dbID != 0); if (!init_ok) goto MEM_INIT_ABORT;
  platform->memory.dbRef = DmOpenDatabase(0, platform->memory.dbID, dmModeWrite);
  init_ok = (platform->memory.dbRef != NULL); if (!init_ok) goto MEM_INIT_ABORT;

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

  // close database
  if (platform->memory.dbRef != NULL) DmCloseDatabase(platform->memory.dbRef);

  // clean up memory usage within memory management system
  if (g->memory.info != NULL) SYSTEM_MemPtrFree(g->memory.info); 
  g->memory.info = (MemChunkInfo *)NULL;

MEM_INIT_SUCCESS:

  return init_ok;
}

/********************************* EOF ***********************************/
