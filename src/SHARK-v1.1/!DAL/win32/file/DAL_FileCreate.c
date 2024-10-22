/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_FileCreate.c
 */

#include "../DAL-prv.h"

file *
DAL_FileCreate(uint8 *fileName)
{
  _file  *f;
  boolean found;

  // default return values
  found = false;

  // try and find the file, close it if it is open
  f = (_file *)_FindFile(fileName);
  if (f != NULL) 
  {
    _FileClose((file *)f); f = NULL;
  }

  // try to create it
  f = (_file *)_MemPtrNew(sizeof(_file), true);
  f->location = 0;    // unused on this platform
  f->fp       = NULL;

  // 
  // MODE#1   ./{filename} (relative directory)
  //

  if (!found)
  {
    sprintf((char *)f->fileName, ".\\%s", fileName);
    f->fp = fopen((char *)f->fileName, "wb");
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

    f->fp = fopen((char *)f->fileName, "wb");
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

    f->fp = fopen((char *)f->fileName, "wb");
    if (f->fp != NULL)
    {
      found = true;
      fclose((FILE *)f->fp); f->fp = NULL;
    }
  }

  // if we didn't find a file, lets bail out
  if (!found)
  {
    _MemPtrFree(f);
    f = NULL;
  }

  return (file *)f;
}

/********************************* EOF ***********************************/
