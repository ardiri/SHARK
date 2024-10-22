/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_FileWrite.c
 */

#include "../DAL-prv.h"

uint16
DAL_FileWrite(file *f, void *buf, uint16 objSize, uint16 numObject, int16 *err)
{
  _file *_f = (_file *)f;
  uint16 result;

  // default return value
  result = 0;

  // do we have a valid file reference
  if (_f->fp != NULL)
    result = fwrite((void *)buf, (size_t)objSize, (size_t)numObject, (FILE *)_f->fp);

  return result;
}

/********************************* EOF ***********************************/
