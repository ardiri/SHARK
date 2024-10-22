/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformGetResourceSize.c
 */

#include "../DAL-prv.h"

uint16
DAL_PlatformGetResourceSize(uint16 resNum)
{
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
  uint16    result = 0;

  // 
  // MODE#1   application resource
  //

  memHandle = DmGetResource(0x5F646174, resNum); // '_dat' = 0x5F646174
  if (memHandle != NULL) result = MemHandleSize(memHandle);

  // 
  // MODE#2   ./xxxx{resNum}.bin (relative directory)
  //

  if (memHandle == NULL) 
  {
    err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version);
    if (err == errNone)
    {
      // generate the filename
//    StrPrintF(filename, "/Palm/Launcher/xxxx%04x.bin", resNum);
      num[0] = hex[(resNum >> 12) & 0x0f];
      num[1] = hex[(resNum >>  8) & 0x0f];
      num[2] = hex[(resNum >>  4) & 0x0f];
      num[3] = hex[ resNum        & 0x0f];
      num[4] = 0;
      _StrCopy(filename, "/Palm/Launcher/xxxx");      
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
            VFSFileSeek(f, vfsOriginEnd, (uint32)0);
            VFSFileTell(f, (uint32 *)&size);

            // return the size  
            result = (uint16)size;

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

  if (memHandle == NULL) 
  {
    err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version);
    if (err == errNone)
    {
      // generate the filename
//    StrPrintF(filename, "/Common/xxxx%04x.bin", resNum);
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
            VFSFileSeek(f, vfsOriginEnd, (uint32)0);
            VFSFileTell(f, (uint32 *)&size);

            // return the size  
            result = (uint16)size;

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

  if (memHandle == NULL) 
  {
    err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version);
    if (err == errNone)
    {
      // generate the filename
//    StrPrintF(filename, "/Program/%s/xxxx%04x.bin", ApplicationGetName(), resNum);
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
            VFSFileSeek(f, vfsOriginEnd, (uint32)0);
            VFSFileTell(f, (uint32 *)&size);

            // return the size  
            result = (uint16)size;

            VFSFileClose(f);
            found = true;
          }
        }
        else
          volIterator = vfsIteratorStop;
      }
    }
  }

  // validation - we cannot handle resources > 64000 bytes
  if (result > 64000)
    result = 0;

  return (uint16)(result & 0xffff);
}

/********************************* EOF ***********************************/
