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
    switch (_f->location)
    {
      case FILE_RAM:
           FileClose((FileHand)_f->fp);
           break;
  
      case FILE_CARD:
           VFSFileClose((FileRef)_f->fp);
           break;
  
      default:
           break;
    }
  }

  // free the file information block
  _MemPtrFree(_f);
}

/********************************* EOF ***********************************/
