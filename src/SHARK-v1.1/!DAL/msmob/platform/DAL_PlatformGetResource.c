/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformGetResource.c
 */

#include "../DAL-prv.h"

void *
DAL_PlatformGetResource(uint16 resNum)
{
  MSmob_PlatformInfo *platform;
  void               *result, *buf;
  uint16              size;
  HRSRC               res;
  HGLOBAL             resPtr;
  uint8               fN[256];
  FILE               *f;
  GLOBALS_ACCESS;

  // default return value
  result = NULL;
  res    = NULL;
  size   = DAL_PlatformGetResourceSize(resNum);  // sanity checking

  // 
  // MODE #1 application resource          (RCDATA)
  //

  // access device/platform information block
  platform = (MSmob_PlatformInfo *)g->platform;
  if ((platform != NULL) && (size != 0))
  {  
    res = FindResource(platform->windows.hInst, MAKEINTRESOURCE(resNum), RT_RCDATA);
    if (res != NULL) 
    {
      size   = (uint16)SizeofResource(platform->windows.hInst, res);
      resPtr = LoadResource(platform->windows.hInst, res);

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
  } 
 
  // 
  // MODE#2   ./xxxx{resNum}.bin (relative directory)
  //

  if (result == NULL)
  {
    sprintf((char *)fN, "./xxxx%04x.bin", resNum);
    f = fopen((char *)fN, "rb");

    // did we find the file?
    if (f != NULL)
    {
      // determine the size of the buffer
      fseek(f, 0, SEEK_END);
      size = (uint16)ftell(f);
      fseek(f, 0, SEEK_SET);

      // allocate buffer (non dynamic) for it, and, read it in
      if (size != 0)
      {
        result = _MemPtrNew(size, true);
        if (result != NULL)
        {
          _MemSemaphore(true);
          fread(result, 1, size, f);
          _MemSemaphore(false);
        }
      }
  
      // close the file :)
      fclose(f);
    }
  }

  // 
  // MODE#3   /Common/xxxx{resNum}.bin     (external memory card)
  //

  if (result == NULL)
  {
    // look on device root
    sprintf((char *)fN, "/Common/xxxx%04x.bin", resNum);
    f = fopen((char *)fN, "rb");

    // look on storage card
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

        sprintf((char *)fN, "/%s/Common/xxxx%04x.bin", path, resNum);
        f = fopen((char *)fN, "rb");
        if (f != NULL) break;

        search = FindNextFlashCard(hFlashCard, &hFlashCardInfo);
      }
    }

    // did we find the file?
    if (f != NULL)
    {
      // determine the size of the buffer
      fseek(f, 0, SEEK_END);
      size = (uint16)ftell(f);
      fseek(f, 0, SEEK_SET);

      // allocate buffer (non dynamic) for it, and, read it in
      if (size != 0)
      {
        result = _MemPtrNew(size, true);
        if (result != NULL)
        {
          _MemSemaphore(true);
          fread(result, 1, size, f);
          _MemSemaphore(false);
        }
      }
  
      // close the file :)
      fclose(f);
    }
  }

  // 
  // MODE#4   /Program/{application}/xxxx{resNum}.bin     (external memory card)
  //

  if (result == NULL)
  {
    // look on device root
    sprintf((char *)fN, "/Program/%s/xxxx%04x.bin", ApplicationGetName(), resNum);
    f = fopen((char *)fN, "rb");

    // look on storage card
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

        sprintf((char *)fN, "/%s/Program/%s/xxxx%04x.bin", path, ApplicationGetName(), resNum);
        f = fopen((char *)fN, "rb");
        if (f != NULL) break;

        search = FindNextFlashCard(hFlashCard, &hFlashCardInfo);
      }
    }

    // did we find the file?
    if (f != NULL)
    {
      // determine the size of the buffer
      fseek(f, 0, SEEK_END);
      size = (uint16)ftell(f);
      fseek(f, 0, SEEK_SET);

      // allocate buffer (non dynamic) for it, and, read it in
      if (size != 0)
      {
        result = _MemPtrNew(size, true);
        if (result != NULL)
        {
          _MemSemaphore(true);
          fread(result, 1, size, f);
          _MemSemaphore(false);
        }
      }
  
      // close the file :)
      fclose(f);
    }
  }

  return result;
}

/********************************* EOF ***********************************/
