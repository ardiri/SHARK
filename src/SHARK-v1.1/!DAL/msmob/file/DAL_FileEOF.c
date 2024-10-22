/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_FileEOF.c
 */

#include "../DAL-prv.h"

boolean
DAL_FileEOF(file *f)
{
  boolean result;
  _file *_f = (_file *)f;

  // default return value
  result = true;

  // do we have a valid file reference
  if (_f->fp != NULL)
    result = (feof((FILE *)_f->fp) != 0);

  return result;
}

/********************************* EOF ***********************************/
