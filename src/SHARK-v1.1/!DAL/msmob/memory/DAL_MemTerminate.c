/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_MemTerminate.c
 */

#include "../DAL-prv.h"

void
DAL_MemTerminate()
{
  MemChunkInfo *chunk;
  int           i;
  GLOBALS_ACCESS;

  // memory leak in application, lazy developer: clean up for them
  if (g->memory.count != 0)
  {
    chunk = (MemChunkInfo *)g->memory.info;
    if (chunk != NULL) 
    {
      i = g->memory.max;
      do
      {
        if (chunk->x != NULL) _MemPtrFree(chunk->x);
        chunk++;
      } while (--i);
    }
  }

  // clean up memory
  if (g->memory.info != NULL) SYSTEM_MemPtrFree(g->memory.info); 
  g->memory.info = (MemChunkInfo *)NULL;
}

/********************************* EOF ***********************************/
