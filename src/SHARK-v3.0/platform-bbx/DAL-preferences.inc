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

#define PREF_DISK_ID       "-PREFERENCES-"
#define PREF_FILENAME      "preferences.dat"

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define PREF_GLOBALS          g_pref
#define GLOBALS_UNINITIALIZED (PREF_GLOBALS == NULL)
#define GLOBALS_DEFINITION    PrefGlobals *g_pref = NULL

// PrefGlobals::   platform additions
#define DAL_PREF_GLOBALS \
  uint32  pref_disk;     \
  file   *pref_file;

#define g_pref_disk            (PREF_GLOBALS) -> pref_disk
#define g_pref_file            (PREF_GLOBALS) -> pref_file

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
  uint32  mode;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(PREF_GLOBALS, 0, sizeof(PrefGlobals));

  // initialize our basics
  g_pref_disk = DISK_INVALID;
  g_pref_file = NULL;

  // we only do this if there are preferences to save
  if (g_config.preferences.size > 0)
  {
    // define the resource disk
    mode = DISK_SYSTEM | DISK_CLOBBER;
    _FileDefineDisk(PREF_DISK_ID, PATH_PREFERENCE, NULL, mode);
    g_pref_disk = _FileLocateDisk(PREF_DISK_ID);

    // get a handle to the preference file
    if (g_pref_disk != DISK_INVALID)
    {
      g_pref_file = _FileGetHandle(g_pref_disk, NULL, PREF_FILENAME);
      if (g_pref_file == NULL)
      {
        // create it if it does not exist
        _FileCreate(g_pref_disk, NULL, PREF_FILENAME, FILE_STANDARD);
        g_pref_file = _FileGetHandle(g_pref_disk, NULL, PREF_FILENAME);
      }
    }
  }

  return init_ok;
}

void
DAL_PreferencesLoad()
{
  uint32 size, cnt;

  // clear the existing preferences
  memset(g_pref_data, 0, g_config.preferences.size);

  // do we have a preference file?
  if (g_pref_file != NULL)
  {
    if (_FileOpen(g_pref_file, FILE_READ_ONLY))
    {
      _FileSeek(g_pref_file, 0, file_end);
      size = _FileTell(g_pref_file);
      _FileSeek(g_pref_file, 0, file_beginning);

      // read the appropriate number of bytes in
      cnt = MIN(size, g_config.preferences.size);
      _FileRead(g_pref_file, g_pref_data, cnt, NULL);

      _FileClose(g_pref_file);
    }
  }
}

void
DAL_PreferencesSave()
{
  // do we have a preference file?
  if (g_pref_file != NULL)
  {
    if (_FileOpen(g_pref_file, FILE_READ_WRITE | FILE_CLOBBER))
    {
      // write appropriate number of bytes to file
      _FileWrite(g_pref_file, g_pref_data, g_config.preferences.size, NULL);

      // close the file
      _FileClose(g_pref_file);
    }
  }
}

void 
DAL_PreferencesTerminate()
{
  // release the handle to the preference file
  if (g_pref_file != NULL)
    _FileReleaseHandle(g_pref_file);
  g_pref_file = NULL;
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef PREF_GLOBALS

#undef PREF_FILENAME
#undef PREF_DISK_ID

/********************************* EOF ***********************************/
