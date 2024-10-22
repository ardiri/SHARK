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
  {
    switch (_f->location)
    {
      case FILE_RAM:
           // obtain the file position
           result = (uint32)FileTell((FileHand)_f->fp, NULL, NULL);
           break;
  
      case FILE_CARD:
           // obtain the file position
           VFSFileTell((FileRef)_f->fp, (uint32 *)&result);
           break;
  
      default:
           break;
    }
  }

  return result;
}

/********************************* EOF ***********************************/
