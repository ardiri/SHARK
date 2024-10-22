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

void *g_prefs;
char  g_pref_dir[MAX_PATH_LENGTH];

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

// initialize the preferences engine
static boolean
_PreferencesInitialize()
{
  boolean init_ok;
  char    tmp[MAX_PATH_LENGTH];

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _PreferencesInitialize");
#endif

  // assume everything is ok
  init_ok = true;

  // defaults
  g_prefs = NULL;

  // ensure the preference directory actually exists
  sprintf(g_pref_dir, "%s/Library/Preferences/%s/%s", 
                      getenv("HOME"), g_config.company, g_config.name);
  init_ok = (chdir(g_pref_dir) == 0);
  if (!init_ok)
  {
    sprintf(tmp, "%s//Library/Preferences/%s", 
                 getenv("HOME"), g_config.company);
    if (chdir(tmp) != 0) mkdir(tmp);

    init_ok = (mkdir(g_pref_dir) == 0);
  }
  if (!init_ok) goto PREF_INIT_ABORT;

  // initialize the memory we need for the preferences
  if (g_config.preferences.size > 0)
  {
    g_prefs = (void *)_MemPtrNew(g_config.preferences.size, false);
    init_ok = (g_prefs != NULL); if (!init_ok) goto PREF_INIT_ABORT;
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
  FILE   *f;
  uint32  cnt;
  char    name[MAX_FILENAME_LENGTH];

  // entry requirements
  if (g_prefs == NULL) return;

  // generate the preference filename
  sprintf(name, "%s\\preferences.dat", g_pref_dir);

  // lets try and open the file (create it)
  f = fopen(name, "rb");
  if (f != NULL)
  {
    fseek(f, 0, SEEK_END);
    cnt = ftell(f);
    fseek(f, 0, SEEK_SET);

    // clear the existing preference set
    memset(g_prefs, 0, g_config.preferences.size);

    // read appropriate number of bytes in
    cnt = MIN(cnt, g_config.preferences.size);
    fread(g_prefs, 1, cnt, f);

    fclose(f);
  }
}

// save the application preferences
void
_PreferencesSave()
{
  FILE   *f;
  uint32  cnt;
  char    name[MAX_FILENAME_LENGTH];

  // entry requirements
  if (g_prefs == NULL) return;

  // generate the preference filename
  sprintf(name, "%s\\preferences.dat", g_pref_dir);

  // lets try and find the file
  f = fopen(name, "wb");
  if (f != NULL)
  {
    // lets just dump our preferences
    cnt = g_config.preferences.size;

    // write appropriate number of bytes out
    fwrite(g_prefs, 1, cnt, f);

    fclose(f);
  }
}

// load a specific preference bank
boolean
_PreferencesLoadBank(preference idx, void *buf, uint16 size)
{
  boolean result;
  FILE   *f;
  uint32  cnt;
  char    name[MAX_FILENAME_LENGTH];

  // assume failure
  result = false;

  // entry requirements
  if ((buf == NULL) || (size == 0)) return result;

  // generate the preference filename
  sprintf(name, "%s\\preferences-%02x.dat", g_pref_dir, idx);

  // lets try and open the file (create it)
  f = fopen(name, "rb");
  if (f != NULL)
  {
    fseek(f, 0, SEEK_END);
    cnt = ftell(f);
    fseek(f, 0, SEEK_SET);

    // clear the existing preference set
    memset(buf, 0, size);

    // read appropriate number of bytes in
    cnt = MIN(cnt, size);
    fread(g_prefs, 1, cnt, f);

    fclose(f);

    // no problemo
    result = true;
  }

  return result;
}

// save a specific preference bank
void
_PreferencesSaveBank(preference idx, void *buf, uint16 size)
{
  FILE   *f;
  uint32  cnt;
  char    name[MAX_FILENAME_LENGTH];

  // entry requirements
  if ((buf == NULL) || (size == 0)) return;

  // generate the preference filename
  sprintf(name, "%s\\preferences-%02x.dat", g_pref_dir, idx);

  // lets try and open the file (create it)
  f = fopen(name, "wb");
  if (f != NULL)
  {
    // lets just dump our preferences
    cnt = size;

    // write appropriate number of bytes out
    fwrite(buf, 1, cnt, f);

    fclose(f);
  }
}

// delete/remove a specific preference bank
void
_PreferencesKillBank(preference idx)
{
  char    name[MAX_FILENAME_LENGTH];

  // generate the preference filename
  sprintf(name, "%s\\preferences-%02x.dat", g_pref_dir, idx);
  remove(name);
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
  if (g_prefs != NULL) _MemPtrFree(g_prefs);
  g_prefs = NULL;
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-preferences.inc"

/********************************* EOF ***********************************/
