/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_FindFile.c
 */

#include "../DAL-prv.h"

file *
DAL_FindFile(uint8 *fileName)
{
  _file  *f;
  Err     err;
  UInt16  volRef;
  UInt32  volIterator, version;
  boolean found;

  // default return values
  found = false;

  f = (_file *)_MemPtrNew(sizeof(_file), false);
  f->location = FILE_UNKNOWN;
  f->fp       = NULL;

  //
  // MODE#1   RAM (FileStreaming API)
  //

  if (!found)
  {
    _StrCopy(f->fileName, fileName);

    f->fp = FileOpen(0, f->fileName, 0, 0, fileModeReadOnly, &err);
    if ((f->fp != NULL) && (err == errNone))
    {
      found = true;
      f->location = FILE_RAM;
      FileClose(f->fp);
    }
  }

  // 
  // MODE#2   VFS /Common/xxxx{resNum}.bin     (external memory card)
  //

  if (!found)
  {
    err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version);
    if (err == errNone)
    {
      // generate the filename
      _StrCopy(f->fileName, "/Common/");
      _StrCat(f->fileName, fileName);
  
      // loop through all the mounted volumes.
      volIterator = vfsIteratorStart;
      while ((volIterator != vfsIteratorStop) && (f->location == FILE_UNKNOWN))
      {
        err = VFSVolumeEnumerate(&volRef, &volIterator);
        if (err == errNone)
        {
          // look for the database on this volume.
          err = VFSFileOpen(volRef, f->fileName, vfsModeRead, (FileRef *)&f->fp);
  
          // did we find the resource?
          if (err == errNone)
          {
            found = true;
            f->location = FILE_CARD;
            VFSFileClose((FileRef)f->fp); 
            f->fp = NULL;
          }
        }
        else
          volIterator = vfsIteratorStop;
      }
    }
  }

  // 
  // MODE#3   /Program/{application}/xxxx{resNum}.bin     (external memory card)
  //

  if (!found)
  {
    err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version);
    if (err == errNone)
    {
      // generate the filename
      _StrCopy(f->fileName, "/Program/");
      _StrCat(f->fileName, ApplicationGetName());
      _StrCat(f->fileName, "/");
      _StrCat(f->fileName, fileName);
  
      // loop through all the mounted volumes.
      volIterator = vfsIteratorStart;
      while ((volIterator != vfsIteratorStop) && (f->location == FILE_UNKNOWN))
      {
        err = VFSVolumeEnumerate(&volRef, &volIterator);
        if (err == errNone)
        {
          // look for the database on this volume.
          err = VFSFileOpen(volRef, f->fileName, vfsModeRead, (FileRef *)&f->fp);
  
          // did we find the resource?
          if (err == errNone)
          {
            found = true;
            f->location = FILE_CARD;
            VFSFileClose((FileRef)f->fp); 
            f->fp = NULL;
          }
        }
        else
          volIterator = vfsIteratorStop;
      }
    }
  }

  // if we didn't find a file, lets bail out
  if (!found)
  {
    _MemPtrFree(f);
    f = NULL;
  }

  return (file *)f;
}

/********************************* EOF ***********************************/
