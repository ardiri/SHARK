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
  UINT   prefSize;
  void  *prefs;
#ifdef PLT_REGISTRY_PREFERENCES
  HKEY   hKey;
  DWORD  status, index, _size;
  uint8  keyName[64], valName[8];
  uint8 *ptr;
#else
  FILE  *fpPrefs;
  uint8  fileName[64];
#endif

  // write the saved preferences/saved state information
  prefSize = ApplicationGetPreferencesSize();
  if (prefSize != 0)
  {
    prefs   = (void *)ApplicationGetPreferences();

#ifdef PLT_REGISTRY_PREFERENCES
    sprintf(keyName, "Software\\Mobile Wizardry\\%s\\Preferences", ApplicationGetName());
    sprintf(valName, "DATA@");

// NOTE: win95/98 registry limitations
// >> max key size   = 64k
// >> max value size = 16382 on win95, 16371 on win98
//
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/sysinfo/base/registry_element_size_limits.asp

    // open the registry entry
    RegCreateKeyEx(HKEY_CURRENT_USER, keyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &status);

    // write to the registry value (single chunk)
    if (prefSize < PLT_REGISTRY_CHUNK_SIZE)
      RegSetValueEx(hKey, valName, 0, REG_BINARY, prefs, prefSize);

    // we need to break it up in order to save
    else
    {
      ptr   = (uint8 *)prefs;
      index = 0;
      while (prefSize > 0)
      {
        valName[4] = (uint8)('0' + index);
        _size      = MIN(PLT_REGISTRY_CHUNK_SIZE, prefSize);
        RegSetValueEx(hKey, valName, 0, REG_BINARY, ptr, _size);
        ptr += _size; prefSize -= _size; index++;
      }
    }

    // close the registry entry
    RegCloseKey(hKey);
#else
    sprintf(fileName, "%s-preferences.dat", ApplicationGetName());
    fpPrefs = fopen(fileName, "wb");
    if (fpPrefs != NULL)
    {
      fwrite(prefs, prefSize, 1, fpPrefs);
      fclose(fpPrefs);
    }
#endif
  }
}

/********************************* EOF ***********************************/
