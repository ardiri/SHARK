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
  uint32  bytesRead, bytes;
  _file  *_f = (_file *)f;

  // entry condition
  if (_f->fp == NULL) return;

  switch (_f->location)
  {
    case FILE_RAM:
         // do the file read
         *objRead = FileRead((FileHand)_f->fp, (void *)buf, (uint16)objSize, (uint16)numObject, NULL);
         break;

    case FILE_CARD:
         bytes = ((uint32)objSize * numObject);

         // do the file read
         VFSFileRead((FileRef)_f->fp, bytes, (void *)buf, &bytesRead);
         *objRead = bytesRead / objSize;
         break;

    default:
         break;
  }
}

/********************************* EOF ***********************************/
