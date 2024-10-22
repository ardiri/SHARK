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
  Win32_DeviceInfo *device;
  Win32_MemInfo    *mem;
  boolean           init_ok;
  GLOBALS_ACCESS;

  // LOGGING EVENT
  LOG_ENTRY(">> MemInitialize");

  // assume everything is ok
  init_ok = true;

  // allocate the information block
  g->memory.platform = (void *)SYSTEM_MemPtrNew(sizeof(Win32_MemInfo));
  init_ok = (g->memory.platform != NULL); if (!init_ok) goto MEM_INIT_ABORT;
  _MemSet(g->memory.platform, sizeof(Win32_MemInfo), 0);

  // allocate the memory chunk information blocks
  g->memory.count = 0;
  g->memory.info  = (MemChunkInfo *)SYSTEM_MemPtrNew(sizeof(MemChunkInfo) * (MAX_MEM+1));  // one more (safety)
  init_ok = (g->memory.info != NULL); if (!init_ok) goto MEM_INIT_ABORT;
  _MemSet(g->memory.info, (uint16)(sizeof(MemChunkInfo) * (MAX_MEM+1)), 0);

  g->memory.max     = MAX_MEM;
  g->memory.max_set = false;

  // access device information block
  device  = (Win32_DeviceInfo *)g->device.platform;
  if (device != NULL)
  {
    mem = (Win32_MemInfo *)g->memory.platform;
    mem->heap.total          = device->memory.heap_size;
    mem->heap.warn           = mem->heap.total / 10;     // 10% heap warning limit
    mem->heap.used           = 0;
    mem->heap.upper_limit    = 0;

    mem->storage.total       = device->memory.storage_size;
    mem->storage.warn        = mem->storage.total / 10;  // 10% storage warning limit
    mem->storage.used        = 0;
    mem->storage.upper_limit = 0;

    mem->memory.max_chunks   = 0;

    mem->warnings            = (device->memory.warnings != false);
  }

  // everything ok?
  goto MEM_INIT_SUCCESS;

MEM_INIT_ABORT:

  // clean up memory usage within memory management system
  if (g->memory.info     != NULL) SYSTEM_MemPtrFree(g->memory.info); 
  if (g->memory.platform != NULL) SYSTEM_MemPtrFree(g->memory.platform); 
  g->memory.info = (MemChunkInfo *)NULL;
  mem = NULL; g->memory.platform = NULL;

MEM_INIT_SUCCESS:

  return init_ok;
}

/********************************* EOF ***********************************/
