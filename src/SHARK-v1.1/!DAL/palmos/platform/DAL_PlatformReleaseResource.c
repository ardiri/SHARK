/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformReleaseResource.c
 */

#include "../DAL-prv.h"

void
DAL_PlatformReleaseResource(void *ptr)
{
  PALM_PlatformInfo *platform;
  GLOBALS_ACCESS;

  // get access to the platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    if (ptr != NULL)
    {
      UInt16    _index;
      MemHandle memHandle;
  
      // recover the memhandle for this pointer
      memHandle = MemPtrRecoverHandle(ptr);
  
      // unlock and release resource
      _index = DmFindResource(platform->memory.dbRef, 0, 0, memHandle);
      MemHandleUnlock(memHandle);
      DmReleaseResource(memHandle);
  
      // remove it from cache database
      if (_index != 0xffff) DmRemoveResource(platform->memory.dbRef, _index);
    }
  }
}

/********************************* EOF ***********************************/
