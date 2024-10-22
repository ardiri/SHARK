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
    sprintf((char *)f->fileName, ".\\%s", fileName);

    f->fp = fopen((char *)f->fileName, "rb");
    if (f->fp != NULL)
    {
      found = true;
      fclose((FILE *)f->fp); f->fp = NULL;
    }
  }

  // 
  // MODE#2   /Common/xxxx{resNum}.bin     (external memory card)
  //

  if (!found)
  {
    sprintf((char *)f->fileName, "\\Common\\%s", fileName);

    f->fp = fopen((char *)f->fileName, "rb");
    if (f->fp != NULL)
    {
      found = true;
      fclose((FILE *)f->fp); f->fp = NULL;
    }
  }

  // 
  // MODE#3   /Program/{application}/xxxx{resNum}.bin     (external memory card)
  //

  if (!found)
  {
    sprintf((char *)f->fileName, "\\Program\\%s\\%s", ApplicationGetName(), fileName);

    f->fp = fopen((char *)f->fileName, "rb");
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
