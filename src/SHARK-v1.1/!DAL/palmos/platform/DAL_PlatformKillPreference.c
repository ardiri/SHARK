/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformKillPreference.c
 */

#include "../DAL-prv.h"

void
DAL_PlatformKillPreference(uint16 index)
{
  PALM_PlatformInfo *platform;
  uint16             _index;
  GLOBALS_ACCESS;
 
  // get access to the platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL) 
  { 
    // remove resource from preferences
    _index = DmFindResource(platform->preferences.dbRef, 0x64617461, index, NULL);   // 'data' = 0x64617461
    if (_index != 0xffff) DmRemoveResource(platform->preferences.dbRef, _index);
  }
}

/********************************* EOF ***********************************/
