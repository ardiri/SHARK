/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_FileWrite.c
 */

#include "../DAL-prv.h"

uint16
DAL_FileWrite(file *f, void *buf, uint16 objSize, uint16 numObject, int16 *err)
{
  uint16  result;
  uint32  bytesWritten, bytes;
  _file  *_f = (_file *)f;

  // default return value
  result = 0;

  // do we have a valid file reference
  if (_f->fp != NULL)
  {
    switch (_f->location)
    {
      case FILE_RAM:
           result = FileWrite((FileHand)_f->fp, (void *)buf, (uint16)objSize, (uint16)numObject, (Err *)&err);
           break;
  
      case FILE_CARD:
           bytes = ((uint32)objSize * numObject);
           VFSFileWrite((FileRef)_f->fp, bytes, (void *)buf, &bytesWritten);
           result = bytesWritten / objSize;
           break;
  
      default:
           break;
    }
  }

  return result;
}

/********************************* EOF ***********************************/
