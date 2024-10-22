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
  DWORD  status;
  uint8  keyName[64];
  uint16 *keyNameU;
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
    keyNameU = DAL_SysCreateUnicodeString(keyName);

    // open the registry entry
    RegCreateKeyEx(HKEY_CURRENT_USER, keyNameU, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &status);

    // write to the registry value
    RegSetValueEx(hKey, _T("DATA"), 0, REG_BINARY, prefs, prefSize);

    // close the registry entry
    RegCloseKey(hKey);

    _MemPtrFree(keyNameU);
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
