/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformSavePreference.c
 */

#include "../DAL-prv.h"

void
DAL_PlatformSavePreference(uint16 index, void *buf, uint16 size)
{
  PALM_PlatformInfo *platform;
  uint16             _index;
  MemHandle          memHandle;
  void              *p;
  GLOBALS_ACCESS;
 
  // get access to the platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL) 
  { 
    // remove previous if it exists, we are going to zap it
    _index = DmFindResource(platform->preferences.dbRef, 0x64617461, index, NULL);   // 'data' = 0x64617461
    if (_index != 0xffff) DmRemoveResource(platform->preferences.dbRef, _index);

    // allocate new handle and copy over information 
    memHandle = DmNewResource(platform->preferences.dbRef, 0x64617461, index, size); // 'data' = 0x64617461
    if (memHandle != NULL)
    {
      p = (void *)MemHandleLock(memHandle);
      DmWrite(p, 0, buf, size);
      MemHandleUnlock(memHandle);
    }
  }
}

/********************************* EOF ***********************************/
