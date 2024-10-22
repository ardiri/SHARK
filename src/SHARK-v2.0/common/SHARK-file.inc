/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-file.c
 */

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

boolean
_FileExists(uint32 disk, file *dir, char *name)
{
  file    *f;
  boolean  result;
  
  // default return value
  result = false;
  
  // entry condition
  if (name == NULL) return result;
  
  f = _FileGetHandle(disk, dir, name);
  if (f != NULL)
  {
    result = true;
    _FileReleaseHandle(f);
  }
  
  return result;
}

uint32
_FileGetSize(file *f)
{
  uint32 result;

  // default return value
  result = 0;

  // entry condition
  if (f == NULL) return result;

  // get the size
  if (_FileOpen(f, FILE_READ_ONLY))
  {
    _FileSeek(f, 0, file_end);
    result = _FileTell(f);
    _FileClose(f);
  }

  return result;
}

boolean
_FileLoadIntoBuffer(file *f, void *buffer, uint32 size)
{
  boolean  result;
  uint32   cnt, num, amt, max;
  char    *tmp, *ptr;

  // default return value
  result = false;

  // entry conditions
  if ((f == NULL) || (buffer == NULL) || (size < 1)) return result;

  // lets read it in!
  if (_FileOpen(f, FILE_READ_ONLY))
  {
    _FileSeek(f, 0, file_beginning);

// ARDIRI:
// - optimization to utilize 8K of heap and read direct to storage
// 
// OLD:
//  _FileRead(f, tmp, 1, size, &cnt);

    max = size; 
    tmp = _MemPtrNew(8192, false);
    if (tmp != NULL)
    {
      cnt = 0;
      ptr = buffer;
      while (max > 0)
      {
        num = MIN(8192, max);     
        _FileRead(f, tmp, 1, num, &amt);
        if (num != amt) goto FILE_LOAD_ABORT;
        
        _MemSemaphore(true);
        _MemMove(ptr, tmp, num);
        ptr += num; max -= num;
        _MemSemaphore(false);
  
        cnt += amt;
      }
    
FILE_LOAD_ABORT:

      _MemPtrFree(tmp);

      result = (cnt == size);
    }
     
    _FileClose(f);
  }
  
  return result;
}

/********************************* EOF ***********************************/
