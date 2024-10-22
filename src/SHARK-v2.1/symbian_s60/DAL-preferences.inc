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
  void *prefs;
  char  dir[MAX_PATH_LENGTH];
} PrfGlobalsType;

#define GLOBALS_DEFINE_PRF PrfGlobalsType *g_prf;
#define GLOBALS_ACCESS_PRF GLOBALS_ACCESS; \
                           PrfGlobalsType *g_prf = (PrfGlobalsType *)g->prf;

#define g_prefs g_prf->prefs
#define g_dir   g_prf->dir

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
  GLOBALS_ACCESS;
  GLOBALS_DEFINE_PRF;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _PreferencesInitialize");
#endif

  // assume everything is ok
  init_ok = true;

  // globals:: allocate appropriate memory
  g_prf = (PrfGlobalsType *)malloc(sizeof(PrfGlobalsType));
  init_ok = (g_prf != NULL); if (!init_ok) goto PREF_INIT_ABORT;
  memset(g_prf, 0, sizeof(PrfGlobalsType));
  g->prf = g_prf;

  // defaults
  g_prefs = NULL;

  // initialize the memory we need for the preferences
  if (g_config.preferences.size > 0)
  {
    g_prefs = (void *)_MemPtrNew(g_config.preferences.size, false);
    init_ok = (g_prefs != NULL); if (!init_ok) goto PREF_INIT_ABORT;
  }

  // ensure the preference directory actually exists
  sprintf(g_dir, "C:\\System\\Data\\%s\\%s", 
                  g_config.company, g_config.name);
  init_ok = (chdir(g_dir) == 0);
  if (!init_ok) 
  {
    sprintf(tmp, "C:\\System\\Data\\%s", g_config.company);
    if (chdir(tmp) != 0) mkdir(tmp); 
    
    init_ok = (mkdir(g_dir) == 0);
  }

PREF_INIT_ABORT:

  // since we've been messing around with directories; revert to default
  chdir(g_path);

  return init_ok;
}

// get a pointer to the preferences structure
void *
_PreferencesGetReference()
{
  GLOBALS_ACCESS_PRF;

  return (void *)g_prefs;
}

// load the application preferences
void
_PreferencesLoad()
{
  FILE   *f;
  uint32  cnt;
  char    name[MAX_FILENAME_LENGTH];
  GLOBALS_ACCESS_PRF;

  // entry requirements
  if (g_prefs == NULL) return;

  // generate the preference filename
  sprintf(name, "%s\\preferences.dat", g_dir);

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
  GLOBALS_ACCESS_PRF;

  // entry requirements
  if (g_prefs == NULL) return;

  // generate the preference filename
  sprintf(name, "%s\\preferences.dat", g_dir);

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
  GLOBALS_ACCESS_PRF;

  // assume failure
  result = false;

  // entry requirements
  if ((buf == NULL) || (size == 0)) return result;

  // generate the preference filename
  sprintf(name, "%s\\preferences-%02x.dat", g_dir, idx);

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
  GLOBALS_ACCESS_PRF;

  // entry requirements
  if ((buf == NULL) || (size == 0)) return;

  // generate the preference filename
  sprintf(name, "%s\\preferences-%02x.dat", g_dir, idx);

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
  GLOBALS_ACCESS_PRF;

  // generate the preference filename
  sprintf(name, "%s\\preferences-%02x.dat", g_dir, idx);
  remove(name);
}

// terminate the preferences engine
static void
_PreferencesTerminate()
{
  GLOBALS_ACCESS_PRF;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _PreferencesTerminate");
#endif

  if (g_prf)
  {
    // clean up
    if (g_prefs != NULL) _MemPtrFree(g_prefs);
    g_prefs = NULL;
  }

  // globals:: clean up memory
  if (g_prf) free(g_prf); g_prf = NULL;
  g->prf = g_prf;
}

#undef GLOBALS_DEFINE_PRF
#undef GLOBALS_ACCESS_PRF

#undef g_prefs
#undef g_dir

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-preferences.inc"

/********************************* EOF ***********************************/
