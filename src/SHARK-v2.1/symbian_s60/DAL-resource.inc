/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-resource.c
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
  void *dummy;
} ResGlobalsType;

#define GLOBALS_DEFINE_RES ResGlobalsType *g_res;
#define GLOBALS_ACCESS_RES GLOBALS_ACCESS; \
                           ResGlobalsType *g_res = (ResGlobalsType *)g->res;

#define RESOURCE_ID_S60  "- RESOURCES S60 -"
#define RESOURCE_ID      "- RESOURCES -"

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static boolean     _ResourceExists_FILE(char *diskname, resource resNum);
static uint16      _ResourceSize_FILE(char *diskname, resource resNum);
static void       *_ResourceLock_FILE(char *diskname, resource resNum);

// initialize the memory manager
static boolean
_ResourceInitialize()
{
  boolean init_ok;
  boolean success;
  GLOBALS_ACCESS;
  GLOBALS_DEFINE_RES; 

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _ResourceInitialize");
#endif

  // assume everything is ok
  init_ok = true;

  // globals:: allocate appropriate memory
  g_res = (ResGlobalsType *)malloc(sizeof(ResGlobalsType));
  init_ok = (g_res != NULL); if (!init_ok) goto RESOURCE_INIT_ABORT; 
  memset(g_res, 0, sizeof(ResGlobalsType));
  g->res = g_res;

  // define the volumes we actually may look for resources on using files
  success  = false;
  success |= _FileDefineDisk(RESOURCE_ID_S60, NULL, DISK_SYSTEM);
  success |= _FileDefineDisk(RESOURCE_ID, "..\\Common", DISK_SYSTEM);
  success |= _FileDefineDisk(RESOURCE_ID, "\\Common",   DISK_SYSTEM);

  // we must have AT LEAST one preference store
  init_ok = success;

RESOURCE_INIT_ABORT:

  return init_ok;
}

boolean
_ResourceExists(resource resNum)
{
  boolean result;
//GLOBALS_ACCESS_RES;

  // default return value
  result = false;

  // look locally first
  result = _ResourceExists_FILE(RESOURCE_ID_S60, resNum);

  // should we look for it using the file system?
  if (!result)
    result = _ResourceExists_FILE(RESOURCE_ID, resNum);

  return result;
}

// determine the size of a resource
uint16
_ResourceSize(resource resNum)
{
  uint16 result;
//GLOBALS_ACCESS_RES;

  // default return value
  result = 0;

  // look locally first
  if (_ResourceExists_FILE(RESOURCE_ID_S60, resNum))
    result = _ResourceSize_FILE(RESOURCE_ID_S60, resNum);

  // should we look for it using the file system?
  else
    result = _ResourceSize_FILE(RESOURCE_ID, resNum);

  return result;
}

// lock down a resource
void *
_ResourceLock(resource resNum)
{
  void *result;
//GLOBALS_ACCESS_RES;

  // default return value
  result = NULL;

  // look locally first
  if (_ResourceExists_FILE(RESOURCE_ID_S60, resNum))
    result = _ResourceLock_FILE(RESOURCE_ID_S60, resNum);

  // should we look for it using the file system?
  else
    result = _ResourceLock_FILE(RESOURCE_ID, resNum);

  return result;
}

void
_ResourceUnlock(void *ptr)
{
//GLOBALS_ACCESS_RES;

  // entry level condition
  if (ptr == NULL) return;

  _MemPtrFree(ptr);
}

// terminate the resource manager
static void
_ResourceTerminate()
{
  GLOBALS_ACCESS_RES;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _ResourceTerminate");
#endif

  // globals:: clean up memory
  if (g_res) free(g_res); g_res = NULL;
  g->res = g_res;
}

#undef GLOBALS_DEFINE_RES
#undef GLOBALS_ACCESS_RES

#undef RESOURCE_ID_S60
#undef RESOURCE_ID

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-resource.inc"

/********************************* EOF ***********************************/
