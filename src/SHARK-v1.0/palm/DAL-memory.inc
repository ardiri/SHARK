/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-memory.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 * Platform Implementation Structures
 *
 *************************************************************************/

typedef struct _MemChunkInfo
{
  void     *x;
  MemHandle memHandle;
  uint16    size;
} _MemChunkInfo;

// local functions
static void _MemInitialize() __DAL__;
static void _MemTerminate()  __DAL__;

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

void
_MemInitialize()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif

  // create the cache database for DAL memory
  gDALGlobals->mem.card = 0;
  DmCreateDatabase(gDALGlobals->mem.card,
                   DAL_CACHE, ApplicationGetUniqueID(), 0x74656D70, true); // 'temp' = 0x74656D70
  gDALGlobals->mem.dbID = DmFindDatabase(gDALGlobals->mem.card, DAL_CACHE);

  // open the cache database
  gDALGlobals->mem.dbRef =
    DmOpenDatabase(gDALGlobals->mem.card, gDALGlobals->mem.dbID, dmModeWrite);

  gDALGlobals->mem.info = (void *)MemPtrNew(sizeof(_MemChunkInfo) * MAX_MEM);
  MemSet(gDALGlobals->mem.info, sizeof(_MemChunkInfo) * MAX_MEM, 0);
}

void
_MemTerminate()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  int            i;
  _MemChunkInfo *memP;

  // memory leak in application, lazy developer: clean up for them
  if (gDALGlobals->mem.count != 0)
  {
    for (i=0; i<MAX_MEM; i++)
    {
      memP = &(((_MemChunkInfo *)gDALGlobals->mem.info)[i]);
      if (memP->x != NULL) _MemPtrFree(memP->x);
    }
  }

  MemPtrFree(gDALGlobals->mem.info);
  gDALGlobals->mem.info = NULL;

  // close and delete the cache database
  DmCloseDatabase(gDALGlobals->mem.dbRef);
  DmDeleteDatabase(gDALGlobals->mem.card, gDALGlobals->mem.dbID);
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

void *
_MemPtrNew(uint16 size, boolean storage)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  void          *ptr = NULL;
  int            i, index = 0;
  _MemChunkInfo *memP;
  uint32         romVersion;

  // get the rom version from the device
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
  storage &= (romVersion < palmos5);  // no storage trick on OS5+

  // locate the first available memory 'management' chunk
  for (i=0; i<MAX_MEM; i++)
  {
    memP = &(((_MemChunkInfo *)gDALGlobals->mem.info)[i]);
    if (memP->x == NULL) { index = i; break; }
  }

  // configure memory 'management' chunk
  MemSet(memP, sizeof(_MemChunkInfo), 0);
  memP->size = size;

  // allocate the memory (either in storage heap, or, in dynamic heap)
  if (storage)
  {
    memP->memHandle = DmNewResource(gDALGlobals->mem.dbRef, 0x5F6D656D, index, size); // '_mem' = 0x5F6D656D
    if (memP->memHandle != NULL)
      ptr = (void *)MemHandleLock(memP->memHandle);
  }
  else
    ptr = (void *)MemPtrNew((uint16)size);

  // successful? alright - save it
  if (ptr != NULL)
  {
    memP->x = ptr;
    gDALGlobals->mem.count++;
  }

  return ptr;
}

void
_MemSet(void *ptr, uint16 size, uint8 value)
{
  MemSet(ptr, size, value);
}

void
_MemMove(void *ptrDst, void *ptrSrc, uint16 count)
{
  MemMove(ptrDst, ptrSrc, count);
}

uint16
_MemPtrSize(void *ptr)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  int           i;
  uint16        size = 0;
  _MemChunkInfo *memP;

  // locate the memory 'management' chunk
  for (i=0; i<gDALGlobals->mem.count; i++)
  {
    memP = &(((_MemChunkInfo *)gDALGlobals->mem.info)[i]);
    if (memP->x == ptr) { size = memP->size; break; }
  }

  return size;
}

void
_MemPtrFree(void *ptr)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  int            i;
  _MemChunkInfo *memP;

  // locate the memory 'management' chunk
  for (i=0; i<MAX_MEM; i++)
  {
    memP = &(((_MemChunkInfo *)gDALGlobals->mem.info)[i]);

    if (memP->x == ptr)
    {
      // memory was allocated in storage heap
      if (memP->memHandle != NULL)
      {
      	uint16 index;

        MemHandleUnlock(memP->memHandle);
        DmReleaseResource(memP->memHandle);
      	index = DmFindResource(gDALGlobals->mem.dbRef, 0, 0, memP->memHandle);
        DmRemoveResource(gDALGlobals->mem.dbRef, index);
      }
      else
        MemPtrFree(ptr);

      // mark memory 'managment' chunk as no longer valid
      MemSet(memP, sizeof(_MemChunkInfo), 0);
      gDALGlobals->mem.count--; break;
    }
  }
}

void
_MemSemaphore(boolean active)
{
  uint32 romVersion;

  // get the rom version from the device
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

  // this command cannot be guarenteed!
  if (romVersion < palmos5)
  {
    if (active) MemSemaphoreReserve(true);
    else        MemSemaphoreRelease(true);
  }
}

/********************************* EOF ***********************************/

