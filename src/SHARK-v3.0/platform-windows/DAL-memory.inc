/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-memory.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define MEM_GLOBALS           g_mem
#define GLOBALS_UNINITIALIZED (MEM_GLOBALS == NULL)
#define GLOBALS_DEFINITION    MemGlobals *g_mem = NULL

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-memory.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
DAL_MemInitialize()
{
  boolean init_ok;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(MEM_GLOBALS, 0, sizeof(MemGlobals));

  return init_ok;
}

static void
DAL_MemPtrNewDynamicHeap(MemChunkInfo *chunk, uint32 size)
{
  // save the basic properties
  chunk -> storage = false; 
  chunk -> size    = size; 

  // allocate the memory block
  chunk -> x = malloc(size);
}

static void
DAL_MemPtrFreeDynamicHeap(MemChunkInfo *chunk)
{
  // free the memory block
  free(chunk -> x);

  // the pointer is no longer valid
  chunk -> x = NULL;
}

static void
DAL_MemPtrNewStorageHeap(MemChunkInfo *chunk, uint32 size)
{
  // save the basic properties
  chunk -> storage = true; 
  chunk -> size    = size; 

  // allocate the memory block
  chunk -> x = malloc(size);
}

static void
DAL_MemPtrFreeStorageHeap(MemChunkInfo *chunk)
{
  // free the memory block
  free(chunk -> x);

  // the pointer is no longer valid
  chunk -> x = NULL;
}

static void
DAL_MemSemaphoreReserve()
{
  // mem semaphores are not required
}

static void
DAL_MemSemaphoreRelease()
{
  // mem semaphores are not required
}

static void 
DAL_MemTerminate()
{
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef MEM_GLOBALS

/********************************* EOF ***********************************/
