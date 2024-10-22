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
  Win32_MemInfo *mem;
  MemChunkInfo  *chunk;
  int            i;
  uint32         max_heap, max_storage, max_chunks;
  GLOBALS_ACCESS;

  // LOGGING EVENT
  LOG_ENTRY(">> MemTerminate");

  // memory leak in application, lazy developer: clean up for them
  if (g->memory.count != 0)
  {
    // LOGGING EVENT
    LOG_ENTRY(":: - application MEMORY LEAKS -");

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

    _SysDebugMessage("MemTerminate()\n\nERROR: memory leak", true);
  }

  mem = g->memory.platform;
  if (mem != NULL)
  {
    max_heap    = mem->heap.upper_limit;
    max_storage = mem->storage.upper_limit;
    max_chunks  = mem->memory.max_chunks;
  }

  // clean up memory
  if (g->memory.platform != NULL) SYSTEM_MemPtrFree(g->memory.platform); 
  if (g->memory.info     != NULL) SYSTEM_MemPtrFree(g->memory.info); 
  g->memory.info = (MemChunkInfo *)NULL;
  mem = NULL; g->memory.platform = NULL;

  // LOGGING EVENT (upper limits)
  {
    uint8 str_log[256];

    LOG_ENTRY(":: - memory usage statistics - ");
    sprintf(str_log, "\tmax heap = %ld\tmax storage = %ld\tmax chunk use = %ld", max_heap, max_storage, max_chunks);
    LOG_ENTRY(str_log);
  }
}

/********************************* EOF ***********************************/
