/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_FileSeek.c
 */

#include "../DAL-prv.h"

void
DAL_FileSeek(file *f, uint32 ofs, file_position position)
{
  _file *_f = (_file *)f;

  // entry condition
  if (_f->fp == NULL) return;

  switch (position)
  {
    case file_beginning: fseek((FILE *)_f->fp, (long)ofs, SEEK_SET); break;
    case file_current:   fseek((FILE *)_f->fp, (long)ofs, SEEK_CUR); break;
    case file_end:       fseek((FILE *)_f->fp, (long)ofs, SEEK_END); break;
    default:                                                         break;
  }
}

/********************************* EOF ***********************************/
