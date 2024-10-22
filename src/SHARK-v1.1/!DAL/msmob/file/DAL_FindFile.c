/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_FindFile.c
 */

#include "../DAL-prv.h"

file *
DAL_FindFile(uint8 *fileName)
{
  _file   *f;
  boolean found;

  // default return values
  found = false;

  f = (_file *)_MemPtrNew(sizeof(_file), true);
  _MemSemaphore(true);
  f->location = 0;    // unused on this platform
  f->fp       = NULL;

  // 
  // MODE#1   ./{filename} (relative directory)
  //

  if (!found)
  {
    sprintf((char *)f->fileName, "./%s", fileName);
    f->fp = fopen((char *)f->fileName, "rb");

    // did we find the file?
    if (f->fp != NULL)
    {
      found = true;
      fclose((FILE *)f->fp); f->fp = NULL;
    }
  }

  // 
  // MODE#2   /Common/{filename}     (external memory card)
  //

  if (!found)
  {
    // look on device root
    sprintf((char *)f->fileName, "/Common/%s", fileName);
    f->fp = fopen((char *)f->fileName, "rb");

    // look on storage card
    if (f->fp == NULL)
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

        sprintf((char *)f->fileName, "/%s/Common/%s", path, fileName);
        f->fp = fopen((char *)f->fileName, "rb");
        if (f->fp != NULL) break;

        search = FindNextFlashCard(hFlashCard, &hFlashCardInfo);
      }
    }

    // did we find the file?
    if (f->fp != NULL)
    {
      found = true;
      fclose((FILE *)f->fp); f->fp = NULL;
    }
  }

  // 
  // MODE#3   /Program/{application}/{filename}     (external memory card)
  //

  if (!found)
  {
    // look on device root
    sprintf((char *)f->fileName, "/Program/%s/%s", ApplicationGetName(), fileName);
    f->fp = fopen((char *)f->fileName, "rb");

    // look on storage card
    if (f->fp == NULL)
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

        sprintf((char *)f->fileName, "/%s/Program/%s/%s", path, ApplicationGetName(), fileName);
        f->fp = fopen((char *)f->fileName, "rb");
        if (f->fp != NULL) break;

        search = FindNextFlashCard(hFlashCard, &hFlashCardInfo);
      }
    }

    // did we find the file?
    if (f->fp != NULL)
    {
      found = true;
      fclose((FILE *)f->fp); f->fp = NULL;
    }
  }
  _MemSemaphore(false);

  // if we didn't find a file, lets bail out
  if (!found)
  {
    _MemPtrFree(f);
    f = NULL;
  }

  return (file *)f;
}

/********************************* EOF ***********************************/
