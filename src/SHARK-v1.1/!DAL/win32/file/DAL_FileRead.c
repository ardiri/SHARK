/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_FileRead.c
 */

#include "../DAL-prv.h"

void
DAL_FileRead(file *f, void *buf, uint16 objSize, uint16 numObject, uint16 *objRead)
{
  _file *_f = (_file *)f;

  // entry condition
  if (_f->fp == NULL) return;

  // do the file read
  *objRead = (uint16)fread((void *)buf, (size_t)objSize, (size_t)numObject, (FILE *)_f->fp);
}

/********************************* EOF ***********************************/
