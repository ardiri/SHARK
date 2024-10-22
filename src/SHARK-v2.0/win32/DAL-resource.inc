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

  // define the volumes we actually may look for resources on using files
  success  = false;
  success |= _FileDefineDisk(RESOURCE_ID, "..\\Common", DISK_SYSTEM);
  success |= _FileDefineDisk(RESOURCE_ID, "\\Common",   DISK_SYSTEM);

  return init_ok;
}

boolean
_ResourceExists(resource resNum)
{
  boolean result;
  HRSRC   res;

  // default return value
  result = false;

  // lets try and locate the resource
  res = FindResource(g_hInst, MAKEINTRESOURCE(resNum), RT_RCDATA);
  result = (res != NULL);

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
  HRSRC  res;

  // default return value
  result = 0;

  // lets try and locate the resource
  res = FindResource(g_hInst, MAKEINTRESOURCE(resNum), RT_RCDATA);
  if (res != NULL)
    result = (uint16)SizeofResource(g_hInst, res);

  // should we look for it using the file system?
  if (res == NULL)
    result = _ResourceSize_FILE(RESOURCE_ID, resNum);

  return result;
}

// lock down a resource
void *
_ResourceLock(resource resNum)
{
  void   *result, *buf;
  uint16  size;
  HRSRC   res;
  HGLOBAL resPtr;

  // default return value
  result = NULL;

  // lets try and locate the resource
  res = FindResource(g_hInst, MAKEINTRESOURCE(resNum), RT_RCDATA);
  if (res != NULL)
  {
    resPtr = LoadResource(g_hInst, res);
    size   = (uint16)SizeofResource(g_hInst, res);

    // allocate buffer (non dynamic) for it, and, read it in
    if ((resPtr != NULL) && (size != 0))
    {
      result = _MemPtrNew(size, true);
      if (result != NULL)
      {
        buf = LockResource(resPtr);

        _MemSemaphore(true);
        _MemMove(result, buf, size);
        _MemSemaphore(false);

        DeleteObject(buf);
      }
    }
  }

  // should we look for it using the file system?
  if (res == NULL)
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
