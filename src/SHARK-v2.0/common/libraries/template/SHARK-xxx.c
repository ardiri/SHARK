/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-xxx.c
 */

#include "../../SHARK.h"

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

#define XXX_LIBRARY_ID  '_xxx'

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct _XXXGlobals
{
  void         *extension;

  boolean     (*fnXXXInitialize)(void);
  void        (*fnXXXTerminate)(void);

} _XXXGlobals;

#define XXX_GLOBALS_ACCESS \
  _XXXGlobals *g_xxx = (_XXXGlobals *)_LibGetGlobals(XXX_LIBRARY_ID);

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibGetGlobals() API.
//------------------------------------------------------------------------

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static void    _XXXInitialize_RESIZE();
static void    _XXXTerminate_RESIZE();

static boolean _XXXDriverConfiguration(void);
static void    _XXXHandleEvent(event *e);

boolean
_XXXInitialize(void)
{
  boolean      init_ok;
  _XXXGlobals *g_xxx;

  // assume everything is ok
  init_ok = true;

  // lets see if we have already been initialized
  g_xxx = (_XXXGlobals *)_LibGetGlobals(XXX_LIBRARY_ID);

  // entry condition
  if (g_xxx != NULL) return false;

  // register the library
  if (_LibRegister(XXX_LIBRARY_ID, "_XXXLibrary", (void *)_XXXHandleEvent))
  {
    // allocate globals memory for the library
    g_xxx = (_XXXGlobals *)_MemPtrNew(sizeof(_XXXGlobals), false);
    init_ok &= (g_xxx != NULL); if (!init_ok) goto XXX_INIT_ABORT;
    _LibSetGlobals(XXX_LIBRARY_ID, (void *)g_xxx);

    // initialize the _XXX driver
    init_ok = _XXXDriverConfiguration();
    if (!init_ok) goto XXX_INIT_ABORT;

    // initialize _Fnt internally
    _XXXInitialize_RESIZE();
  }
  else
    init_ok = false;

XXX_INIT_ABORT:

  return init_ok;
}

void
_XXXTerminate()
{
  XXX_GLOBALS_ACCESS;

  // entry requirement (must call _XXXInitialize())
  if (g_xxx == NULL) return;

  // shutdown the sub-XXX layer
  if (g_xxx->fnXXXTerminate != NULL)
    g_xxx->fnXXXTerminate();

  // terminate _XXX internally
  _XXXTerminate_RESIZE();

  // free the globals used
  _MemPtrFree(g_xxx); g_xxx = NULL;
  _LibSetGlobals(XXX_LIBRARY_ID, (void *)g_xxx);

  // shutdown the library
  _LibShutdown(XXX_LIBRARY_ID);
}

static void
_XXXInitialize_RESIZE()
{
}

static void
_XXXTerminate_RESIZE()
{
}

static boolean
_XXXDriverConfiguration(void)
{
  boolean     result;
  XXX_GLOBALS_ACCESS;

  // default return value
  result = false;

  // initialize the sub-xxx layer
  if (g_xxx->fnXXXInitialize != NULL)
    result = g_xxx->fnXXXInitialize();

  return result;
}

static void
_XXXHandleEvent(event *e)
{
  switch (e->eType)
  {
    case _keyEvent:
         switch (e->data.key.chr)
         {
           case _key_gfx_change:
                _XXXTerminate_RESIZE();
                _XXXInitialize_RESIZE();
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }
}

/********************************* EOF ***********************************/
