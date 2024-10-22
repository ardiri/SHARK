/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-core.c
 */

#include "../SHARK.h"
#include "../SHARK-core.h"

/*************************************************************************
 * Configuration
 *************************************************************************/

#define FILE_READ_CHUNK_SIZE  8192
#define FILE_WRITE_CHUNK_SIZE 8192

/*************************************************************************
 * Component Variables
 *************************************************************************/

// none

/*************************************************************************
 * Global Variables
 *************************************************************************/

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibSetGlobals() API.
//------------------------------------------------------------------------

// none

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

// none

/*************************************************************************
 * Implementation
 *************************************************************************/

boolean
_FileExists(uint32 disk, file *dir, char *name)
{
  boolean  result;
  file    *f;

  // default return value
  result = false;

  // entry requirement
  if (name == NULL) return result;

  f = _FileGetHandle(disk, dir, name);
  if (f != NULL)
  {
    _FileReleaseHandle(f);
    result = true;
  }

  return result;
}

uint32
_FileGetSize(file *f)
{
  uint32 result;

  // default return value
  result = 0;

  // get the size
  if (_FileOpen(f, FILE_READ_ONLY))
  {
    _FileSeek(f, 0, file_end);
    result = _FileTell(f);
    _FileClose(f);
  }

  return result;
}

error
_FileLoadIntoBuffer(file *f, void *buffer, uint32 size)
{
  error    result, err;
  uint32   cnt, num, amt, max;
  char    *tmp, *ptr;

  // default return value
  result = -1;

  // entry requirement
  if ((buffer == NULL) || (size < 1)) return result;

  // lets read it in!
  if (_FileOpen(f, FILE_READ_ONLY))
  {
    _FileSeek(f, 0, file_beginning);

    // try to read the file contents
    max = size;
    tmp = _MemPtrNew(FILE_READ_CHUNK_SIZE, false);
    if (tmp != NULL)
    {
      amt = cnt = 0;
      ptr = buffer;
      while (max > 0)
      {
        num = MIN(FILE_READ_CHUNK_SIZE, max);
        err = _FileRead(f, tmp, num, &amt);
        if ((err != errNone) || (num != amt)) break;

        _MemSemaphore(true);
        _MemMove(ptr, tmp, num); ptr += num; max -= num;
        _MemSemaphore(false);

        cnt += amt;
      }
      _MemPtrFree(tmp);

      // we have success if we loaded the complete file
      if (cnt == size) result = errNone;
    }

    _FileClose(f);
  }

  return result;
}

error
_FileShred(file *f)
{
  error    result, err;
  uint8   *buf;
  uint32   len, cnt, amt;

  // default return value
  result = -1;

  // entry requirement
  if (f == NULL) return result;

  buf = _MemPtrNew(FILE_WRITE_CHUNK_SIZE, false);
  if (buf != NULL) return result;

  // lets create the buffer we will be writing to the file
  _MemSet(buf, 0, FILE_WRITE_CHUNK_SIZE);
 
  // open the file for writing
  if (_FileOpen(f, FILE_READ_WRITE))
  {
    // how big is the file?
    _FileSeek(f, 0, file_end);
    len = _FileTell(f); 
    _FileSeek(f, 0, file_beginning);

    // lets overwrite the file with the shredded data
    while (len > 0)
    {
      cnt = MIN(len, FILE_WRITE_CHUNK_SIZE);
      err = _FileWrite(f, buf, cnt, &amt); 
      if ((err != errNone) || (cnt != amt))  break;
      len -= amt;
    }

    // did we read everything we requested?
    if (len == 0) result = errNone; 

    // close the file
    _FileClose(f);
  }

  return result;
}

char *
_MemChr(void *p, char chr, uint32 count)
{
  char *pos;
  char *x;
  int   i;

  // default return value
  pos = NULL;

  // pre-condition 
  if ((p == NULL) || (count == 0)) return pos;

  // use temporary variables for processing
  x = (char *)p;

  // lets scan the memory buffer
  i = count;
  do
  {
    if (*x == chr) { pos = x; break; }
    x++;
  } while (--i);

  return pos;
}

int32
_MemCompare(void *p1, void *p2, uint32 count)
{
  uint8  *x, *y;
  int32   diff;
  int     i;

  // default return value
  diff = 0;

  // pre-condition 
  if ((p1 == NULL) || (p2 == NULL) || (count == 0)) return diff;

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
      break;
    }
  } while (--i);

  return diff;
}

char *
_MemRChr(void *p, char chr, uint32 count)
{
  char *pos;
  char *x;
  int   i;

  // default return value
  pos = NULL;

  // pre-condition (cannot have null pointer)
  if ((p == NULL) || (count == 0)) return pos;
  
  // use temporary variables for processing
  x = (char *)p;
  x += count;

  // lets scan the memory buffer
  i = count;
  do
  {
    if (*x == chr) { pos = x; break; }
    x--;
  } while (--i);

  return pos;
}

void
_SysEnqueueQuitEvent()
{
  event e;

  _MemSet(&e, 0, sizeof(event));
  e.eType = _appStopEvent;
  _SysEnqueueEvent(&e);
}

#undef FILE_READ_CHUNK_SIZE  
#undef FILE_WRITE_CHUNK_SIZE  

/********************************* EOF ***********************************/
