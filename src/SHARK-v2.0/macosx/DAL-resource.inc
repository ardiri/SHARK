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

#define RESOURCE_ID_MAC  "- RESOURCES MAC -"
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

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _ResourceInitialize");
#endif

  // assume everything is ok
  init_ok = true;

//
// MACOS-X
// /macosx/PROGRAM.app/
// - where, "PROGRAM.app" is the launchable application (from finder)
//

  // define the volumes we actually may look for resources on using files
  success  = false;
  success |= _FileDefineDisk(RESOURCE_ID_MAC,
                            "#Contents\\Resources",        DISK_SYSTEM);
  success |= _FileDefineDisk(RESOURCE_ID, "..\\Common",     DISK_SYSTEM);
  success |= _FileDefineDisk(RESOURCE_ID, "\\Common",       DISK_SYSTEM);

  // we must have AT LEAST one preference store
  init_ok = success;

  return init_ok;
}

boolean
_ResourceExists(resource resNum)
{
  boolean result;

  // default return value
  result = false;

  // look locally first
  result = _ResourceExists_FILE(RESOURCE_ID_MAC, resNum);

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

  // default return value
  result = 0;

  // look locally first
  if (_ResourceExists_FILE(RESOURCE_ID_MAC, resNum))
    result = _ResourceSize_FILE(RESOURCE_ID_MAC, resNum);

  // should we look for it using the file system?
  else
    result = _ResourceSize_FILE(RESOURCE_ID, resNum);

  return result;
}

// lock down a resource
void *
_ResourceLock(resource resNum)
{
  void   *result;

  // default return value
  result = NULL;

  // look locally first
  if (_ResourceExists_FILE(RESOURCE_ID_MAC, resNum))
    result = _ResourceLock_FILE(RESOURCE_ID_MAC, resNum);

  // should we look for it using the file system?
  else
    result = _ResourceLock_FILE(RESOURCE_ID, resNum);

  return result;
}

void
_ResourceUnlock(void *ptr)
{
  // entry level condition
  if (ptr == NULL) return;

  _MemPtrFree(ptr);
}

// terminate the resource manager
static void
_ResourceTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _ResourceTerminate");
#endif
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-resource.inc"

/********************************* EOF ***********************************/
