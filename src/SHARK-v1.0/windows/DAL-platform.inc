/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)windows/DAL-platform.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 * Platform Implementation Structures
 *
 *************************************************************************/

// none

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

void
PlatformInitialize()
{
}

void
PlatformTerminate()
{
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

uint16
PlatformGetResourceSize(uint16 resNum)
{
  uint16  result;
  HRSRC   res;
  uint8   fileName[256];
  FILE   *f;

  // 
  // MODE#1   application resource
  //

  res = FindResource(gDALGlobals.hInst, MAKEINTRESOURCE(resNum), RT_RCDATA);
  if (res != NULL) 
    result = (uint16)SizeofResource(gDALGlobals.hInst, res);

  // 
  // MODE#2   /Common/xxxx{resNum}.bin
  //

  if (res == NULL) 
  {
#ifdef _WIN32_WCE
    sprintf((char *)fileName, "/Common/xxxx%04x.bin", resNum);
#else
    sprintf((char *)fileName, "\\Common\\xxxx%04x.bin", resNum);
#endif

    f = fopen((char *)fileName, "rb");
#ifdef _WIN32_WCE
    // if not found, then - we need to start looking on storage cards
    if (f == NULL)
    {
      HANDLE           hFlashCard;
      WIN32_FIND_DATA  hFlashCardInfo;
      TCHAR           *cFlashCardPath;
      boolean          search;
      uint8            path[MAX_PATH], i;

      // find first card, no handle? break
      hFlashCard = FindFirstFlashCard(&hFlashCardInfo);
      search = (hFlashCard != INVALID_HANDLE_VALUE);

      while (search)
      {
        cFlashCardPath = hFlashCardInfo.cFileName;

        // convert from TCHAR --> uint8 buffer
        i = 0;
        while (cFlashCardPath[i] != 0) { path[i] = (uint8)cFlashCardPath[i]; i++; }
        path[i] = 0;

        sprintf((char *)fileName, "/%s/Common/xxxx%04x.bin", path, resNum);
        f = fopen((char *)fileName, "rb");
        if (f != NULL) break;

        search = FindNextFlashCard(hFlashCard, &hFlashCardInfo);
      }
    }
#endif

    if (f != NULL)
    {
      // determine the size of the buffer
      fseek(f, 0, SEEK_END);
      result = (uint16)ftell(f);
      fseek(f, 0, SEEK_SET);
  
      // close the file :)
      fclose(f);
    }
  }

  return result;
}

void *
PlatformGetResource(uint16 resNum)
{
  void    *result = NULL;
  HRSRC    res;
  HGLOBAL  resPtr;
  uint8    fileName[256];
  FILE    *f;
  uint16   size;

  // 
  // MODE#1   application resource
  //

  res = FindResource(gDALGlobals.hInst, MAKEINTRESOURCE(resNum), RT_RCDATA);
  if (res != NULL)
  {
    size   = (uint16)SizeofResource(gDALGlobals.hInst, res);
    resPtr = LoadResource(gDALGlobals.hInst, res);
    
    result = malloc(size);
    if (result != NULL)
    {
      void *buf = LockResource(resPtr);
      memmove(result, buf, (size_t)size);
      DeleteObject(buf);
    }
  }

  // 
  // MODE#2   /Common/xxxx{resNum}.bin
  //

  if (res == NULL) 
  {
#ifdef _WIN32_WCE
    sprintf((char *)fileName, "/Common/xxxx%04x.bin", resNum);
#else
    sprintf((char *)fileName, "\\Common\\xxxx%04x.bin", resNum);
#endif

    f = fopen((char *)fileName, "rb");
#ifdef _WIN32_WCE
    // if not found, then - we need to start looking on storage cards
    if (f == NULL)
    {
      HANDLE           hFlashCard;
      WIN32_FIND_DATA  hFlashCardInfo;
      TCHAR           *cFlashCardPath;
      boolean          search;
      uint8            path[MAX_PATH], i;

      // find first card, no handle? break
      hFlashCard = FindFirstFlashCard(&hFlashCardInfo);
      search = (hFlashCard != INVALID_HANDLE_VALUE);

      while (search)
      {
        cFlashCardPath = hFlashCardInfo.cFileName;

        // convert from TCHAR --> uint8 buffer
        i = 0;
        while (cFlashCardPath[i] != 0) { path[i] = (uint8)cFlashCardPath[i]; i++; }
        path[i] = 0;

        sprintf((char *)fileName, "/%s/Common/xxxx%04x.bin", path, resNum);
        f = fopen((char *)fileName, "rb");
        if (f != NULL) break;

        search = FindNextFlashCard(hFlashCard, &hFlashCardInfo);
      }
    }
#endif

    if (f != NULL)
    {
      // determine the size of the buffer
      fseek(f, 0, SEEK_END);
      size = (uint16)ftell(f);
      fseek(f, 0, SEEK_SET);

      // allocate buffer large enough for it, and, read it in
      result = malloc(size);
      if (result != NULL)
        fread(result, 1, size, f);

      // close the file :)
      fclose(f);
    }
  }

  return result;
}

void
PlatformReleaseResource(void *ptr)
{
  // its just a pointer, free it :)
  if (ptr != NULL) free(ptr);
}

/********************************* EOF ***********************************/
