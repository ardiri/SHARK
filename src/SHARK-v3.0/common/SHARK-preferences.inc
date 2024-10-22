/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-preferences.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

typedef struct PrefGlobals
{
  void *struct_prefix;

  void  *pref_data;

#if defined(DAL_PREF_GLOBALS)
  DAL_PREF_GLOBALS
#endif

} PrefGlobals;

GLOBALS_DEFINITION;

#define g_pref_data            (PREF_GLOBALS) -> pref_data 

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

static boolean DAL_PreferencesInitialize();
static void    DAL_PreferencesLoad();
static void    DAL_PreferencesSave();
static void    DAL_PreferencesTerminate();

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
_PreferencesInitialize()
{
  boolean init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _PreferencesInitialize");
#endif

  // allocate memory for globals
  PREF_GLOBALS = malloc(sizeof(PrefGlobals));
  if (GLOBALS_UNINITIALIZED) return false;

  // assume everything is ok
  init_ok =  true;
  init_ok &= DAL_PreferencesInitialize();

  // did everything initialize ok?
  if (init_ok)
  {
    // is there a need for preferences
    if (g_config.preferences.size > 0)
    {
      // we must allocate our preference memory block
      g_pref_data = malloc(g_config.preferences.size);
      init_ok &= (g_pref_data != NULL); if (!init_ok) goto PREF_INIT_ABORT;
      memset(g_pref_data, 0, g_config.preferences.size);
    }
  }

PREF_INIT_ABORT:;

  return init_ok;
}

void *
_PreferencesGetReference()
{
  void *result;

  // default return value
  result = NULL;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;
 
  // get the pointer to preferences
  result = g_pref_data;

  return result;
}

void
_PreferencesLoad()
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;
 
  // entry condition
  if (g_pref_data == NULL) return;

  // pass to the underlying hardware platform
  DAL_PreferencesLoad();
}

void
_PreferencesSave()
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry condition
  if (g_pref_data == NULL) return;

  // pass to the underlying hardware platform
  DAL_PreferencesSave();
}

static void
_PreferencesTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _PreferencesTerminate");
#endif

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // we must free our preference memory block
  if (g_pref_data != NULL) free(g_pref_data);
  g_pref_data = NULL;

  DAL_PreferencesTerminate();

  // free memory for globals
  free(PREF_GLOBALS); 
  PREF_GLOBALS = NULL;
}

/********************************* EOF ***********************************/
