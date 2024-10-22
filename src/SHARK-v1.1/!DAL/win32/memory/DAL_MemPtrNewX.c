/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_MemPtrNewX.c
 */

#include "../DAL-prv.h"

void *
DAL_MemPtrNewX(uint32 size, boolean storage)
{
  Win32_MemInfo *mem;
  MemChunkInfo  *chunk;
  void          *p;
  int            i;
  boolean        success;
  GLOBALS_ACCESS;

  // default return value
  p = NULL;

  // can we allocate another chunk?
  if (g->memory.count < (g->memory.max-1))
  {
    chunk = (MemChunkInfo *)g->memory.info;

    // locate the first total memory 'management' chunk
    i = g->memory.max;
    do
    {
      if (chunk->x == NULL) goto NEW_SEARCH; 
      chunk++; // look at the next chunk?
    } while (--i);

    // didn't find a memory slot, lets bail!
    goto FAIL_SEARCH;

NEW_SEARCH:

    // allocate memory
    p = (void *)SYSTEM_MemPtrNew(size);

    // configure memory 'management' chunk
    if (p != NULL)
    {
      _MemSet(chunk, sizeof(MemChunkInfo), 0);
      chunk->size    = size;
      chunk->storage = storage;
      chunk->x       = p;
      g->memory.count++;
    }

    // heap validation
    success = true;
    mem     = (Win32_MemInfo *)g->memory.platform;
    if (mem != NULL)
    {  
      if (chunk->storage) 
      {
        // are we breaching the storage warning limits?
        if (mem->storage.warn > ((mem->storage.total - mem->storage.used) - size))
          _SysDebugMessage("MemPtrNew()\n\nWARNING: < 10% storage heap memory available", mem->warnings);

        // do we have enough memory for this operation?
        if ((mem->storage.total - mem->storage.used) >= size)
        {    
          mem->storage.used += size;
          if (mem->storage.upper_limit < mem->storage.used) mem->storage.upper_limit = mem->storage.used;
        }
        else
        {
//        success = false;
          _SysDebugMessage("MemPtrNew()\n\nERROR: insufficient storage heap memory available", mem->warnings);

          // remember:: lets try dynamic heap if it fails (simulate)
          chunk->storage = false;
          goto MEM_DYNAMIC_HEAP;
        }
      }
      else
      {
MEM_DYNAMIC_HEAP:

        // are we breaching the heap warning limits?
        if (mem->heap.warn > ((mem->heap.total - mem->heap.used) - size))
          _SysDebugMessage("MemPtrNew()\n\nWARNING: < 10% dynamic heap memory available", mem->warnings);
  
        // do we have enough memory for this operation?
        if ((mem->heap.total - mem->heap.used) >= size)
        {    
          mem->heap.used    += size;
          if (mem->heap.upper_limit < mem->heap.used) mem->heap.upper_limit = mem->heap.used;
        }
        else
        {
          success = false;
          _SysDebugMessage("MemPtrNew()\n\nERROR: insufficient dynamic heap memory available", mem->warnings);
        }
      }

#ifdef MEM_FORCE_NULL
      if (!success)
      {
         SYSTEM_MemPtrFree(p);
         p = (void *)NULL;
  
         // we gotta mark it as free now as well
         _MemSet(chunk, sizeof(MemChunkInfo), 0);
         g->memory.count--;
      }
#endif

      // record upper limit
      if (mem->memory.max_chunks < g->memory.count)
        mem->memory.max_chunks = g->memory.count;
    }

    // LOGGING EVENT
    {
      uint8 str_log[256];
  
      mem = (Win32_MemInfo *)g->memory.platform;
      if (mem != NULL)
      {
        sprintf(str_log, "[%4d]\tMemPtrNew\tsize = %d,\tmemory use = %ld\t%s%s%s", g->memory.count, size, (uint32)(mem->storage.used + mem->heap.used), (storage) ? "storage" : "heap", (storage == chunk->storage) ? "\tSAME" : "\tDIFF", (success) ? "" : "\t[failed]");
        LOG_ENTRY(str_log);
      }
    }
  }
  else
  {

FAIL_SEARCH:

    mem     = (Win32_MemInfo *)g->memory.platform;
    if (mem != NULL)
    {  
      _SysDebugMessage("MemPtrNew()\n\nERROR: unable to find free memory management block", mem->warnings);
    }
  }

  return p;
}

/********************************* EOF ***********************************/
