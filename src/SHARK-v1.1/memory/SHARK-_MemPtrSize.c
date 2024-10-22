/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_MemPtrSize.c
 */

#include "../SHARK-prv.h"

uint16
_MemPtrSize(void *p)
{
  MemChunkInfo *chunk;
  int           i;
  uint32        size;
  GLOBALS_ACCESS;

  // default return value
  size = 0;

  // locate the memory 'management' chunk
  if (g->memory.count != 0)
  {
    chunk = (MemChunkInfo *)g->memory.info;
    if (chunk != NULL)
    {
      i = MAX_MEM;
      do
      {
        if (chunk->x == p) { size = chunk->size; break; }
      } while (--i);
    }
  }

  // we dont want the developer playing with chunks too large (out of range)
  if (size > 64000)
    _SysDebugMessage("MemPtrSize()\n\nWARNING: > 64000 byte memory buffer processed", true);

  return (uint16)size;
}

/********************************* EOF ***********************************/
