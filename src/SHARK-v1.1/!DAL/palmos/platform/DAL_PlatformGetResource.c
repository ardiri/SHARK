/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformGetResource.c
 */

#include "../DAL-prv.h"

void *
DAL_PlatformGetResource(uint16 resNum)
{
  PALM_PlatformInfo *platform;
  MemHandle memHandle;
  int8      filename[256];
  uint16    volRef;
  uint32    volIterator, version;
  FileRef   f;
  uint32    size;
  Err       err;
  int8      hex[] = "0123456789abcdef";
  int8      num[5];
  Boolean   found;
  void     *result = NULL;
  GLOBALS_ACCESS;

  // get access to the platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // 
    // MODE#1   application resource
    //
  
    memHandle = DmGetResource(0x5F646174, resNum); // '_dat' = 0x5F646174
    if (memHandle != NULL) result = (void *)MemHandleLock(memHandle);
  
    // 
    // MODE#2   ./xxxx{resNum}.bin (relative directory)
    //
  
    if (result == NULL) 
    {
      err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version);
      if (err == errNone)
      {
        // generate the filename
        num[0] = hex[(resNum >> 12) & 0x0f];
        num[1] = hex[(resNum >>  8) & 0x0f];
        num[2] = hex[(resNum >>  4) & 0x0f];
        num[3] = hex[ resNum        & 0x0f];
        num[4] = 0;
        _StrCopy(filename, "/Palm/Launcher/xxxx");      
        _StrCat(filename, num);
        _StrCat(filename, ".bin");
    
        found = false;
        volIterator = vfsIteratorStart;
        while ((volIterator != vfsIteratorStop) && (!found))
        {
          err = VFSVolumeEnumerate(&volRef, &volIterator);
          if (err == errNone)
          {
            // look for the file on this volume.
            err = VFSFileOpen(volRef, filename, vfsModeRead, &f);
    
            // did we find the resource?
            if (err == errNone)
            {
              void *buf;
  
              // allocate buffer large enough for it, and, read it in
              VFSFileSeek(f, vfsOriginEnd,       (uint32)0);
              VFSFileTell(f, &size);
              VFSFileSeek(f, vfsOriginBeginning, (uint32)0);
  
              buf = _MemPtrNew(size, false);
              if (buf != NULL)
              {
                uint16 _index;
  
                VFSFileRead(f, size, (void *)buf, NULL);

                // remove previous (if exists, it shouldn't - but, never know)
                _index = DmFindResource(platform->memory.dbRef, 0x5F646174, resNum, NULL);
                if (_index != 0xffff) DmRemoveResource(platform->memory.dbRef, _index);
  
                memHandle = DmNewResource(platform->memory.dbRef, 0x5F646174, resNum, size); // '_dat' = 0x5F646174
                if (memHandle != NULL)
                {
                  result = (void *)MemHandleLock(memHandle);
                  if (DmWrite(result, 0, buf, size) != errNone) result = NULL;
                }
                _MemPtrFree(buf);
              }
  
              VFSFileClose(f);
              found = true;
            }
          }
          else
            volIterator = vfsIteratorStop;
        }
      }
    }
  
    // 
    // MODE#3   /Common/xxxx{resNum}.bin
    //
  
    if (result == NULL) 
    {
      err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version);
      if (err == errNone)
      {
        // generate the filename
        num[0] = hex[(resNum >> 12) & 0x0f];
        num[1] = hex[(resNum >>  8) & 0x0f];
        num[2] = hex[(resNum >>  4) & 0x0f];
        num[3] = hex[ resNum        & 0x0f];
        num[4] = 0;
        _StrCopy(filename, "/Common/xxxx");      
        _StrCat(filename, num);
        _StrCat(filename, ".bin");
    
        // loop through all the mounted volumes.
        found = false;
        volIterator = vfsIteratorStart;
        while ((volIterator != vfsIteratorStop) && (!found))
        {
          err = VFSVolumeEnumerate(&volRef, &volIterator);
          if (err == errNone)
          {
            // look for the file on this volume.
            err = VFSFileOpen(volRef, filename, vfsModeRead, &f);
    
            // did we find the resource?
            if (err == errNone)
            {
              void *buf;
  
              // allocate buffer large enough for it, and, read it in
              VFSFileSeek(f, vfsOriginEnd,       (uint32)0);
              VFSFileTell(f, &size);
              VFSFileSeek(f, vfsOriginBeginning, (uint32)0);
  
              buf = _MemPtrNew(size, false);
              if (buf != NULL)
              {
                uint16 _index;
  
                VFSFileRead(f, size, (void *)buf, NULL);

                // remove previous (if exists, it shouldn't - but, never know)
                _index = DmFindResource(platform->memory.dbRef, 0x5F646174, resNum, NULL);
                if (_index != 0xffff) DmRemoveResource(platform->memory.dbRef, _index);
  
                memHandle = DmNewResource(platform->memory.dbRef, 0x5F646174, resNum, size); // '_dat' = 0x5F646174
                if (memHandle != NULL)
                {
                  result = (void *)MemHandleLock(memHandle);
                  if (DmWrite(result, 0, buf, size) != errNone) result = NULL;
                }
                _MemPtrFree(buf);
              }
  
              VFSFileClose(f);
              found = true;
            }
          }
          else
            volIterator = vfsIteratorStop;
        }
      }
    }
  
    // 
    // MODE#4   /Program/{application}/xxxx{resNum}.bin     (external memory card)
    //
  
    if (result == NULL) 
    {
      err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version);
      if (err == errNone)
      {
        // generate the filename
        num[0] = hex[(resNum >> 12) & 0x0f];
        num[1] = hex[(resNum >>  8) & 0x0f];
        num[2] = hex[(resNum >>  4) & 0x0f];
        num[3] = hex[ resNum        & 0x0f];
        num[4] = 0;
        _StrCopy(filename, "/Program/");      
        _StrCat(filename, ApplicationGetName());
        _StrCat(filename, "/xxxx");
        _StrCat(filename, num);
        _StrCat(filename, ".bin");
    
        // loop through all the mounted volumes.
        found = false;
        volIterator = vfsIteratorStart;
        while ((volIterator != vfsIteratorStop) && (!found))
        {
          err = VFSVolumeEnumerate(&volRef, &volIterator);
          if (err == errNone)
          {
            // look for the file on this volume.
            err = VFSFileOpen(volRef, filename, vfsModeRead, &f);
    
            // did we find the resource?
            if (err == errNone)
            {
              void *buf;
  
              // allocate buffer large enough for it, and, read it in
              VFSFileSeek(f, vfsOriginEnd,       (uint32)0);
              VFSFileTell(f, &size);
              VFSFileSeek(f, vfsOriginBeginning, (uint32)0);
  
              buf = _MemPtrNew(size, false);
              if (buf != NULL)
              {
                uint16 _index;
  
                VFSFileRead(f, size, (void *)buf, NULL);

                // remove previous (if exists, it shouldn't - but, never know)
                _index = DmFindResource(platform->memory.dbRef, 0x5F646174, resNum, NULL);
                if (_index != 0xffff) DmRemoveResource(platform->memory.dbRef, _index);
  
                memHandle = DmNewResource(platform->memory.dbRef, 0x5F646174, resNum, size); // '_dat' = 0x5F646174
                if (memHandle != NULL)
                {
                  result = (void *)MemHandleLock(memHandle);
                  if (DmWrite(result, 0, buf, size) != errNone) result = NULL;
                }
                _MemPtrFree(buf);
              }
  
              VFSFileClose(f);
              found = true;
            }
          }
          else
            volIterator = vfsIteratorStop;
        }
      }
    }
  }

  return result;
}

/********************************* EOF ***********************************/
