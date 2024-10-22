/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_FileClose.c
 */

#include "../DAL-prv.h"

void
DAL_FileClose(file *f)
{
  _file *_f = (_file *)f;

  // do we have a valid file reference?
  if (_f->fp != NULL) 
  {
    // close the file 
    fclose((FILE *)_f->fp); 
    _f->fp = NULL;
  }
  
  // free the file information block
  _MemPtrFree(_f);

}

/********************************* EOF ***********************************/
