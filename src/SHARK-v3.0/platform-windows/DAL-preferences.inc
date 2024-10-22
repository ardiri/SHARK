/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-preferences.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

#define REGISTRY_CHUNK_SIZE  8192    // maximum chunk size (registry)

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define PREF_GLOBALS          g_pref
#define GLOBALS_UNINITIALIZED (PREF_GLOBALS == NULL)
#define GLOBALS_DEFINITION    PrefGlobals *g_pref = NULL

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-preferences.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
DAL_PreferencesInitialize()
{
  boolean init_ok;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(PREF_GLOBALS, 0, sizeof(PrefGlobals));

  return init_ok;
}

void
DAL_PreferencesLoad()
{
  UINT   prefSize;
  void  *prefs;
  HKEY   hKey;
  DWORD  keyType, status, size, _size, index, cnt;
  char   keyName[96], valName[8];
  char  *ptr;
  UINT   success;

  // do we have any preferences?
  if (g_config.preferences.size > 0) 
  {
    // lets clear the preferences
    memset(g_pref_data, 0, g_config.preferences.size);

    sprintf(keyName, "Software\\%s\\%s\\Preferences",
                     g_config.company, g_config.name);
    sprintf(valName, "DATA@");

// NOTE: win95/98 registry limitations
// >> max key size   = 64k
// >> max value size = 16382 on win95, 16371 on win98
//
// http://msdn.microsoft.com/library/default.asp?
//    url=/library/en-us/sysinfo/base/registry_element_size_limits.asp

    // lets try and open the registry entry
    RegCreateKeyEx(HKEY_CURRENT_USER, keyName, 0, NULL,
                   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                   &hKey, &status);
    if ((hKey == NULL) || (status == REG_CREATED_NEW_KEY))
      success = false;
    else
    {
      success = true;

      // the key reference information
      prefSize = g_config.preferences.size;
      prefs    = g_pref_data;

      // read from the registry value (single chunk)
      if (prefSize < REGISTRY_CHUNK_SIZE)
      {
        // lets get information on the value
        status   = RegQueryValueEx(hKey, valName, 0, &keyType, NULL, &size);
        success &= ((status == ERROR_SUCCESS) && (prefSize >= size));

        // read in the value based on the success state
        if (success)
          RegQueryValueEx(hKey, valName, 0, &keyType, prefs, &size);
      }

      // we need to re-construct it in order to save
      else
      {
        // lets get information on the value
        size = 0;
        cnt  = ((prefSize+(REGISTRY_CHUNK_SIZE-1))/REGISTRY_CHUNK_SIZE);
        for (index=0; index<cnt; index++)
        {
          valName[4] = (char)('0' + index);
          status = RegQueryValueEx(hKey, valName, 0, &keyType, NULL, &_size);
          success &= (status == ERROR_SUCCESS);
          size += _size;
        }
        success &= (prefSize >= size);

        // read in the value based on the success state
        if (success)
        {
          ptr   = (char *)prefs;
          index = 0;
          while (size > 0)
          {
            valName[4] = (char)('0' + index);
            _size      = MIN(REGISTRY_CHUNK_SIZE, size);
            RegQueryValueEx(hKey, valName, 0, &keyType, ptr, &_size);
            ptr += _size; size -= _size; index++;
          }
        }
      }
    }
    if (hKey != NULL) RegCloseKey(hKey);

    // if something went wrong, clean up 
    if (!success)
      RegDeleteKey(HKEY_CURRENT_USER, keyName);
  }
}

void
DAL_PreferencesSave()
{
  UINT   prefSize;
  void  *prefs;
  HKEY   hKey;
  DWORD  status, index, _size;
  char   keyName[96], valName[8];
  char  *ptr;

  // do we have any preferences?
  if (g_config.preferences.size > 0)
  {
    sprintf(keyName, "Software\\%s\\%s\\Preferences",
                     g_config.company, g_config.name);
    sprintf(valName, "DATA@");

// NOTE: win95/98 registry limitations
// >> max key size   = 64k
// >> max value size = 16382 on win95, 16371 on win98
//
// http://msdn.microsoft.com/library/default.asp?
//    url=/library/en-us/sysinfo/base/registry_element_size_limits.asp

    // open the registry entry
    RegCreateKeyEx(HKEY_CURRENT_USER, keyName, 0, NULL,
                   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                   &hKey, &status);

    // the key reference information
    prefSize = g_config.preferences.size;
    prefs    = g_pref_data;

    // write to the registry value (single chunk)
    if (prefSize < REGISTRY_CHUNK_SIZE)
      RegSetValueEx(hKey, valName, 0, REG_BINARY, prefs, prefSize);

    // we need to break it up in order to save
    else
    {
      ptr   = (char *)prefs;
      index = 0;
      while (prefSize > 0)
      {
        valName[4] = (char)('0' + index);
        _size      = MIN(REGISTRY_CHUNK_SIZE, prefSize);
        RegSetValueEx(hKey, valName, 0, REG_BINARY, ptr, _size);
        ptr += _size; prefSize -= _size; index++;
      }
    }

    // close the registry entry
    RegCloseKey(hKey);
  }
}

void 
DAL_PreferencesTerminate()
{
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef PREF_GLOBALS

#undef REGISTRY_CHUNK_SIZE

/********************************* EOF ***********************************/
