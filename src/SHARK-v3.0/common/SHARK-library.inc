/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-library.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

typedef struct _library
{
  boolean                  in_use:1;
  uint32                   id:31;
  char                     name[MAX_LIBNAME_LENGTH];
  _LibHandleEventCallback  callback;
  void                    *globals;

#if defined(DAL_LIBRARY)
  DAL_LIBRARY
#endif

} _library;

typedef struct LibGlobals
{
  void *struct_prefix;

  uint32   lib_count;
  _library lib_reference[MAX_LIBRARIES];

#if defined(DAL_LIB_GLOBALS)
  DAL_LIB_GLOBALS
#endif

} LibGlobals;

GLOBALS_DEFINITION;

#define g_lib_count            (LIB_GLOBALS) -> lib_count
#define g_lib_reference        (LIB_GLOBALS) -> lib_reference

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

static boolean DAL_LibInitialize();
static void    DAL_LibTerminate();

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
_LibInitialize()
{
  boolean init_ok;
  int16   i;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _LibInitialize");
#endif

  // allocate memory for globals
  LIB_GLOBALS = malloc(sizeof(LibGlobals));
  if (GLOBALS_UNINITIALIZED) return false;

  // assume everything is ok
  init_ok =  true;
  init_ok &= DAL_LibInitialize();

  // did we initialize ok?
  if (init_ok)
  {
    // no libraries have been registered at this point
    g_lib_count = 0;

    // initialize our globals
    for (i=0; i<MAX_LIBRARIES; i++)
      memset(&g_lib_reference[i], 0, sizeof(_library));
  }

  return init_ok;
}

static boolean
_LibHandleEvent(event *e)
{
  int32   index;
  boolean result;

  // default return values
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // we need to pass this event onto every library that wants to know
  index = 0;
  while (index < MAX_LIBRARIES)
  {
    if ((g_lib_reference[index].in_use) && 
        (g_lib_reference[index].callback != NULL))
      result |= g_lib_reference[index].callback(e);

    index++;
  }

  return result;
}

void *
_LibGetGlobals(uint32 id)
{
  void *result;
  int32 index;

  // default return value
  result = NULL;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // lets find out matching index
  index = 0;
  while ((index < MAX_LIBRARIES) && (g_lib_reference[index].id != id)) index++;

  // are we dealing with a valid library
  if ((index < MAX_LIBRARIES) && (g_lib_reference[index].in_use))
    result = g_lib_reference[index].globals;

  return result;
}

boolean
_LibRegister(uint32 id, char *name, _LibHandleEventCallback callback)
{
  boolean result;
  int32   index;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // is there any room?
  if (g_lib_count < MAX_LIBRARIES)
  {
    // we need to find the first available spot
    index = 0;
    while (g_lib_reference[index].in_use) index++;

    // found library; lets initialize it
    strcpy(g_lib_reference[index].name, name);
    g_lib_reference[index].id       = id;
    g_lib_reference[index].globals  = NULL;
    g_lib_reference[index].callback = callback;
    g_lib_reference[index].in_use   = true; // now we are using it
    g_lib_count++;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  {
    char str_log[256];

    sprintf(str_log, ">>  _LibRegister :: %s", name);
    _SHARKLogCreateEntry(str_log);
  }
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
  while ((index < MAX_LIBRARIES) && (g_lib_reference[index].id != id)) index++;

  // entry condition
  if ((index >= MAX_LIBRARIES) || (!g_lib_reference[index].in_use)) return;

  // save the globals pointer value
  g_lib_reference[index].globals = gbls;
}

void
_LibShutdown(uint32 id)
{
  int32 index;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // lets find out matching index and ensure its defined
  index = 0;
  while ((index < MAX_LIBRARIES) && (g_lib_reference[index].id != id)) index++;
  if ((index >= MAX_LIBRARIES) || (!g_lib_reference[index].in_use)) return;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  {
    char str_log[256];

    sprintf(str_log, ">>  _LibShutdown :: %s", g_lib_reference[index].name);
    _SHARKLogCreateEntry(str_log);
  }
#endif

  // save the globals pointer value
  g_lib_reference[index].in_use = false;
}

static void
_LibTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _LibTerminate");
#endif

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  DAL_LibTerminate();

  // free memory for globals
  free(LIB_GLOBALS); 
  LIB_GLOBALS = NULL;
}

/********************************* EOF ***********************************/
