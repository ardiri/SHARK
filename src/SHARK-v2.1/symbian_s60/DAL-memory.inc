/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-memory.c
 */

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

#define MEM_DUMP_LINE_SIZE 16         // for memory leak reporting

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct MemChunkInfo
{
  void    *x;
  uint32   storage:1;
  uint32   size:31;
} MemChunkInfo;

typedef struct
{
  MemChunkInfo *info;
  uint16        count;
  uint16        max;

  uint32        heap_total;
  uint32        heap_used;
  uint32        heap_upper_limit;
  uint32        stor_total;
  uint32        stor_used;
  uint32        stor_upper_limit;
  uint32        max_chunks;
} MemGlobalsType;

#define GLOBALS_DEFINE_MEM MemGlobalsType *g_mem;
#define GLOBALS_ACCESS_MEM GLOBALS_ACCESS; \
                           MemGlobalsType *g_mem = (MemGlobalsType *)g->mem;

#define g_mem_info             g_mem->info
#define g_mem_count            g_mem->count
#define g_mem_max              g_mem->max
#define g_mem_heap_total       g_mem->heap_total
#define g_mem_heap_used        g_mem->heap_used
#define g_mem_heap_upper_limit g_mem->heap_upper_limit
#define g_mem_stor_total       g_mem->stor_total
#define g_mem_stor_used        g_mem->stor_used
#define g_mem_stor_upper_limit g_mem->stor_upper_limit
#define g_mem_max_chunks       g_mem->max_chunks

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

// initialize the memory manager
static boolean
_MemInitialize()
{
  boolean init_ok;
  GLOBALS_ACCESS;
  GLOBALS_DEFINE_MEM;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _MemInitialize");
#endif

  // assume everything is ok
  init_ok = true;

  // globals:: allocate appropriate memory
  g_mem = (MemGlobalsType *)malloc(sizeof(MemGlobalsType));
  init_ok = (g_mem != NULL); if (!init_ok) goto MEM_INIT_ABORT;
  memset(g_mem, 0, sizeof(MemGlobalsType));
  g->mem = g_mem;
  
  // allocate the memory chunk information blocks
  g_mem_count = 0;
  g_mem_max   = MAX(g_config.memory.count, 64);  // 64 minimum
  g_mem_info  = (MemChunkInfo *)malloc(sizeof(MemChunkInfo) * (g_mem_max + 1));
  init_ok = (g_mem_info != NULL); if (!init_ok) goto MEM_INIT_ABORT;
  memset(g_mem_info, 0, (sizeof(MemChunkInfo) * (g_mem_max+1)));

  // initialize the memory requirements
  g_mem_heap_total       = MAX(g_config.memory.min_heap, 1048576L); // 1MB min
  g_mem_heap_used        = 0;
  g_mem_heap_upper_limit = 0;
  g_mem_stor_total       = g_config.memory.min_storage;
  g_mem_stor_used        = 0;
  g_mem_stor_upper_limit = 0;

  g_mem_max_chunks       = 0;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  {
    char str_log[256];

    sprintf(str_log, "\n\tmaximum chunks:    %d", g_mem_max);
    _LOGCreateEntry(str_log);
    sprintf(str_log, "\tdynamic heap size: %6lu", g_mem_heap_total);
    _LOGCreateEntry(str_log);
    sprintf(str_log, "\tstorage heap size: %6lu\n", g_mem_stor_total);
    _LOGCreateEntry(str_log);
  }
#endif

MEM_INIT_ABORT:

  return init_ok;
}

// allocate a chunk of memory
void *
_MemPtrNew(uint32 size, boolean storage)
{
  MemChunkInfo *chunk;
  void         *p;
  int           i;
  boolean       success;
  GLOBALS_ACCESS_MEM;

  // default return value
  p = NULL;

  // can we allocate another chunk?
  if (g_mem_count < (g_mem_max-1))
  {
    chunk = (MemChunkInfo *)g_mem_info;

    // locate the first total memory 'management' chunk
    i = g_mem_max;
    do
    {
      if (chunk->x == NULL) goto MEM_FOUND_SLOT;
      chunk++; // look at the next chunk?
    } while (--i);

    // didn't find a memory slot, lets bail!
    goto MEM_DONE;

MEM_FOUND_SLOT:

    // allocate memory
    p = (void *)malloc(size);

    // configure memory 'management' chunk
    if (p != NULL)
    {
      memset(chunk, 0, sizeof(MemChunkInfo));
      chunk->size    = size;
      chunk->storage = storage;
      chunk->x       = p;
      g_mem_count++;
    }
    else
    {
#if (LOG_DISABLE == 0)
      _LOGCreateEntry("[ERROR]\t_MemPtrNew()- malloc failed");
#endif
      goto MEM_DONE;
    }

    memset(p, 0, size);

    // heap validation
    success = true;
    if (chunk->storage)
    {
      // do we have enough memory for this operation?
      if ((g_mem_stor_total - g_mem_stor_used) >= size)
      {
        g_mem_stor_used += size;
        if (g_mem_stor_upper_limit < g_mem_stor_used)
          g_mem_stor_upper_limit = g_mem_stor_used;
      }
      else
      {
        // remember:: lets try dynamic heap if it fails (simulate)
        chunk->storage = false;
        goto MEM_DYNAMIC_HEAP;
      }
    }
    else
    {

MEM_DYNAMIC_HEAP:

      // do we have enough memory for this operation?
      if ((g_mem_heap_total - g_mem_heap_used) >= size)
      {
        g_mem_heap_used += size;
        if (g_mem_heap_upper_limit < g_mem_heap_used)
          g_mem_heap_upper_limit = g_mem_heap_used;
      }
      else
      {
        success = false;
#if (LOG_DISABLE == 0)
        _LOGCreateEntry("[ERROR]\t_MemPtrNew()- insufficient memory available");
#endif
      }
    }

    if (!success)
    {
      free(p);
      p = (void *)NULL;

      // we gotta mark it as free now as well
      memset(chunk, 0, sizeof(MemChunkInfo));
      g_mem_count--;

      goto MEM_DONE;
    }

    // record upper limit
    if (g_mem_max_chunks < g_mem_count)
      g_mem_max_chunks = g_mem_count;

#if (LOG_DISABLE == 0)
    // LOGGING EVENT
    {
      char str_log[256];

      sprintf(str_log,
              "[%4d]\t_MemPtrNew\tsize = %6lu,\tmemory use = %6lu\t%s%s%s",
              g_mem_count-1, size, (uint32)(g_mem_stor_used + g_mem_heap_used),
              (storage) ? "storage" : "heap",
              (storage == chunk->storage) ? "\tSAME" : "\tDIFF",
              (success) ? "" : "\t[failed]");
      _LOGCreateEntry(str_log);
    }
#endif
  }
#if (LOG_DISABLE == 0)
  else
    _LOGCreateEntry("[ERROR]\t_MemPtrNew()- unable to find available block");
#endif

MEM_DONE:

  return p;
}

// turn off memory protection (write to storage)
void
_MemSemaphore(boolean active)
{
  // this isn't used at all on symbian
}

// free a chunk of memory
void
_MemPtrFree(void *p)
{
  MemChunkInfo  *chunk;
  int            i;
  boolean        found;
  GLOBALS_ACCESS_MEM;

  // entry level condition (no NULL pointers)
  if (p == NULL) return;

  // locate the memory 'management' chunk
  if (g_mem_count != 0)
  {
    chunk = (MemChunkInfo *)g_mem_info;

    found = false;
    i     = g_mem_max;
    do
    {
      if (chunk->x == p) { found = true; goto FREE_SEARCH; }
      chunk++; // look at the next chunk?
    } while (--i);

FREE_SEARCH:

    // did we find the pointer in question?
    if (found)
    {
      free(p);

      // heap/storage validation
      if (chunk->storage)
        g_mem_stor_used -= chunk->size;
      else
        g_mem_heap_used -= chunk->size;

#if (LOG_DISABLE == 0)
      // LOGGING EVENT
      {
        char str_log[256];

        sprintf(str_log,
                "[%4d]\t_MemPtrFree\tsize = %6lu,\tmemory use = %6lu",
                (g_mem_count-1), (uint32)chunk->size,
                (uint32)(g_mem_stor_used + g_mem_heap_used));
        _LOGCreateEntry(str_log);
      }
#endif

      chunk->size    = 0;
      chunk->storage = false;
      chunk->x       = NULL;
      g_mem_count--;
    }
#if (LOG_DISABLE == 0)
    else
      _LOGCreateEntry("[ERROR]\t_MemPtrFree()- unable to find memory block");
#endif
  }
}

// determine how much memory is available
uint32
_MemAvailable(boolean storage)
{
  GLOBALS_ACCESS_MEM;

  if (storage) return g_mem_stor_total - g_mem_stor_used;
  else         return g_mem_heap_total - g_mem_heap_used;
}

// determine the size of a memory pointer allocated previously
uint32
_MemPtrSize(void *p)
{
  MemChunkInfo *chunk;
  int           i;
  uint32        size;
  GLOBALS_ACCESS_MEM;

  // default return value
  size = 0;

  // locate the memory 'management' chunk
  if (g_mem_count != 0)
  {
    chunk = (MemChunkInfo *)g_mem_info;
    if (chunk != NULL)
    {
      i = g_mem_max;
      do
      {
        if (chunk->x == p) { size = chunk->size; goto SIZE_SEARCH_DONE; }
      } while (--i);

SIZE_SEARCH_DONE:
      ;
    }
  }

  return size;
}

// terminate the memory manager; free all used memory
static void
_MemTerminate()
{
  MemChunkInfo *chunk;
  int           i, pass;
#if (LOG_DISABLE == 0)
  int           j, k;
#endif
  GLOBALS_ACCESS_MEM;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _MemTerminate");
#endif

  if (g_mem)
  {
    // memory leak in application, lazy developer: clean up for them
    if (g_mem_count != 0)
    {
#if (LOG_DISABLE == 0)
      // LOGGING EVENT
      _LOGCreateEntry(":: - application MEMORY LEAKS -");
#endif

      for (pass=0; pass<2; pass++)
      {
#if (LOG_DISABLE == 0)
        _LOGCreateEntry("");
#endif

        chunk = (MemChunkInfo *)g_mem_info;
        if (chunk != NULL)
        {
          i = g_mem_max;
          do
          {
            if (chunk->x != NULL)
            {
              if (pass == 0)
              {
#if (LOG_DISABLE == 0)
                char  str_log[256], str_tmp[32];
                char  data[(MEM_DUMP_LINE_SIZE * 4)];
                int32 size, cnt;

                size = MIN((MEM_DUMP_LINE_SIZE * 4), chunk->size);
                cnt  = size / MEM_DUMP_LINE_SIZE;
                if ((size % MEM_DUMP_LINE_SIZE) != 0) cnt++;

                // log entry
                sprintf(str_log,
                        "> mem size = %6lu @ 0x%08lx", 
                        (uint32)chunk->size, (uint32)chunk->x);
                _LOGCreateEntry(str_log);
  
                memcpy(data, chunk->x, size);
                for (k=0; k<cnt; k++)
                {
                  sprintf(str_log, "\t");
                  for (j=0; j<MEM_DUMP_LINE_SIZE; j++)
                  {
                    if (j >= (int)size)
                      sprintf(str_tmp, "   ");
                    else
                      sprintf(str_tmp, "%02x ",
                              (uint8)data[k*MEM_DUMP_LINE_SIZE + j]);
                    strcat(str_log, str_tmp);
                  }
                  strcat(str_log, " :: '");
                  for (j=0; j<MIN(MEM_DUMP_LINE_SIZE, (int)size); j++)
                  {
                    if ((data[k*MEM_DUMP_LINE_SIZE + j] > 31) &&
                        (data[k*MEM_DUMP_LINE_SIZE + j] < 127))
                      sprintf(str_tmp, "%c", data[k*MEM_DUMP_LINE_SIZE + j]);
                    else
                      sprintf(str_tmp, ".");
                    strcat(str_log, str_tmp);
                  }
                  strcat(str_log, "'");

                  _LOGCreateEntry(str_log);
                  size -= MEM_DUMP_LINE_SIZE;
                }
#endif
              }
              else
                _MemPtrFree(chunk->x);
            }
            chunk++;
          } while (--i);
        }
      }
    }

    // clean up memory
    if (g_mem_info != NULL) free(g_mem_info);
    g_mem_info = (MemChunkInfo *)NULL;

#if (LOG_DISABLE == 0)
    // LOGGING EVENT (upper limits)
    {
      char str_log[256];

      _LOGCreateEntry("\n:: - memory usage statistics - ");
      sprintf(str_log,
              "\tmax heap = %ld\tmax storage = %ld\tmax chunk use = %ld",
              g_mem_heap_upper_limit, g_mem_stor_upper_limit, g_mem_max_chunks);
      _LOGCreateEntry(str_log);
    }
#endif
  }

  // globals:: clean up memory
  if (g_mem) free(g_mem); g_mem = NULL;
  g->mem = g_mem;
}

#undef GLOBALS_DEFINE_MEM
#undef GLOBALS_ACCESS_MEM

#undef g_mem_info
#undef g_mem_count
#undef g_mem_max
#undef g_mem_heap_total
#undef g_mem_heap_used
#undef g_mem_heap_upper_limit
#undef g_mem_stor_total
#undef g_mem_stor_used
#undef g_mem_stor_upper_limit
#undef g_mem_max_chunks

#undef MEM_DUMP_LINE_SIZE

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-memory.inc"

/********************************* EOF ***********************************/
