/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-platform.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 * Platform Implementation Structures
 *
 *************************************************************************/

#define FILENAME_SIZE 256

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

void
PlatformInitialize()
{
}

void
PlatformTerminate()
{
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

uint16
PlatformGetResourceSize(uint16 resNum)
{
  MemHandle memHandle;
  uint16    result = 0;

  // 
  // MODE#1   application resource
  //

  memHandle = DmGetResource(0x5F646174, resNum); // '_dat' = 0x5F646174
  if (memHandle != NULL) result = MemHandleSize(memHandle);

  // 
  // MODE#2   /Common/xxxx{resNum}.bin
  //

  if (memHandle == NULL) 
  {
    int8     filename[FILENAME_SIZE];
    uint16   volRef;
    uint32   volIterator, version;
    FileRef  f;
    uint32   size;
    Err      err;
    Boolean  found;

    err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version);
    if (err == errNone)
    {
      int8 hex[] = "0123456789abcdef";
      int8 num[5];

      // generate the filename
//    StrPrintF(filename, "/Common/xxxx%04x.bin", resNum);
      num[0] = hex[(resNum >> 12) & 0x0f];
      num[1] = hex[(resNum >>  8) & 0x0f];
      num[2] = hex[(resNum >>  4) & 0x0f];
      num[3] = hex[ resNum        & 0x0f];
      num[4] = 0;
      StrCopy(filename, "/Common/xxxx");      
      StrCat(filename, num);
      StrCat(filename, ".bin");
  
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

  return result;
}

void *
PlatformGetResource(uint16 resNum)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  MemHandle  memHandle;
  void      *result = NULL;

  // 
  // MODE#1   application resource
  //

  memHandle = DmGetResource(0x5F646174, resNum); // '_dat' = 0x5F646174
  if (memHandle != NULL) result = (void *)MemHandleLock(memHandle);

  // 
  // MODE#2   /Common/xxxx{resNum}.bin
  //

  if (memHandle == NULL) 
  {
    int8    filename[FILENAME_SIZE];
    uint16  volRef;
    uint32  volIterator, version;
    FileRef f;
    uint32  size;
    Err     err;
    Boolean found;

    err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version);
    if (err == errNone)
    {
      int8 hex[] = "0123456789abcdef";
      int8 num[5];

      // generate the filename
//    StrPrintF(filename, "/Common/xxxx%04x.bin", resNum);
      num[0] = hex[(resNum >> 12) & 0x0f];
      num[1] = hex[(resNum >>  8) & 0x0f];
      num[2] = hex[(resNum >>  4) & 0x0f];
      num[3] = hex[ resNum        & 0x0f];
      num[4] = 0;
      StrCopy(filename, "/Common/xxxx");      
      StrCat(filename, num);
      StrCat(filename, ".bin");

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

            buf = MemPtrNew(size);
            if (buf != NULL)
            {
              VFSFileRead(f, size, (void *)buf, NULL);

              memHandle = DmNewResource(gDALGlobals->mem.dbRef, 0x5F646174, resNum, size); // '_dat' = 0x5F646174
              if (memHandle != NULL)
              {
                result = (void *)MemHandleLock(memHandle);
                if (DmWrite(result, 0, buf, size) != errNone) result = NULL;  // sanity check for failure?
              }
              MemPtrFree(buf);
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

  return result;
}

void
PlatformReleaseResource(void *ptr)
{
  if (ptr != NULL)
  {
    MemHandle memHandle;

    memHandle = MemPtrRecoverHandle(ptr);
    MemHandleUnlock(memHandle);
    DmReleaseResource(memHandle);
  }
}

/********************************* EOF ***********************************/
