/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformLoadPreference.c
 */

#include "../DAL-prv.h"

boolean
DAL_PlatformLoadPreference(uint16 index, void *buf, uint16 size)
{
  UINT    prefSize;
  void   *prefs;
#ifdef PLT_REGISTRY_PREFERENCES
  HKEY    hKey;
  DWORD   keyType, status, _size;
  uint8   keyName[64];
  uint16 *keyNameU;
#else
  FILE   *fpPrefs;
  uint8   fileName[64];
#endif
  boolean result;

  // read the saved preferences/saved state information
  prefSize = size;
  if (prefSize != 0)
  {
    prefs   = (void *)buf;

#ifdef PLT_REGISTRY_PREFERENCES
    sprintf(keyName, "Software\\Mobile Wizardry\\%s\\Preferences\\SLOT-%d", ApplicationGetName(), index);
    keyNameU = DAL_SysCreateUnicodeString(keyName);

    // lets try and open the registry entry
    RegCreateKeyEx(HKEY_CURRENT_USER, keyNameU, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &status);
    if (status == REG_CREATED_NEW_KEY)
      _MemSet(prefs, (uint16)prefSize, 0);
    else
    {
      // lets get information on the DATA key
      status = RegQueryValueEx(hKey, _T("DATA"), 0, &keyType, NULL, &_size);

      // key exists, right type and right size?
      if ((status == ERROR_SUCCESS) && (keyType == REG_BINARY) && (prefSize == _size))
      {
        RegQueryValueEx(hKey, _T("DATA"), 0, &keyType, prefs, &_size);

        // mark it as successful
        result = true;
      }

      // we better clean up, reset preferences
      else
      {
        RegDeleteKey(HKEY_CURRENT_USER, keyNameU);
        _MemSet(prefs, (uint16)prefSize, 0);
      }
    }
    if (hKey != NULL) RegCloseKey(hKey);

    _MemPtrFree(keyNameU);
#else
    sprintf(fileName, "%s-preferences-SLOT-%d.dat", ApplicationGetName(), index);
    fpPrefs = fopen(fileName, "rb");
    if (fpPrefs == NULL)
      _MemSet(prefs, prefSize, 0);
    else
    {
      fread(prefs, prefSize, 1, fpPrefs);
      fclose(fpPrefs);

      // mark it as successful
      result = true;
    }
#endif
  }

  return result;
}

/********************************* EOF ***********************************/
