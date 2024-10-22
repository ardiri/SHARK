/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)windows/DAL-memory.inc
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
  uint16    size;
} _MemChunkInfo;

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

void
_MemInitialize()
{
  gDALGlobals.mem.info = (void *)malloc(sizeof(_MemChunkInfo) * MAX_MEM);
  _MemSet(gDALGlobals.mem.info, sizeof(_MemChunkInfo) * MAX_MEM, 0);
}

void
_MemTerminate()
{
  int            i;
  _MemChunkInfo *memP;

  // memory leak in application, lazy developer: clean up for them
  if (gDALGlobals.mem.count != 0)
  {
    for (i=0; i<MAX_MEM; i++)
    {
      memP = &(((_MemChunkInfo *)gDALGlobals.mem.info)[i]);
      if (memP->x != NULL) _MemPtrFree(memP->x);
    }
  }

  free(gDALGlobals.mem.info);
  gDALGlobals.mem.info = NULL;
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

void *
_MemPtrNew(uint16 size, boolean storage)
{
  void          *ptr = NULL;
  int            i, index = 0;
  _MemChunkInfo *memP;

  // locate the first available memory 'management' chunk
  for (i=0; i<MAX_MEM; i++)
  {
    memP = &(((_MemChunkInfo *)gDALGlobals.mem.info)[i]);
    if (memP->x == NULL) { index = i; break; }
  }

  // configure memory 'management' chunk
  memset(memP, 0, sizeof(_MemChunkInfo));
  memP->size = size;

  // allocate the memory (either in storage heap, or, in dynamic heap)
  ptr = (void *)malloc(size);

  // successful? alright - save it
  if (ptr != NULL)
  {
    memP->x = ptr;
    gDALGlobals.mem.count++;
  }

  return ptr;
}

void
_MemSet(void *ptr, uint16 size, uint8 value)
{
  memset((void *)ptr, (int)value, (size_t)size);
}

void
_MemMove(void *ptrDst, void *ptrSrc, uint16 count)
{
  memmove((void *)ptrDst, (void *)ptrSrc, (size_t)count);
}

uint16
_MemPtrSize(void *ptr)
{
  int           i;
  uint16        size = 0;
  _MemChunkInfo *memP;

  // locate the memory 'management' chunk
  for (i=0; i<gDALGlobals.mem.count; i++)
  {
    memP = &(((_MemChunkInfo *)gDALGlobals.mem.info)[i]);
    if (memP->x == ptr) { size = memP->size; break; }
  }

  return size;
}

void
_MemPtrFree(void *ptr)
{
  int            i;
  _MemChunkInfo *memP;

  // locate the memory 'management' chunk
  for (i=0; i<MAX_MEM; i++)
  {
    memP = &(((_MemChunkInfo *)gDALGlobals.mem.info)[i]);

    if (memP->x == ptr)
    {
      // memory was allocated in storage heap
      free((void *)ptr);

      // mark memory 'managment' chunk as no longer valid
      memset(memP, 0, sizeof(_MemChunkInfo));
      gDALGlobals.mem.count--; break;
    }
  }
}

void
_MemSemaphore(boolean active)
{
}

/********************************* EOF ***********************************/