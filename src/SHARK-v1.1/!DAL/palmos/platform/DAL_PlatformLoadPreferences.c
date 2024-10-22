/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformLoadPreferences.c
 */

#include "../DAL-prv.h"

void
DAL_PlatformLoadPreferences()
{
  void   *prefs;
  UInt16  prefSize, prefSizeX;
  UInt32  appid;
  Int16   prefFlag;                                 // preferences variables

  // read the saved preferences/saved state information
  prefs     = (void *)ApplicationGetPreferences();
  prefSizeX = ApplicationGetPreferencesSize();
  if ((prefSizeX != 0) && (prefs != NULL))
  {
    appid    = ApplicationGetUniqueID();
    prefSize = prefSizeX;
    prefFlag = PrefGetAppPreferences(appid, 0, prefs, &prefSize, true);

    // no preferences
    if ((prefFlag == noPreferenceFound) || (prefSize != prefSizeX))
      _MemSet(prefs, prefSizeX, 0); 
  }
}

/********************************* EOF ***********************************/
