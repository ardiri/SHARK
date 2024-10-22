/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformSavePreferences.c
 */

#include "../DAL-prv.h"

void
DAL_PlatformSavePreferences()
{
  void   *prefs;
  UInt16  prefSize;
  UInt32  appid;

  // write the saved preferences/saved state information
  prefs    = (void *)ApplicationGetPreferences();
  prefSize = ApplicationGetPreferencesSize();
  if ((prefSize != 0) && (prefs != NULL))
  {
    appid = ApplicationGetUniqueID();
    PrefSetAppPreferences(appid, 0, 0, prefs, prefSize, true);
  }
}

/********************************* EOF ***********************************/
