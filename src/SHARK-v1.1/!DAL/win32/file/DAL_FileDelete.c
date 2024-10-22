/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_FileDelete.c
 */

#include "../DAL-prv.h"

void
DAL_FileDelete(file *f)
{
  _file *_f = (_file *)f;

  // is the file open? close it first
  if (_f->fp != NULL) 
  {
    fclose((FILE *)_f->fp); 
    _f->fp = NULL;
  }

  // remove the file 
  remove(_f->fileName); 

  // free the file information block
  _MemPtrFree(_f);
}

/********************************* EOF ***********************************/
