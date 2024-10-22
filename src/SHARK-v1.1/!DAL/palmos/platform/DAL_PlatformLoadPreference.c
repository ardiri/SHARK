/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformLoadPreference.c
 */

#include "../DAL-prv.h"

boolean
DAL_PlatformLoadPreference(uint16 index, void *buf, uint16 size)
{
  PALM_PlatformInfo *platform;
  uint16             _index;
  MemHandle          memHandle;
  void              *p;
  boolean            result;
  GLOBALS_ACCESS;
 
  // default return value
  result = false;

  // get access to the platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL) 
  { 
    // did we find the resource?
    _index = DmFindResource(platform->preferences.dbRef, 0x64617461, index, NULL); // 'data' = 0x64617461
    if (_index != 0xffff)
    {
      // grab the resource, and check its size
      memHandle = DmGetResourceIndex(platform->preferences.dbRef, _index);
      if (MemHandleSize(memHandle) == size)
      {
        // correct size? copy it over to buffer
        p = (void *)MemHandleLock(memHandle);
        _MemMove(buf, p, size);
        MemHandleUnlock(memHandle);

        result = true;
      }
    }
  }

  // failure?
  if (!result) _MemSet(buf, size, 0);

  return result;
}

/********************************* EOF ***********************************/
