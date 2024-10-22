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
  PALM_PlatformInfo *platform;
  MemChunkInfo      *chunk;
  UInt32             romVersion;
  void              *p;
  int                i;
#ifndef MEM_FTR_MEM_STORAGE
  uint16             _index;
  MemHandle          memHandle;
#endif
  GLOBALS_ACCESS;

  // default return value
  p = NULL;

#ifndef MEM_ALLOW_STORAGE
  // disable all storage memory usage
  storage = false;
#endif

  // get access to the platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
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

      // get the rom version from the device
      FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

      // we cannot use storage heap on OS6 and above - no way to write to it
      storage &= (romVersion < sysMakeROMVersion(6,0,0,sysROMStageDevelopment,0));

      // allocate the memory (either in storage heap, or, in dynamic heap)
      if (storage)
      {  
#ifdef MEM_FTR_MEM_STORAGE
        // remove previous (if it exists, it shouldn't - but, never know)
        FtrPtrFree(0x5F6D775F, (UInt16)(i + 256));

        // allocate new feature memory chunk
        FtrPtrNew(0x5F6D775F,  (UInt16)(i + 256), size, (void **)&p); // '_mw_' = 0x5F6D775F
#else
        // remove previous (if it exists, it shouldn't - but, never know)
        _index = DmFindResource(platform->memory.dbRef, 0x5F6D656D, i, NULL);   // '_mem' = 0x5F6D656D
        if (_index != 0xffff) DmRemoveResource(platform->memory.dbRef, _index);

        // allocate new handle
        memHandle = DmNewResource(platform->memory.dbRef, 0x5F6D656D, i, size); // '_mem' = 0x5F6D656D
        if (memHandle != NULL)
          p = (void *)MemHandleLock(memHandle);
#endif

        // SPECIAL CASE:: storage heap fails? lets try dynamic heap
        if (p == NULL)
        {
          p       = (void *)SYSTEM_MemPtrNew(size);
          storage = false;
        }
      }
      else
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
    }
    else
    {

FAIL_SEARCH:

      ;
    }
  }

  // ERROR HANDLING:: writing to NULL can be FATAL (with MemSemaphore)
  if (p == NULL)
  {
    _SysDebugMessage("MEMORY ALLOCATION ERROR", true);
    SysReset();
  }

  return p;
}

/********************************* EOF ***********************************/
