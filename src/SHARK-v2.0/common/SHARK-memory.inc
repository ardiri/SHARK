/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-memory.c
 */

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

// set a series of memory locations with a specific value
void
_MemSet(void *p, uint8 value, uint32 count)
{
  uint8 *x;
  int    i;

  // pre-condition (cannot have null pointer)
  if ((p == NULL) || (count == 0)) return;

  // use temporary variables for processing
  x = (uint8 *)p;

  // fill buffer with value
  i = count;
  do
  {
    *x++ = value;
  } while (--i);
}

// move memory from one location to another
void
_MemMove(void *pDst, void *pSrc, uint32 count)
{
  uint8  *x, *y;
  int     i;
  boolean reverse;

  // pre-condition (cannot have null pointer)
  if ((pDst == NULL) || (pSrc == NULL) || (count == 0)) return;

  // use temporary variables for processing
  x = (uint8 *)pDst;
  y = (uint8 *)pSrc;

  // we must check if the memory regions overlap
  reverse = false;
  if (x > y) reverse = ((uint32)x < (uint32)(y + count));
  
  // if so, we start at the end, work way back
  if (reverse) 
  { 
    x += count; 
    y += count;

    // copy buffer as appropriate
    i = count;
    do
    {
      *--x = *--y;
    } while (--i);
  }
  else
  {
    // copy buffer as appropriate
    i = count;
    do
    {
      *x++ = *y++;
    } while (--i);
  }
}

// copy memory from one location to another
void
_MemCopy(void *pDst, void *pSrc, uint32 count)
{
  uint8 *x, *y;
  int    i;

  // pre-condition (cannot have null pointer)
  if ((pDst == NULL) || (pSrc == NULL) || (count == 0)) return;

  // use temporary variables for processing
  x = (uint8 *)pDst;
  y = (uint8 *)pSrc;

  // copy buffer as appropriate
  i = count;
  do
  {
    *x++ = *y++;
  } while (--i);
}

// compare two regions of memory
int32
_MemCompare(void *p1, void *p2, uint32 count)
{
  uint8  *x, *y;
  int32   diff;
  int     i;

  // default return value
  diff = 0;

  // pre-condition (cannot have null pointer)
  if ((p1 != NULL) && (p2 != NULL) && (count != 0))
  {
    // use temporary variables for processing
    x = (uint8 *)p1;
    y = (uint8 *)p2;

    // compare memory buffer as appropriate
    i = count;
    do
    {
      if (*x == *y) { x++; y++; }
      else
      {
        diff = *x - *y;
        goto MEM_COMPARE_DONE;
      }
    } while (--i);

MEM_COMPARE_DONE:
    ;
  }

  return diff;
}

// look for a specific character in the memory buffer
char *
_MemChr(void *p, char chr, uint32 count)
{
  char *pos;
  char *x;
  int   i;

  // default return value
  pos = NULL;

  // pre-condition (cannot have null pointer)
  if ((p != NULL) && (count != 0))
  {
    // use temporary variables for processing
    x = (char *)p;

    // lets scan the memory buffer
    i = count;
    do
    {
      if (*x == chr) { pos = x; goto MEM_CHR_DONE; }
      x++;
    } while (--i);

MEM_CHR_DONE:
    ;
  }

  return pos;
}

// look for a specific character in the memory buffer, starting at end
char *
_MemRChr(void *p, char chr, uint32 count)
{
  char *pos;
  char *x;
  int   i;

  // default return value
  pos = NULL;

  // pre-condition (cannot have null pointer)
  if ((p != NULL) && (count != 0))
  {
    // use temporary variables for processing
    x = (char *)p;
    x += count;

    // lets scan the memory buffer
    i = count;
    do
    {
      if (*x == chr) { pos = x; goto MEM_R_CHR_DONE; }
      x--;
    } while (--i);

MEM_R_CHR_DONE:
    ;
  }

  return pos;
}

typedef struct
{
  uint32  nSize;
  uint32  nCount;
  uint32  nOffset;
  void   *pData;
} _MemPool;

MemPool *
_MemPoolNew(uint32 size, uint32 count)
{
  _MemPool *pool;

  // default return value
  pool = NULL;

  // try to allocate the chunks required
  pool = (_MemPool *)_MemPtrNew(sizeof(_MemPool), false);
  if (pool != NULL)
  {
    pool -> nSize   = size;
    pool -> nCount  = count;
    pool -> nOffset = 0;
    pool -> pData   = _MemPtrNew((pool -> nSize * pool -> nCount), false);

    // memory allocation check - ensure we could allocate it
    if (pool -> pData == NULL)
    {
      _MemPtrFree(pool); 
      pool = NULL;
    }
  }

  return (MemPool *)pool;
}

void *
_MemPoolReserve(MemPool *pool, uint32 count)
{
  uint8    *result;
  _MemPool *_p = (_MemPool *)pool;

  // default return value
  result = NULL;

  // lets ensure we have the pool allocated
  if ((_p != NULL) && (count > 0)  &&
      (_p -> pData != NULL))
  {
    // do we have enough space free?
    if ((_p -> nCount - _p -> nOffset) >= count)
    {
      result  = (uint8 *)_p -> pData;
      result += (_p -> nOffset * _p -> nSize);

      _p -> nOffset += count;
    }
  }

  return result;
}

void
_MemPoolFree(MemPool *pool)
{
  _MemPool *_p = (_MemPool *)pool;

  if (_p != NULL)
  {
    if (_p -> pData != NULL) 
      _MemPtrFree(_p -> pData);
    _MemPtrFree(_p);
  }
}

/********************************* EOF ***********************************/
