/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_MemPtrFree.c
 */

#include "../DAL-prv.h"

void
DAL_MemPtrFree(void *p)
{
  Win32_MemInfo *mem;
  MemChunkInfo  *chunk;
  int            i;
  boolean        found;
  GLOBALS_ACCESS;

  // entry level condition (no NULL pointers)
  if (p == NULL) return;

  // locate the memory 'management' chunk
  if (g->memory.count != 0)
  {
    chunk = (MemChunkInfo *)g->memory.info;

    found = false;
    i     = g->memory.max;
    do
    {
      if (chunk->x == p) { found = true; goto FREE_SEARCH; } 
      chunk++; // look at the next chunk?
    } while (--i);

FREE_SEARCH:

    // did we find the pointer in question?
    if (found)
    {
      SYSTEM_MemPtrFree(p);
  
      // heap/storage validation
      mem = (Win32_MemInfo *)g->memory.platform;
      if (mem != NULL)  
      {
        if (chunk->storage) 
          mem->storage.used -= chunk->size;
        else
          mem->heap.used    -= chunk->size;
      }  

      // LOGGING EVENT
      {
        uint8 str_log[256];
    
        mem = (Win32_MemInfo *)g->memory.platform;
        if (mem != NULL)
        {
          sprintf(str_log, "[%4d]\tMemPtrFree\tsize = %d,\tmemory use = %ld", (g->memory.count-1), chunk->size, (uint32)(mem->storage.used + mem->heap.used));
          LOG_ENTRY(str_log);
        }
      }

      chunk->size    = 0;
      chunk->storage = false;
      chunk->x       = NULL;
      g->memory.count--;
    }
    else
      _SysDebugMessage("MemPtrFree()\n\nERROR: unable to find memory management block", true);
  }
}

/********************************* EOF ***********************************/
