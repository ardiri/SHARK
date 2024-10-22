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

typedef struct
{
  boolean  in_use;
  uint32   id;
  char     name[MAX_LIBNAME_LENGTH];
  _LibHandleEventCallback callback;
  void    *globals;
} _library;

typedef struct
{
  uint32   count;
  _library libraries[MAX_LIBRARIES];
} LibGlobalsType;

#define GLOBALS_DEFINE_LIB LibGlobalsType *g_lib;
#define GLOBALS_ACCESS_LIB GLOBALS_ACCESS; \
                           LibGlobalsType *g_lib = (LibGlobalsType *)g->lib;

#define g_libraries    g_lib->libraries
#define g_libraryCount g_lib->count

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

// initialize the library module
static boolean
_LibInitialize()
{
  boolean init_ok;
  GLOBALS_ACCESS;
  GLOBALS_DEFINE_LIB;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _LibInitialize");
#endif

  // assume everything is ok
  init_ok = true;

  // globals:: allocate appropriate memory
  g_lib = (LibGlobalsType *)malloc(sizeof(LibGlobalsType));
  init_ok = (g_lib != NULL); if (!init_ok) goto LIBRARY_INIT_ABORT;
  memset(g_lib, 0, sizeof(LibGlobalsType));
  g->lib = g_lib;

  // no libraries have been registered at this point
  g_libraryCount = 0;

LIBRARY_INIT_ABORT:

  return init_ok;
}

boolean
_LibRegister(uint32 id, char *name, _LibHandleEventCallback callback)
{
  boolean result;
  int32   index;
  char    log[256];
  GLOBALS_ACCESS_LIB;

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

    _StrCopy(log, ">>  _LibRegister :: ");
    _StrCat(log, name);
    _SysDebugMessage(log, false);

    result = true;
  }

  return result;
}

void
_LibSetGlobals(uint32 id, void *gbls)
{
  int32 index;
  GLOBALS_ACCESS_LIB;

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
  GLOBALS_ACCESS_LIB;

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
  GLOBALS_ACCESS_LIB;

  // lets find out matching index and ensure its defined
  index = 0;
  while ((index < MAX_LIBRARIES) && (g_libraries[index].id != id)) index++;
  if ((index >= MAX_LIBRARIES) || (!g_libraries[index].in_use)) return;

  _StrCopy(log, ">>  _LibShutdown :: ");
  _StrCat(log, g_libraries[index].name);
  _SysDebugMessage(log, false);

  // save the globals pointer value
  g_libraries[index].in_use = false;
}

// terminate the library manager
static void
_LibTerminate()
{
  GLOBALS_ACCESS_LIB;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _LibTerminate");
#endif
  
  // globals:: clean up memory
  if (g_lib) free(g_lib); g_lib = NULL; 
  g->lib = g_lib;
}

static boolean
_LibHandleEvent(event *e)
{
  int32   index;
  boolean processed;
  GLOBALS_ACCESS_LIB;

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

#undef GLOBALS_DEFINE_LIB
#undef GLOBALS_ACCESS_LIB

#undef g_libraries
#undef g_libraryCount

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-library.inc"

/********************************* EOF ***********************************/
