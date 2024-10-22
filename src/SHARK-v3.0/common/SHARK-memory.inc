/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-memory.inc
 */

#if (LOG_DISABLE == 0)
#define MEM_MANAGED_BUFFERS    
#endif

#define MEM_OPTIMIZATIONS

/*************************************************************************
 * Configuration
 *************************************************************************/

#define MEM_DUMP_LINE_SIZE         16 // for memory leak reporting

/*************************************************************************
 * Component Variables
 *************************************************************************/

typedef struct MemChunkInfo
{
  void      *x;
  uint32     storage:1;
  uint32     size:31;

#if defined(DAL_MEMCHUNKINFO)
  DAL_MEMCHUNKINFO
#endif

} MemChunkInfo;

typedef struct MemGlobals
{
  void *struct_prefix;

  MemChunkInfo *mem_info;
  uint16        mem_count;
  uint16        mem_max;

  uint32        mem_heap_total;
  uint32        mem_heap_used;
  uint32        mem_heap_upper_limit;
  uint32        mem_stor_total;
  uint32        mem_stor_used;
  uint32        mem_stor_upper_limit;
  uint32        mem_max_chunks;
   int32        mem_semaphore_count;

#if defined(DAL_MEM_GLOBALS)
  DAL_MEM_GLOBALS
#endif

} MemGlobals;

GLOBALS_DEFINITION;

#define g_mem_info             (MEM_GLOBALS) -> mem_info
#define g_mem_count            (MEM_GLOBALS) -> mem_count
#define g_mem_max              (MEM_GLOBALS) -> mem_max

#define g_mem_heap_total       (MEM_GLOBALS) -> mem_heap_total
#define g_mem_heap_used        (MEM_GLOBALS) -> mem_heap_used
#define g_mem_heap_upper_limit (MEM_GLOBALS) -> mem_heap_upper_limit
#define g_mem_stor_total       (MEM_GLOBALS) -> mem_stor_total
#define g_mem_stor_used        (MEM_GLOBALS) -> mem_stor_used
#define g_mem_stor_upper_limit (MEM_GLOBALS) -> mem_stor_upper_limit
#define g_mem_max_chunks       (MEM_GLOBALS) -> mem_max_chunks

#define g_mem_semaphore_count  (MEM_GLOBALS) -> mem_semaphore_count

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

static boolean DAL_MemInitialize();
static void    DAL_MemPtrNewDynamicHeap(MemChunkInfo *, uint32);
static void    DAL_MemPtrNewStorageHeap(MemChunkInfo *, uint32);
static void    DAL_MemPtrFreeDynamicHeap(MemChunkInfo *);
static void    DAL_MemPtrFreeStorageHeap(MemChunkInfo *);
static void    DAL_MemSemaphoreReserve();
static void    DAL_MemSemaphoreRelease();
static void    DAL_MemTerminate();

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
_MemInitialize()
{
  boolean init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _MemInitialize");
#endif

  // allocate memory for globals
  MEM_GLOBALS = malloc(sizeof(MemGlobals));
  if (GLOBALS_UNINITIALIZED) return false;

  // assume everything is ok
  init_ok  = true;
  init_ok &= DAL_MemInitialize();

  // did we initialize ok?
  if (init_ok)
  {
    g_mem_count = 0;
    g_mem_max   = MAX(g_config.memory.count, 64); // 64 min
    g_mem_info  = (MemChunkInfo *)malloc(sizeof(MemChunkInfo) * (g_mem_max+1));
    init_ok &= (g_mem_info != NULL); if (!init_ok) goto MEM_INIT_ABORT;
    memset(g_mem_info, 0, sizeof(MemChunkInfo) * (g_mem_max+1));

    // initialize the memory requirements
    g_mem_heap_total       = MAX(1048576L, g_config.memory.max_heap);
    g_mem_stor_total       = g_config.memory.max_storage;

    // initialize our reporting statistics
    g_mem_heap_used        = 0;
    g_mem_heap_upper_limit = 0;
    g_mem_stor_used        = 0;
    g_mem_stor_upper_limit = 0;
    g_mem_max_chunks       = 0;

    // initialize our semaphore count
    g_mem_semaphore_count  = 0;
  }

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  {
    char str_log[256];

    sprintf(str_log, "\nmaximum chunks  :: %d", g_mem_max);
    _SHARKLogCreateEntry(str_log);
    sprintf(str_log, "dynamic heap    :: %lu", (unsigned long)g_mem_heap_total);
    _SHARKLogCreateEntry(str_log);
    sprintf(str_log, "storage heap    :: %lu\n", (unsigned long)g_mem_stor_total);
    _SHARKLogCreateEntry(str_log);
  }
#endif

MEM_INIT_ABORT:

  return init_ok;
}

uint32
_MemAvailable(boolean storage)
{
  uint32 result;

  // default return value
  result = 0;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  if (storage) result = g_mem_stor_total - g_mem_stor_used;
  else         result = g_mem_heap_total - g_mem_heap_used;

  return result;
}

void *
_MemCopy(void *pDst, void *pSrc, uint32 count)
{
  uint8  *x, *y;
  uint32 *_x, *_y;   // optimizations
  uint16 *__x, *__y; // optimizations
  int    i, cnt;

  // pre-condition 
  if ((pDst == NULL) || (pSrc == NULL) || (count == 0)) return pDst;

  // use temporary variables for processing
  x = (uint8 *)pDst;
  y = (uint8 *)pSrc;

  // do pointers align on dword's : same way?
  if ((((unsigned long)x & 0x3) == ((unsigned long)y & 0x03)) && (count > 4)) 
  {
    // copy the bits in front (non dword)
    cnt = 4 - (uint32)((unsigned long)x & 0x03);
    if (cnt > 0)
    { 
      i = cnt;
      do
      {
#ifdef MEM_OPTIMIZATIONS
        x[0] = y[0]; 
        x++; y++;
#else
        *x++ = *y++;
#endif
      } while (--i);
      count -= cnt;
    }

    // lets copy dwords
    _x = (uint32 *)x;
    _y = (uint32 *)y;
    i = (count >> 2);
    if (i > 0)
    {
#ifdef MEM_OPTIMIZATIONS
      while (i & 7)
      {
        _x[0] = _y[0]; 
        _x++; _y++; i--;
      }
      i /= 8;
      while (i--)
      {
        _x[0] = _y[0]; 
        _x[1] = _y[1]; 
        _x[2] = _y[2]; 
        _x[3] = _y[3]; 
        _x[4] = _y[4]; 
        _x[5] = _y[5]; 
        _x[6] = _y[6]; 
        _x[7] = _y[7]; 
        _x += 8; _y += 8;
      }
#else
      do
      {
        *_x++ = *_y++;
      } while (--i);
#endif
    }

    // copy the bits at end (non dword) 
    cnt = count & 0x03;
    if (cnt > 0)
    {
      x = (uint8 *)_x;
      y = (uint8 *)_y;

      i = cnt;
      do
      {
#ifdef MEM_OPTIMIZATIONS
        x[0] = y[0]; 
        x++; y++;
#else
        *x++ = *y++;
#endif
      } while (--i);
    }
  }
  else

  // do pointers align on word's : same way?
  if ((((unsigned long)x & 0x1) == ((unsigned long)y & 0x01)) && (count > 2))
  {
    // copy the bits in front (non word)
    if ((unsigned long)x & 0x01)
    { 
#ifdef MEM_OPTIMIZATIONS
      x[0] = y[0]; 
      x++; y++;
#else
      *x++ = *y++;
#endif
      count--;
    }

    // lets copy words
    __x = (uint16 *)x;
    __y = (uint16 *)y;
    i = (count >> 1);
    if (i > 0)
    {
#ifdef MEM_OPTIMIZATIONS
      while (i & 7)
      {
        __x[0] = __y[0]; 
        __x++; __y++; i--;
      }
      i /= 8;
      while (i--)
      {
        __x[0] = __y[0]; 
        __x[1] = __y[1]; 
        __x[2] = __y[2]; 
        __x[3] = __y[3]; 
        __x[4] = __y[4]; 
        __x[5] = __y[5]; 
        __x[6] = __y[6]; 
        __x[7] = __y[7]; 

        __x += 8; __y += 8;
      }
#else
      do
      {
        *__x++ = *__y++;
      } while (--i);
#endif
    }

    // copy the bits at end (non word) 
    if (count & 0x01)
    {
      x = (uint8 *)__x;
      y = (uint8 *)__y;

#ifdef MEM_OPTIMIZATIONS
      x[0] = y[0]; 
#else
      *x = *y;
#endif
    }
  }

  // nope; not much we can do
  else
  {
    // copy buffer as appropriate
    i = count;
#ifdef MEM_OPTIMIZATIONS
    while (i & 7)
    {
      x[0] = y[0]; 
      x++; y++; i--;
    }
    i /= 8;
    while (i--)
    {
      x[0] = y[0]; 
      x[1] = y[1]; 
      x[2] = y[2]; 
      x[3] = y[3]; 
      x[4] = y[4]; 
      x[5] = y[5]; 
      x[6] = y[6]; 
      x[7] = y[7]; 

      x += 8; y += 8;
    }
#else
    do
    {
      *x++ = *y++;
    } while (--i);
#endif
  }

  return pDst;
}

void *
_MemMove(void *pDst, void *pSrc, uint32 count)
{
  uint8  *x, *y;
  uint32 *_x, *_y;   // optimizations
  uint16 *__x, *__y; // optimizations
  int     i, cnt;
  boolean reverse;

  // pre-condition 
  if ((pDst == NULL) || (pSrc == NULL) || (count == 0)) return pDst;

  // use temporary variables for processing
  x = (uint8 *)pDst;
  y = (uint8 *)pSrc;

  // we must check if the memory regions overlap
  reverse = false;
  if (x > y) reverse = ((unsigned long)x < (unsigned long)(y + count));
  
  // if so, we start at the end, work way back
  if (reverse) 
  { 
    x += count; 
    y += count;

    // do pointers align on dword's : same way?
    if ((((unsigned long)x & 0x3) == ((unsigned long)y & 0x03)) && (count > 4))
    {
      // copy the bits in front (non dword)
      cnt = (uint32)((((unsigned long)x)-1) & 0x03);
      if (cnt > 0)
      { 
        i = cnt;
        do
        {
#ifdef MEM_OPTIMIZATIONS
          x--; y--;
          x[0] = y[0];
#else
          *--x = *--y;
#endif
        } while (--i);
        count -= cnt;
      }

      // lets copy dwords
      _x = (uint32 *)x;
      _y = (uint32 *)y;
      i = (count >> 2);
      if (i > 0)
      {
#ifdef MEM_OPTIMIZATIONS
        while (i & 7)
        {
          _x--; _y--; i--;
          _x[0] = _y[0];
        }
        i /= 8;
        while (i--)
        {
          _x -= 8; _y -= 8;

          _x[7] = _y[7];
          _x[6] = _y[6];
          _x[5] = _y[5];
          _x[4] = _y[4];
          _x[3] = _y[3];
          _x[2] = _y[2];
          _x[1] = _y[1];
          _x[0] = _y[0];
        }
#else
        do
        {
          *--_x = *--_y;
        } while (--i);
#endif
      }
  
      // copy the bits at end (non dword) 
      cnt = 4 - ((count-1) & 0x03);
      if (cnt > 0)
      {
        x = (uint8 *)_x;
        y = (uint8 *)_y;
  
        i = cnt;
        do
        {
#ifdef MEM_OPTIMIZATIONS
          x--; y--;
          x[0] = y[0];
#else
          *--x = *--y;
#endif
        } while (--i);
      }
    }
    else
  
    // do pointers align on word's : same way?
    if ((((unsigned long)x & 0x1) == ((unsigned long)y & 0x01)) && (count > 2))
    {
      // copy the bits in front (non word)
      if ((((unsigned long)x)-1) & 0x01)
      { 
#ifdef MEM_OPTIMIZATIONS
        x--; y--;
        x[0] = y[0];
#else
        *--x = *--y;
#endif
        count--;
      }
  
      // lets copy words
      __x = (uint16 *)x;
      __y = (uint16 *)y;
      i = (count >> 1);
      if (i > 0)
      {
#ifdef MEM_OPTIMIZATIONS
        while (i & 7)
        {
          __x--; __y--; i--;
          __x[0] = __y[0];
        }
        i /= 8;
        while (i--)
        {
          __x -= 8; __y -= 8;

          __x[7] = __y[7];
          __x[6] = __y[6];
          __x[5] = __y[5];
          __x[4] = __y[4];
          __x[3] = __y[3];
          __x[2] = __y[2];
          __x[1] = __y[1];
          __x[0] = __y[0];
        }
#else
        do
        {
          *--__x = *--__y;
        } while (--i);
#endif
      }
  
      // copy the bits at end (non word) 
      if ((count-1) & 0x01)
      {
        x = (uint8 *)__x;
        y = (uint8 *)__y;
  
#ifdef MEM_OPTIMIZATIONS
        x--; y--;
        x[0] = y[0];
#else
        *--x = *--y;
#endif
      }
    }
  
    // nope; not much we can do
    else
    {
      // copy buffer as appropriate
      i = count;
#ifdef MEM_OPTIMIZATIONS
      while (i & 7)
      {
        x--; y--; i--;
        x[0] = y[0];
      }
      i /= 8;
      while (i--)
      {
        x -= 8; y -= 8;

        x[7] = y[7];
        x[6] = y[6];
        x[5] = y[5];
        x[4] = y[4];
        x[3] = y[3];
        x[2] = y[2];
        x[1] = y[1];
        x[0] = y[0];
      }
#else
      do
      {
        *--x = *--y;
      } while (--i);
#endif
    }
  }
  else
    _MemCopy(x, y, count);

  return pDst;
}

void
_MemPtrFree(void *p)
{
  MemChunkInfo  *chunk;
  int16          i;
  boolean        found;
#ifdef MEM_MANAGED_BUFFERS    
  uint8         *_p;
  uint32         size;
#endif

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry level condition 
  if (p == NULL) return;

  // locate the memory 'management' chunk
  if (g_mem_count != 0)
  {
    chunk = (MemChunkInfo *)g_mem_info;

#ifdef MEM_MANAGED_BUFFERS    
    _p = p;
    _p = _p - 4;
#endif

    found = false;
    i     = g_mem_max;
    do
    {
#ifndef MEM_MANAGED_BUFFERS    
      if (chunk -> x ==  p) { found = true; break; }
#else
      if (chunk -> x == _p) { found = true; break; }
#endif
      chunk++; // look at the next chunk?
    } while (--i);

    // did we find the pointer in question?
    if (found)
    {
#ifdef MEM_MANAGED_BUFFERS    
      size = chunk -> size;
      size = size + 8;

      if ((_p[0]      != 'D') || (_p[1]      != 'E') || 
          (_p[2]      != 'A') || (_p[3]      != 'D'))
        printf("buffer underflow\n");
      if ((_p[size-4] != 'B') || (_p[size-3] != 'E') || 
          (_p[size-2] != 'E') || (_p[size-1] != 'F'))
        printf("buffer overflow\n");
#endif

      // free the memory
      if (chunk -> storage) DAL_MemPtrFreeStorageHeap(chunk);
      else                  DAL_MemPtrFreeDynamicHeap(chunk);

      // heap/storage validation
      if (chunk -> storage) g_mem_stor_used -= chunk -> size;
      else                  g_mem_heap_used -= chunk -> size;

      g_mem_count--;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  {
    char   str_log[256];
    uint32 chunk_size;

    chunk_size = chunk -> size;
    sprintf(str_log,
            "[%4d]\t_MemPtrFree\tsize = %7lu,\t\t\tmemory use = %7lu",
            g_mem_count, (unsigned long)chunk_size,
            (unsigned long)(g_mem_stor_used + g_mem_heap_used));
    _SHARKLogCreateEntry(str_log);
  }
#endif
    }
    else
    {

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry("[ERROR]\t_MemPtrFree()- unable to find memory block");
#endif

    }
  }
}

void *
_MemPtrNew(uint32 size, boolean storage)
{
  void         *result;
  MemChunkInfo *chunk;
  int16         index;
#ifdef MEM_MANAGED_BUFFERS    
  uint8        *_p;
#endif

  // default return value
  result = NULL;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry-condition
  if (size == 0) return result;

  // can we allocate another chunk?
  if (g_mem_count < (g_mem_max-1))
  {
    chunk = (MemChunkInfo *)g_mem_info;

    // locate the first available memory management chunk
    index = g_mem_max;
    do
    {
      if (chunk -> x == NULL) goto MEM_NEW_FOUND;
      chunk++;
    } while (--index);

    // did not find a free slot, time to exit
    goto MEM_NEW_DONE;

MEM_NEW_FOUND:;

    // confiure the memory management chunk
    memset(chunk, 0, sizeof(MemChunkInfo));

    // heap validation
    if (storage)
    {
      // do we have enough memory for this operation?
      if ((g_mem_stor_total - g_mem_stor_used) >= size)
        g_mem_stor_used += size;
      else
      {
        // try dynamic heap if it fails
        storage = false;
        goto MEM_NEW_DYNAMIC_HEAP;
      }
    }
    else
    {

MEM_NEW_DYNAMIC_HEAP:;

      // do we have enough memory for this operation?
      if ((g_mem_heap_total - g_mem_heap_used) >= size)
        g_mem_heap_used += size;
      else
      {

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry("[ERROR]\t_MemPtrNew()- insufficient memory available");
#endif

        goto MEM_NEW_DONE;
      }
    }

    // try the storage heap (if requested), then dynamic heap
#ifdef MEM_MANAGED_BUFFERS    
    size += 8;
#endif
    if (storage)              DAL_MemPtrNewStorageHeap(chunk, size);
    if (chunk -> x == NULL) { storage = false;
                              DAL_MemPtrNewDynamicHeap(chunk, size); }

    // were we able to allocate the memory?
    if (chunk -> x != NULL) 
    {
      // clear the memory
      if (storage) _MemSemaphore(true);
      _MemSet(chunk -> x, 0, size);
      if (storage) _MemSemaphore(false);

#ifdef MEM_MANAGED_BUFFERS    
      _p = chunk -> x;
      _p[0]      = 'D'; _p[1]      = 'E'; _p[2]      = 'A'; _p[3]      = 'D';
      _p[size-4] = 'B'; _p[size-3] = 'E'; _p[size-2] = 'E'; _p[size-1] = 'F';

      size -= 8;
      chunk -> size = size;
#endif

      g_mem_count++;

      // we now can use the pointer
#ifndef MEM_MANAGED_BUFFERS    
      result = chunk -> x;
#else
      _p     = chunk -> x;
      result = _p + 4;
#endif
      
      // record upper limits
      if (g_mem_max_chunks < g_mem_count)
        g_mem_max_chunks = g_mem_count;
      if (g_mem_heap_upper_limit < g_mem_heap_used)
        g_mem_heap_upper_limit = g_mem_heap_used;
      if (g_mem_stor_upper_limit < g_mem_stor_used)
        g_mem_stor_upper_limit = g_mem_stor_used;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  {
    char str_log[256];
    sprintf(str_log,
            "[%4d]\t_MemPtrNew\tsize = %7lu @ 0x%08lx,\tmemory use = %7lu",
            g_mem_count-1, (unsigned long)size, (unsigned long)chunk -> x, 
            (unsigned long)(g_mem_stor_used+g_mem_heap_used));
    _SHARKLogCreateEntry(str_log);
  }
#endif

    }

    // not able to find a consecutive chunk?
    else
    {
      // correct our memory usage values
      if (storage) g_mem_stor_used -= size;
      else         g_mem_heap_used -= size;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry("[ERROR]\t_MemPtrNew()- cannot allocate consecutive chunk");
#endif

    }
  }
  else
  {

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry("[ERROR]\t_MemPtrNew()- unable to find a free memory block");
#endif

  }

MEM_NEW_DONE:;

  return result;
}

uint32
_MemPtrSize(void *p)
{
  uint32        result;
  MemChunkInfo *chunk;
  int16         i;
  boolean       found;
#ifdef MEM_MANAGED_BUFFERS    
  uint8         *_p;
#endif

  // default return value
  result = 0;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry level condition 
  if (p == NULL) return result;

  // locate the memory 'management' chunk
  if (g_mem_count != 0)
  {
    chunk = (MemChunkInfo *)g_mem_info;

#ifdef MEM_MANAGED_BUFFERS    
    _p = p;
    _p = _p - 4;
#endif

    found = false;
    i     = g_mem_max;
    do
    {
#ifndef MEM_MANAGED_BUFFERS    
      if (chunk -> x ==  p) { found = true; break; }
#else
      if (chunk -> x == _p) { found = true; break; }
#endif
      chunk++; // look at the next chunk?
    } while (--i);

    // if we located 
    if (found)
      result = chunk -> size;
    else
    {

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry("[ERROR]\t_MemPtrSize()- unable to find memory block");
#endif

    }
  }

  return result;
}

void 
_MemSemaphore(boolean active)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  if (active)
    g_mem_semaphore_count++;
  else
  {

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  if (g_mem_semaphore_count == 0)
    _SHARKLogCreateEntry("[ERROR]\t_MemSemaphore() - activation mismatch");
#endif

    g_mem_semaphore_count--;
  }

  // remember the stack while calling this routine
  active |= (g_mem_semaphore_count != 0);

  // make the request to change the storage area protection
  if (active) DAL_MemSemaphoreReserve();
  else        DAL_MemSemaphoreRelease();
}

void *
_MemSet(void *p, uint8 value, uint32 count)
{
  uint8  *x;
  uint32 *_x, val;
  int    i, cnt;

  // pre-condition 
  if ((p == NULL) || (count == 0)) return p;

  // use temporary variables for processing
  x = (uint8 *)p;

  cnt = 4 - (uint32)((unsigned long)p & 0x03);
  if (cnt > 0)
  {
    i = cnt;
    if (cnt > (int)count) cnt = count;  // for small buffers
    do
    {
      *x++ = value;
    } while (--i);
    count -= cnt;
  }
  
  _x = (uint32 *)x;
  cnt = count >> 2;
  if (cnt > 0)
  {
    val = (value << 24) | (value << 16) | (value << 8) | value;
    i = cnt;
#ifdef MEM_OPTIMIZATIONS
    while (i & 7)
    {
      _x[0] = val; 
      _x++; i--;
    }
    i /= 8;
    while (i--)
    {
      _x[0] = val; 
      _x[1] = val; 
      _x[2] = val; 
      _x[3] = val; 
      _x[4] = val; 
      _x[5] = val; 
      _x[6] = val; 
      _x[7] = val;
      _x += 8; 
    }
#else
    do
    {
      *_x++ = val;
    } while (--i);
#endif
  }
  
  x = (uint8 *)_x;
  cnt = count & 0x03;
  if (cnt > 0)
  {
    i = cnt;
    do
    {
      *x++ = value;
    } while (--i);
  }
  
  return p;
}

void 
_MemTerminate()
{
  MemChunkInfo *chunk;
  int           i;
#if (LOG_DISABLE == 0)
  int           j, k;
#endif

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _MemTerminate");
#endif

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // memory leak in application, lazy developer: clean up for them
  if (g_mem_count != 0)
  {

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry("\nMEMORY LEAKS:");
#endif

    chunk = (MemChunkInfo *)g_mem_info;
    if (chunk != NULL)
    {
      i = g_mem_max;
      do
      {
        if (chunk -> x != NULL)
        {
#if (LOG_DISABLE == 0)
          char   str_log[256], str_tmp[32];
          char   data[(MEM_DUMP_LINE_SIZE * 4)];
          int32  size, cnt;
          uint32 chunk_size;

          size = MIN((MEM_DUMP_LINE_SIZE * 4), chunk -> size);
          cnt  = size / MEM_DUMP_LINE_SIZE;
          if ((size % MEM_DUMP_LINE_SIZE) != 0) cnt++;

          chunk_size = chunk -> size;

  // LOGGING EVENT
  sprintf(str_log,
          "\n> mem size = %7lu @ 0x%08lx",
          (unsigned long)chunk_size, (unsigned long)chunk -> x);
  _SHARKLogCreateEntry(str_log);

          memcpy(data, chunk -> x, size);
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

  // LOGGING EVENT
  _SHARKLogCreateEntry(str_log);

            size -= MEM_DUMP_LINE_SIZE;
          }
#endif

          _MemPtrFree(chunk -> x);
        }

        chunk++;
      } while (--i);
    }
  }

  // clean up memory
  if (g_mem_info != NULL) free(g_mem_info);
  g_mem_info = (MemChunkInfo *)NULL;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT 
  {
    char str_log[256];

    sprintf(str_log, "\nmemory chunks   :: %lu", (unsigned long)g_mem_max_chunks);
    _SHARKLogCreateEntry(str_log);
    sprintf(str_log, "dynamic heap    :: %lu", (unsigned long)g_mem_heap_upper_limit);
    _SHARKLogCreateEntry(str_log);
    sprintf(str_log, "storage heap    :: %lu\n", (unsigned long)g_mem_stor_upper_limit);
    _SHARKLogCreateEntry(str_log);
  }
#endif

  // terminate the underlying platform manager
  DAL_MemTerminate();
}

#undef MEM_DUMP_LINE_SIZE

/********************************* EOF ***********************************/
