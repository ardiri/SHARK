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
  MemChunkInfo *chunk;
  int           i;
  boolean       found;
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
  
      chunk->size    = 0;
      chunk->storage = false;
      chunk->x       = NULL;
      g->memory.count--;
    }
  }
}

/********************************* EOF ***********************************/
