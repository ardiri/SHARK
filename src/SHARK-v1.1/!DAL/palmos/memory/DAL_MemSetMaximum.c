/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_MemSetMaximum.c
 */

#include "../DAL-prv.h"

void
DAL_MemSetMaximum(uint16 max)
{
  MemChunkInfo *chunk;
  GLOBALS_ACCESS;

  // we will NOT allow the developer to increase the memory info buffer once set
  if ((g->memory.max_set) && (max < MAX_MEM)) return;

  chunk = (MemChunkInfo *)SYSTEM_MemPtrNew(sizeof(MemChunkInfo) * (max+1));
  if (chunk != NULL)
  {
    _MemSet(chunk, (uint16)(sizeof(MemChunkInfo) * (max+1)), 0);                   // clear
    _MemMove(chunk, g->memory.info, (uint16)(sizeof(MemChunkInfo) * (MAX_MEM+1))); // copy all existing

    SYSTEM_MemPtrFree(g->memory.info);
    g->memory.info    = chunk;  // new buffer in use now
    g->memory.max     = max;
    g->memory.max_set = true;
  }
}

/********************************* EOF ***********************************/
