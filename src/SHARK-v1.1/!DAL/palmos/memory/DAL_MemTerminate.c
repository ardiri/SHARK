/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_MemTerminate.c
 */

#include "../DAL-prv.h"

void
DAL_MemTerminate()
{
  PALM_PlatformInfo *platform;
  MemChunkInfo      *chunk;
  int               i;
  GLOBALS_ACCESS;

  // memory leak in application, lazy developer: clean up for them
  if (g->memory.count != 0)
  {
    chunk = (MemChunkInfo *)g->memory.info;
    if (chunk != NULL) 
    {
      i = g->memory.max;
      do
      {
        if (chunk->x != NULL) _MemPtrFree(chunk->x);
        chunk++;
      } while (--i);
    }
  }

  // clean up memory
  if (g->memory.info != NULL) SYSTEM_MemPtrFree(g->memory.info); 
  g->memory.info = (MemChunkInfo *)NULL;

  // get access to the platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL) 
  { 
    // close and delete the cache database
    if (platform->memory.dbRef != NULL)
    {
      DmCloseDatabase(platform->memory.dbRef);
      DmDeleteDatabase(0, platform->memory.dbID);
    }
    platform->memory.dbRef = NULL;
    platform->memory.dbID  = 0;
  }
}

/********************************* EOF ***********************************/
