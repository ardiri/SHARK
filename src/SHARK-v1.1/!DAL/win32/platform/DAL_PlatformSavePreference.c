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
  UINT   prefSize;
  void  *prefs;
#ifdef PLT_REGISTRY_PREFERENCES
  HKEY   hKey;
  DWORD  status, _index, _size;
  uint8  keyName[64], valName[8];
  uint8 *ptr;
#else
  FILE  *fpPrefs;
  uint8  fileName[64];
#endif

  // write the saved preferences/saved state information
  prefSize = size;
  if (prefSize != 0)
  {
    prefs   = (void *)buf;

#ifdef PLT_REGISTRY_PREFERENCES
    sprintf(keyName, "Software\\Mobile Wizardry\\%s\\Preferences\\SLOT-%d", ApplicationGetName(), index);
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
      ptr    = (uint8 *)prefs;
      _index = 0;
      while (prefSize > 0)
      {
        valName[4] = (uint8)('0' + _index);
        _size      = MIN(PLT_REGISTRY_CHUNK_SIZE, prefSize);
        RegSetValueEx(hKey, valName, 0, REG_BINARY, ptr, _size);
        ptr += _size; prefSize -= _size; _index++;
      }
    }

    // close the registry entry
    RegCloseKey(hKey);
#else
    sprintf(fileName, "%s-preferences-SLOT-%d.dat", ApplicationGetName(), index);
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
