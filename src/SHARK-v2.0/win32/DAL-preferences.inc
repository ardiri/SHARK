/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-preferences.c
 */

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

#define PLT_REGISTRY_CHUNK_SIZE  8192    // maximum chunk size (registry)

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

void *g_prefs;

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

// initialize the preferences engine
static boolean
_PreferencesInitialize()
{
  boolean     init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _PreferencesInitialize");
#endif

  // assume everything is ok
  init_ok = true;

  // defaults
  g_prefs = NULL;

  // initialize the memory we need for the preferences
  if (g_config.preferences.size > 0)
  {
    g_prefs = (void *)malloc(g_config.preferences.size);
    init_ok = (g_prefs != NULL); if (!init_ok) goto PREF_INIT_ABORT;
    memset(g_prefs, 0, g_config.preferences.size);
  }

PREF_INIT_ABORT:

  return init_ok;
}

// get a pointer to the preferences structure
void *
_PreferencesGetReference()
{
  return (void *)g_prefs;
}

// load the application preferences
void
_PreferencesLoad()
{
  UINT   prefSize;
  void  *prefs;
  HKEY   hKey;
  DWORD  keyType, status, size, _size, index, cnt;
  char   keyName[64], valName[8];
  char  *ptr;
  UINT   success;

  // read the saved preferences/saved state information
  prefSize = g_config.preferences.size;
  prefs    = _PreferencesGetReference();
  if ((prefSize != 0) && (prefs != NULL))
  {
    sprintf(keyName, "Software\\%s\\%s\\Preferences",
                     g_config.company, g_config.name);
    sprintf(valName, "DATA@");

// NOTE: win95/98 registry limitations
// >> max key size   = 64k
// >> max value size = 16382 on win95, 16371 on win98
//
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/sysinfo/base/registry_element_size_limits.asp

    // lets try and open the registry entry
    RegCreateKeyEx(HKEY_CURRENT_USER, keyName, 0, NULL,
                   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                   &hKey, &status);
    if (status == REG_CREATED_NEW_KEY)
      memset(prefs, 0, (uint16)prefSize);
    else
    {
      success = true;

      // read from the registry value (single chunk)
      if (prefSize < PLT_REGISTRY_CHUNK_SIZE)
      {
        // lets get information on the value
        status   = RegQueryValueEx(hKey, valName, 0, &keyType, NULL, &size);
        success &= ((status == ERROR_SUCCESS) &&
                    (keyType == REG_BINARY) && (prefSize == size));

        // read in the value based on the success state
        if (success)
          RegQueryValueEx(hKey, valName, 0, &keyType, prefs, &size);
      }

      // we need to re-construct it in order to save
      else
      {
        // lets get information on the value
        size = 0;
        cnt  = ((prefSize+(PLT_REGISTRY_CHUNK_SIZE-1))/PLT_REGISTRY_CHUNK_SIZE);
        for (index=0; index<cnt; index++)
        {
          valName[4] = (char)('0' + index);
          status = RegQueryValueEx(hKey, valName, 0, &keyType, NULL, &_size);
          success &= ((status == ERROR_SUCCESS) && (keyType == REG_BINARY));
          size += _size;
        }
        success &= (prefSize == size);

        // read in the value based on the success state
        if (success)
        {
          ptr   = (char *)prefs;
          index = 0;
          while (size > 0)
          {
            valName[4] = (char)('0' + index);
            _size      = MIN(PLT_REGISTRY_CHUNK_SIZE, size);
            RegQueryValueEx(hKey, valName, 0, &keyType, ptr, &_size);
            ptr += _size; size -= _size; index++;
          }
        }
      }

      // if something went wrong, clean up and reset preferences
      if (!success)
      {
        RegDeleteKey(HKEY_CURRENT_USER, keyName);
        memset(prefs, 0, (uint16)prefSize);
      }
    }
    if (hKey != NULL) RegCloseKey(hKey);
  }
}

// save the application preferences
void
_PreferencesSave()
{
  UINT   prefSize;
  void  *prefs;
  HKEY   hKey;
  DWORD  status, index, _size;
  char   keyName[64], valName[8];
  char  *ptr;

  // write the saved preferences/saved state information
  prefSize = g_config.preferences.size;
  prefs    = _PreferencesGetReference();
  if ((prefSize != 0) && (prefs != NULL))
  {
    sprintf(keyName, "Software\\%s\\%s\\Preferences",
                     g_config.company, g_config.name);
    sprintf(valName, "DATA@");

// NOTE: win95/98 registry limitations
// >> max key size   = 64k
// >> max value size = 16382 on win95, 16371 on win98
//
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/sysinfo/base/registry_element_size_limits.asp

    // open the registry entry
    RegCreateKeyEx(HKEY_CURRENT_USER, keyName, 0, NULL,
                   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                   &hKey, &status);

    // write to the registry value (single chunk)
    if (prefSize < PLT_REGISTRY_CHUNK_SIZE)
      RegSetValueEx(hKey, valName, 0, REG_BINARY, prefs, prefSize);

    // we need to break it up in order to save
    else
    {
      ptr   = (char *)prefs;
      index = 0;
      while (prefSize > 0)
      {
        valName[4] = (char)('0' + index);
        _size      = MIN(PLT_REGISTRY_CHUNK_SIZE, prefSize);
        RegSetValueEx(hKey, valName, 0, REG_BINARY, ptr, _size);
        ptr += _size; prefSize -= _size; index++;
      }
    }

    // close the registry entry
    RegCloseKey(hKey);
  }
}

// load a specific preference bank
boolean
_PreferencesLoadBank(preference idx, void *buf, uint16 size)
{
  UINT    prefSize;
  void   *prefs;
  HKEY    hKey;
  DWORD   keyType, status, _size, _sizeX, _index, cnt;
  char    keyName[64], valName[8];
  char   *ptr;
  UINT    success;
  boolean result;

  // assume failure
  result = false;

  // read the saved preferences/saved state information
  prefSize = size;
  prefs    = (void *)buf;
  if ((prefSize != 0) && (prefs != NULL))
  {
    sprintf(keyName, "Software\\%s\\%s\\Preferences\\SLOT-%d",
                     g_config.company, g_config.name, idx);
    sprintf(valName, "DATA@");

// NOTE: win95/98 registry limitations
// >> max key size   = 64k
// >> max value size = 16382 on win95, 16371 on win98
//
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/sysinfo/base/registry_element_sizeX_limits.asp

    // lets try and open the registry entry
    RegCreateKeyEx(HKEY_CURRENT_USER, keyName, 0, NULL,
                   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                   &hKey, &status);
    if (status == REG_CREATED_NEW_KEY)
      memset(prefs, 0, (uint16)prefSize);
    else
    {
      success = true;

      // read from the registry value (single chunk)
      if (prefSize < PLT_REGISTRY_CHUNK_SIZE)
      {
        // lets get information on the value
        status   = RegQueryValueEx(hKey, valName, 0, &keyType, NULL, &_size);
        success &= ((status == ERROR_SUCCESS) &&
                    (keyType == REG_BINARY) && (prefSize == _size));

        // read in the value based on the success state
        if (success)
          RegQueryValueEx(hKey, valName, 0, &keyType, prefs, &_size);
      }

      // we need to re-construct it in order to save
      else
      {
        // lets get information on the value
        _size = 0;
        cnt   = ((prefSize+(PLT_REGISTRY_CHUNK_SIZE-1))/PLT_REGISTRY_CHUNK_SIZE);
        for (_index=0; _index<cnt; _index++)
        {
          valName[4] = (char)('0' + _index);
          status = RegQueryValueEx(hKey, valName, 0, &keyType, NULL, &_sizeX);
          success &= ((status == ERROR_SUCCESS) && (keyType == REG_BINARY));
          _size += _sizeX;
        }
        success &= (prefSize == _size);

        // read in the value based on the success state
        if (success)
        {
          ptr    = (char *)prefs;
          _index = 0;
          while (_size > 0)
          {
            valName[4] = (char)('0' + _index);
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
        memset(prefs, 0, (uint16)prefSize);
      }

      // mark success based on result
      result = success;
    }
    if (hKey != NULL) RegCloseKey(hKey);
  }

  return result;
}

// save a specific preference bank
void
_PreferencesSaveBank(preference idx, void *buf, uint16 size)
{
  UINT   prefSize;
  void  *prefs;
  HKEY   hKey;
  DWORD  status, _index, _size;
  char   keyName[64], valName[8];
  char  *ptr;

  // write the saved preferences/saved state information
  prefSize = size;
  prefs    = (void *)buf;
  if ((prefSize != 0) && (prefs != NULL))
  {
    sprintf(keyName, "Software\\%s\\%s\\Preferences\\SLOT-%d",
                     g_config.company, g_config.name, idx);
    sprintf(valName, "DATA@");

// NOTE: win95/98 registry limitations
// >> max key size   = 64k
// >> max value size = 16382 on win95, 16371 on win98
//
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/sysinfo/base/registry_element_size_limits.asp

    // open the registry entry
    RegCreateKeyEx(HKEY_CURRENT_USER, keyName, 0, NULL,
                   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                   &hKey, &status);

    // write to the registry value (single chunk)
    if (prefSize < PLT_REGISTRY_CHUNK_SIZE)
      RegSetValueEx(hKey, valName, 0, REG_BINARY, prefs, prefSize);

    // we need to break it up in order to save
    else
    {
      ptr    = (char *)prefs;
      _index = 0;
      while (prefSize > 0)
      {
        valName[4] = (char)('0' + _index);
        _size      = MIN(PLT_REGISTRY_CHUNK_SIZE, prefSize);
        RegSetValueEx(hKey, valName, 0, REG_BINARY, ptr, _size);
        ptr += _size; prefSize -= _size; _index++;
      }
    }

    // close the registry entry
    RegCloseKey(hKey);
  }
}

// delete/remove a specific preference bank
void
_PreferencesKillBank(preference idx)
{
  char keyName[64];
  sprintf(keyName, "Software\\%s\\%s\\Preferences\\SLOT-%d",
                   g_config.company, g_config.name, idx);

  // delete the registry key
  RegDeleteKey(HKEY_CURRENT_USER, keyName);
}

// terminate the preferences engine
static void
_PreferencesTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _PreferencesTerminate");
#endif

  // clean up
  if (g_prefs != NULL) free(g_prefs);
  g_prefs = NULL;
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-preferences.inc"

/********************************* EOF ***********************************/
