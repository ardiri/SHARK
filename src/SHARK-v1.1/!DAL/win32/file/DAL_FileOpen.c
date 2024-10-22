/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_FileOpen.c
 */

#include "../DAL-prv.h"

boolean
DAL_FileOpen(file *f)
{
  boolean result;
  _file *_f = (_file *)f;

  // default return value
  result = false;

  // open file - only if NULL file pointer
  if (_f->fp == NULL)
  {
    _f->fp = fopen((char *)_f->fileName, "rb");
    result = (_f->fp != NULL);
  }

  return result;
}

/********************************* EOF ***********************************/
