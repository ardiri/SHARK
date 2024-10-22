/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-resource.c
 */

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

boolean
_ResourceExists_FILE(char *diskname, resource resNum)
{
  boolean result;
  uint32  disk;
  file   *f;
  char    name[MAX_FILENAME_LENGTH], id[9];

  // default return value
  result = false;

  // do we have a 'resource' drive?
  if (_FileLocateDisk(diskname, &disk))
  {
    _StrHToA(id, resNum);
    _StrCopy(name, "xxxx");
    _StrCat(name, &id[6]);
    _StrCat(name, ".bin");   // build xxxx0000.bin based file name

    // lets try and find the file
    f = _FileGetHandle(disk, NULL, name);
    if (f != NULL)
    {
      result = true;
      _FileReleaseHandle(f);
    }
  }

  return result;
}

// determine the size of a resource
uint16
_ResourceSize_FILE(char *diskname, resource resNum)
{
  uint16 result;
  uint32 disk;
  file  *f;
  char   name[MAX_FILENAME_LENGTH], id[9];

  // default return value
  result = 0;

  // do we have a 'resource' drive?
  if (_FileLocateDisk(diskname, &disk))
  {
    _StrHToA(id, resNum);
    _StrCopy(name, "xxxx");
    _StrCat(name, &id[6]);
    _StrCat(name, ".bin");   // build xxxx0000.bin based file name

    // lets try and find the file
    f = _FileGetHandle(disk, NULL, name);
    if (f != NULL)
    {
      if (_FileOpen(f, FILE_READ_ONLY))
      {
        _FileSeek(f, 0, file_end);
        result = (uint16)_FileTell(f);

        _FileClose(f);
      }
      _FileReleaseHandle(f);
    }
  }

  return result;
}

// lock down a resource
void *
_ResourceLock_FILE(char *diskname, resource resNum)
{
  void   *result;
  uint16  size;
  uint32  cnt;
  uint32  disk;
  file   *f;
  char    name[MAX_FILENAME_LENGTH], id[9];

  // default return value
  result = NULL;

  // do we have a 'resource' drive?
  if (_FileLocateDisk(diskname, &disk))
  {
    _StrHToA(id, resNum);
    _StrCopy(name, "xxxx");
    _StrCat(name, &id[6]);
    _StrCat(name, ".bin");   // build xxxx0000.bin based file name

    // lets try and find the file
    f = _FileGetHandle(disk, NULL, name);
    if (f != NULL)
    {
      if (_FileOpen(f, FILE_READ_ONLY))
      {
        _FileSeek(f, 0, file_end);
        size = (uint16)_FileTell(f);
        _FileSeek(f, 0, file_beginning);

        // allocate buffer (non dynamic) for it, and, read it in
        if (size != 0)
        {
          result = _MemPtrNew(size, true);
          if (result != NULL)
          {
            _MemSemaphore(true);
            _FileRead(f, result, 1, size, &cnt);
            _MemSemaphore(false);
          }
        }
        _FileClose(f);
      }
      _FileReleaseHandle(f);
    }
  }

  return result;
}

/********************************* EOF ***********************************/
