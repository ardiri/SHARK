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
  PALM_PlatformInfo *platform;
  MemChunkInfo *chunk;
  int           i;
  boolean       found;
  GLOBALS_ACCESS;

  // entry level condition (no NULL pointers)
  if (p == NULL) return;

  // get access to the platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
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
        if (chunk->storage)
        {
#ifdef MEM_FTR_MEM_STORAGE
          // release feature memory
          FtrPtrFree(0x5F6D775F, (UInt16)(i + 256)); // '_mw_' = 0x5F6D775F
#else
          UInt16    _index;
          MemHandle memHandle;

          // recover the memhandle for this pointer
          memHandle = MemPtrRecoverHandle(p);
      
          // unlock and release resource
          _index = DmFindResource(platform->memory.dbRef, 0, 0, memHandle);
          MemHandleUnlock(memHandle);
          DmReleaseResource(memHandle);

          // remove it from cache database
          if (_index != 0xffff) DmRemoveResource(platform->memory.dbRef, _index);
#endif
        }
        else
          MemChunkFree(p);

        chunk->size    = 0;
        chunk->storage = false;
        chunk->x       = NULL;
        g->memory.count--;
      }
    }
  }
}

/********************************* EOF ***********************************/
