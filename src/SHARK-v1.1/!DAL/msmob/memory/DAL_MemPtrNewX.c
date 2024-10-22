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
  MemChunkInfo *chunk;
  void         *p;
  int           i;
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
  }
  else
  {

FAIL_SEARCH:

    ;
  }

  return p;
}

/********************************* EOF ***********************************/
