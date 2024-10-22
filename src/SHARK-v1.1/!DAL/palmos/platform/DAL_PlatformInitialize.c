/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformInitialize.c
 */

#include "../DAL-prv.h"

boolean
DAL_PlatformInitialize()
{
  PALM_PlatformInfo *platform;
  boolean            init_ok;
  uint8              dbName[32];
  GLOBALS_ACCESS;

  // assume everything is ok
  init_ok = true;

  // get access to the platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  init_ok = (platform != NULL); if (!init_ok) goto PLT_INIT_ABORT;

  // create and open the cache database for DAL memory
  _StrCopy(dbName, "[_mw_]-");
  _StrNCat(dbName, ApplicationGetName(), 25);

  // does it already exist, if not - create it
  platform->preferences.dbID = DmFindDatabase(0, dbName);
  if (platform->preferences.dbID == 0)
    DmCreateDatabase(0, dbName, ApplicationGetUniqueID(), 0x70726566, true); // 'pref' = 0x70726566
  platform->preferences.dbID = DmFindDatabase(0, dbName);
  init_ok = (platform->preferences.dbID != 0); if (!init_ok) goto PLT_INIT_ABORT;

  // open the database for future use
  platform->preferences.dbRef = DmOpenDatabase(0, platform->preferences.dbID, dmModeWrite);
  init_ok = (platform->preferences.dbRef != NULL); if (!init_ok) goto PLT_INIT_ABORT;

  goto PLT_INIT_SUCCESS;

PLT_INIT_ABORT:

  ;

PLT_INIT_SUCCESS:

  return init_ok;
}

/********************************* EOF ***********************************/
