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
  _file   *_f = (_file *)f;

  // default return value
  result = true;

  // do we have a valid file reference
  if (_f->fp != NULL)
  {
    switch (_f->location)
    {
      case FILE_RAM:
           // get the file position
           result = (FileEOF((FileHand)_f->fp) != 0);
           break;
  
      case FILE_CARD:
           // get the file position
           result = (VFSFileEOF((FileRef)_f->fp) != errNone);
           break;
  
      default:
           break;
    }
  }

  return result;
}

/********************************* EOF ***********************************/
