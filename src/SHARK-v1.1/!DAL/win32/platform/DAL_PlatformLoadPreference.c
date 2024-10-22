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
  UINT   prefSize;
  void  *prefs;
#ifdef PLT_REGISTRY_PREFERENCES
  HKEY   hKey;
  DWORD  keyType, status, _size, _sizeX, _index, cnt;
  uint8  keyName[64], valName[8];
  uint8 *ptr;
  UINT   success;
#else
  FILE  *fpPrefs;
  uint8  fileName[64];
#endif
  boolean result;

  // assume failure
  result = false;

  // read the saved preferences/saved state information
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
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/sysinfo/base/registry_element_sizeX_limits.asp

    // lets try and open the registry entry
    RegCreateKeyEx(HKEY_CURRENT_USER, keyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &status);
    if (status == REG_CREATED_NEW_KEY)
      _MemSet(prefs, (uint16)prefSize, 0);
    else
    {
      success = true;

      // read from the registry value (single chunk)
      if (prefSize < PLT_REGISTRY_CHUNK_SIZE)
      {
        // lets get information on the value
        status   = RegQueryValueEx(hKey, valName, 0, &keyType, NULL, &_size);
        success &= ((status == ERROR_SUCCESS) && (keyType == REG_BINARY) && (prefSize == _size));

        // read in the value based on the success state
        if (success)
          RegQueryValueEx(hKey, valName, 0, &keyType, prefs, &_size);
      }

      // we need to re-construct it in order to save
      else
      {
        // lets get information on the value
        _size = 0;
        cnt   = ((prefSize+(PLT_REGISTRY_CHUNK_SIZE-1))/PLT_REGISTRY_CHUNK_SIZE);  // round upwards
        for (_index=0; _index<cnt; _index++)
        {
          valName[4] = (uint8)('0' + _index);
          status = RegQueryValueEx(hKey, valName, 0, &keyType, NULL, &_sizeX); _size += _sizeX;
          success &= ((status == ERROR_SUCCESS) && (keyType == REG_BINARY));
        }
        success &= (prefSize == _size);

        // read in the value based on the success state
        if (success)
        {
          ptr    = (uint8 *)prefs;
          _index = 0;
          while (_size > 0)
          {
            valName[4] = (uint8)('0' + _index);
            _sizeX      = MIN(PLT_REGISTRY_CHUNK_SIZE, _size);
            RegQueryValueEx(hKey, valName, 0, &keyType, ptr, &_sizeX);
            ptr += _sizeX; _size -= _sizeX; _index++;
          }
        }
      }

      // if something went wrong, clean up and reset preferences
      if (!success)
      {
        RegDeleteKey(HKEY_CURRENT_USER, keyName);
        _MemSet(prefs, (uint16)prefSize, 0);
      }

      // mark success based on result
      result = success;
    }
    if (hKey != NULL) RegCloseKey(hKey);
#else
    sprintf(fileName, "%s-preferences-SLOT-%d.dat", ApplicationGetName(), index);
    fpPrefs = fopen(fileName, "rb");
    if (fpPrefs == NULL)
      _MemSet(prefs, (uint16)prefSize, 0);
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
