/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-library.c
 */

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct _library
{
  boolean  in_use;
  uint32   id;
  char     name[MAX_LIBNAME_LENGTH];
  _LibHandleEventCallback callback;
  void    *globals;
} _library;

typedef struct
{
  _library libraries[MAX_LIBRARIES];
  uint32   libraryCount;
} LibGlobals;

#define g_libraries            ((LibGlobals *)g -> lib) -> libraries
#define g_libraryCount         ((LibGlobals *)g -> lib) -> libraryCount

#define GLOBALS_UNINITIALIZED  (g -> lib == NULL)

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

// initialize the library module
static boolean
_LibInitialize()
{
  int16       i;
  boolean     init_ok;
  LibGlobals *g_lib;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _LibInitialize");
#endif

  // allocate memory for library globals
  g_lib = (LibGlobals *)malloc(sizeof(LibGlobals));
  if (g_lib != NULL) g -> lib = (void *)g_lib;

  // assume everything is ok
  init_ok  = true;
  init_ok &= (g_lib != NULL);

  // no libraries have been registered at this point
  if (init_ok) 
  {
    memset(g_lib, 0, sizeof(LibGlobals));

    g_libraryCount = 0;

    // initialize our globals
    for (i=0; i<MAX_LIBRARIES; i++)
      memset(&g_libraries[i], 0, sizeof(_library));
  }

  return init_ok;
}

boolean
_LibRegister(uint32 id, char *name, _LibHandleEventCallback callback)
{
  boolean result;
  int32   index;
  char    log[256];

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return false;

  // default return value
  result = false;

  // is there any room?
  if (g_libraryCount < MAX_LIBRARIES)
  {
    // we need to find the first available spot
    index = 0;
    while (g_libraries[index].in_use) index++;

    // found library; lets initialize it
    strcpy(g_libraries[index].name, name);
    g_libraries[index].id       = id;
    g_libraries[index].globals  = NULL;
    g_libraries[index].callback = callback;
    g_libraries[index].in_use   = true; // now we are using it
    g_libraryCount++;

#if (LOG_DISABLE == 0)
    _StrCopy(log, ">>  _LibRegister :: ");
    _StrCat(log, name);
    _LOGCreateEntry(log);
#endif

    result = true;
  }

  return result;
}

void
_LibSetGlobals(uint32 id, void *gbls)
{
  int32 index;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // lets find out matching index
  index = 0;
  while ((index < MAX_LIBRARIES) && (g_libraries[index].id != id)) index++;

  // entry condition
  if ((index >= MAX_LIBRARIES) || (!g_libraries[index].in_use)) return;

  // save the globals pointer value
  g_libraries[index].globals = gbls;
}

void *
_LibGetGlobals(uint32 id)
{
  void *result;
  int32 index;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return NULL;

  // default return value
  result = NULL;

  // lets find out matching index
  index = 0;
  while ((index < MAX_LIBRARIES) && (g_libraries[index].id != id)) index++;

  // are we dealing with a valid library
  if ((index < MAX_LIBRARIES) && (g_libraries[index].in_use))
    result = g_libraries[index].globals;

  return result;
}

void
_LibShutdown(uint32 id)
{
  int32 index;
  char  log[256];

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // lets find out matching index and ensure its defined
  index = 0;
  while ((index < MAX_LIBRARIES) && (g_libraries[index].id != id)) index++;
  if ((index >= MAX_LIBRARIES) || (!g_libraries[index].in_use)) return;

#if (LOG_DISABLE == 0)
  _StrCopy(log, ">>  _LibShutdown :: ");
  _StrCat(log, g_libraries[index].name);
  _LOGCreateEntry(log);
#endif

  // save the globals pointer value
  g_libraries[index].in_use = false;
}

// terminate the library manager
static void
_LibTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _LibTerminate");
#endif

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // free memory used by library globals
  if (g -> lib != NULL) free(g -> lib);
  g -> lib = NULL;
}

static boolean
_LibHandleEvent(event *e)
{
  int32   index;
  boolean processed;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return false;

  // default return values
  processed = false;

  // we need to pass this event onto every library that wants to know
  index = 0;
  while (index < MAX_LIBRARIES)
  {
    if ((g_libraries[index].in_use) && (g_libraries[index].callback != NULL))
      processed |= g_libraries[index].callback(e);

    index++;
  }

  return processed;
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-library.inc"

#undef GLOBALS_UNINITIALIZED

/********************************* EOF ***********************************/
