/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_MemSemaphore.c
 */

#include "../DAL-prv.h"

void
DAL_MemSemaphore(boolean active)
{
#ifdef MEM_ALLOW_STORAGE
  uint32 romVersion;

  // get the rom version from the device
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

  // we can only do this on versions prior to palmos 6
  if (romVersion < sysMakeROMVersion(6,0,0,sysROMStageDevelopment,0))
  {
    if (active) MemSemaphoreReserve(true);
    else        MemSemaphoreRelease(true);
  }
#endif
}

/********************************* EOF ***********************************/
