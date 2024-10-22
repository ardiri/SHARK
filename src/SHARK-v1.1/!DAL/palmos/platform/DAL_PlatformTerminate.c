/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformTerminate.c
 */

#include "../DAL-prv.h"

void
DAL_PlatformTerminate()
{
  PALM_PlatformInfo *platform;
  UInt16             count;
  GLOBALS_ACCESS;

  // get access to the platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL) 
  { 
    // close and delete the preferences database if needed
    if (platform->preferences.dbRef != NULL)
    {
      count = DmNumResources(platform->preferences.dbRef);
      DmCloseDatabase(platform->preferences.dbRef);

      // if there are no resources in this database, zap it
      if (count == 0)
        DmDeleteDatabase(0, platform->preferences.dbID);
    }
    platform->preferences.dbRef = NULL;
    platform->preferences.dbID  = 0;
  }
}

/********************************* EOF ***********************************/
