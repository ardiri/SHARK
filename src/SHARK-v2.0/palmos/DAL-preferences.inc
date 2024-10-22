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

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct
{
  LocalID   pref_dbID;
  DmOpenRef pref_dbRef; 

  void     *prefs;
} PrefGlobals;

#define g_pref_dbID            ((PrefGlobals *)g -> pref) -> pref_dbID
#define g_pref_dbRef           ((PrefGlobals *)g -> pref) -> pref_dbRef

#define g_prefs                ((PrefGlobals *)g -> pref) -> prefs

#define GLOBALS_UNINITIALIZED  (g -> pref == NULL)

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

// initialize the preferences engine
static boolean
_PreferencesInitialize()
{
  boolean      init_ok;
  char         dbName[32];
  PrefGlobals *g_lib;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _PreferencesInitialize");
#endif

  // allocate memory for memory globals
  g_lib = (PrefGlobals *)malloc(sizeof(PrefGlobals));
  if (g_lib != NULL) g -> pref = (void *)g_lib;

  // assume everything is ok
  init_ok = true;
  init_ok &= (g_lib != NULL);

  if (init_ok)
  {
    memset(g_lib, 0, sizeof(PrefGlobals));

    // defaults
    g_prefs = NULL;

    // initialize the memory we need for the preferences
    if (g_config.preferences.size > 0)
    {
      g_prefs = malloc(g_config.preferences.size);
      init_ok = (g_prefs != NULL); if (!init_ok) goto PREF_INIT_ABORT;
      memset(g_prefs, 0, g_config.preferences.size);
    }

    // create and open the preferences cache resource database
     strcpy(dbName, "[_mw_]-");
    strncat(dbName, g_config.name, 25);

    g_pref_dbID = DmFindDatabase(dbName);
    if (g_pref_dbID == 0)
      DmCreateDatabase(dbName, PalmOSGetUniqueID(), 'pref', true);
    g_pref_dbID = DmFindDatabase(dbName);
    init_ok = (g_pref_dbID != 0); if (!init_ok) goto PREF_INIT_ABORT;

    // open the database
    g_pref_dbRef = DmOpenDatabase(g_pref_dbID, dmModeReadWrite);
    init_ok = (g_pref_dbRef != NULL); if (!init_ok) goto PREF_INIT_ABORT;
  }

PREF_INIT_ABORT:

  return init_ok;
}

// get a pointer to the preferences structure
void *
_PreferencesGetReference()
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return NULL;

  return (void *)g_prefs;
}

// load the application preferences
void
_PreferencesLoad()
{
  UInt16 prefSize;
  UInt32 appid;
  Int16  prefFlag;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // read the application preferences 
  prefSize = g_config.preferences.size;
  if ((prefSize != 0) && (g_prefs != NULL))
  {
    appid    = PalmOSGetUniqueID();
    prefFlag = PrefGetAppPreferences(appid, 0, g_prefs, &prefSize, true);

    // no preferences (or, they are a different size)
    if ((prefFlag == noPreferenceFound) || 
        (prefSize != g_config.preferences.size))
      memset(g_prefs, 0, g_config.preferences.size); 
  }
}

// save the application preferences
void
_PreferencesSave()
{
  UInt16 prefSize;
  UInt32 appid;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // write the application preferences 
  prefSize = g_config.preferences.size;
  if ((g_prefs != NULL) && (prefSize != 0))
  {
    appid = PalmOSGetUniqueID();
    PrefSetAppPreferences(appid, 0, 0, g_prefs, prefSize, true);
  }
}

// load a specific preference bank
boolean
_PreferencesLoadBank(preference idx, void *buf, uint16 size)
{
  MemHandle memHandle;
  UInt16    index;
  void     *p;
  boolean   result;
 
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return false;

  // default return value
  result = false;

  // did we find the resource?
  index = DmFindResource(g_pref_dbRef, 'data', idx, NULL);
  if (index != dmMaxRecordIndex)
  {
    // grab the resource, and check its size
    memHandle = DmGetResourceIndex(g_pref_dbRef, index);
    if (MemHandleSize(memHandle) == size)
    {
      // correct size? copy it over to buffer
      p = (void *)MemHandleLock(memHandle);
      memcpy(buf, p, size);
      MemHandleUnlock(memHandle);

      result = true;
    }
    DmReleaseResource(memHandle);
  }

  // failure?
  if (!result) memset(buf, 0, size);

  return result;
}

// save a specific preference bank
void
_PreferencesSaveBank(preference idx, void *buf, uint16 size)
{
  MemHandle memHandle;
  UInt16    index;
  void     *p;
 
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // remove previous if it exists, we are going to zap it
  index = DmFindResource(g_pref_dbRef, 'data', idx, NULL);
  if (index != dmMaxRecordIndex) DmRemoveResource(g_pref_dbRef, index);

  // allocate new handle and copy over information 
  memHandle = DmNewResource(g_pref_dbRef, 'data', idx, size);
  if (memHandle != NULL)
  {
    p = (void *)MemHandleLock(memHandle);
    DmWrite(p, 0, buf, size);
    MemHandleUnlock(memHandle);

    DmReleaseResource(memHandle);
  }
}

// delete/remove a specific preference bank
void
_PreferencesKillBank(preference idx)
{
  UInt16 index;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // remove previous if it exists, we are going to zap it
  index = DmFindResource(g_pref_dbRef, 'data', idx, NULL);
  if (index != dmMaxRecordIndex) DmRemoveResource(g_pref_dbRef, index);
}

// terminate the preferences engine
static void
_PreferencesTerminate()
{
  UInt16 count;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _PreferencesTerminate");
#endif

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // close and delete the preferences database if needed
  if (g_pref_dbRef != NULL)
  {
    count = DmNumResources(g_pref_dbRef);
    DmCloseDatabase(g_pref_dbRef);

    // if there are no resources in this database, zap it
    if (count == 0) DmDeleteDatabase(g_pref_dbID);

    g_pref_dbRef = NULL;
    g_pref_dbID  = 0;
  }

  // clean up
  if (g_prefs != NULL) free(g_prefs);
  g_prefs = NULL;

  // free memory used by library globals
  if (g -> pref != NULL) free(g -> pref);
  g -> pref = NULL;
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-preferences.inc"

#undef GLOBALS_UNINITIALIZED

/********************************* EOF ***********************************/
