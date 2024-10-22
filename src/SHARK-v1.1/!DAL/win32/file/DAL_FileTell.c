/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_FileTell.c
 */

#include "../DAL-prv.h"

uint32
DAL_FileTell(file *f)
{
  uint32 result;
  _file *_f = (_file *)f;

  // default return value
  result = 0;

  // do we have a valid file reference?
  if (_f->fp != NULL)
    result = (uint32)ftell((FILE *)_f->fp);

  return result;
}

/********************************* EOF ***********************************/
