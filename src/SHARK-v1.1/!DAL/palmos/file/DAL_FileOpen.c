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
  boolean  result;
  Err      err;
  uint16   volRef;
  uint32   volIterator;
  Boolean  done;
  _file   *_f = (_file *)f;

  // default return value
  result = false;

  // open file - only if NULL file pointer
  if (_f->fp == NULL)
  {
    switch (_f->location)
    {
      case FILE_RAM:
  
           _f->fp = FileOpen(0, _f->fileName, 0, 0, fileModeReadOnly, &err);
           result = ((_f->fp != 0) && (err == errNone));
           break;
  
      case FILE_CARD:
  
           done = false;
  
           // loop through all the mounted volumes.
           volIterator = vfsIteratorStart;
           while ((volIterator != vfsIteratorStop) && (!done))
           {
             err = VFSVolumeEnumerate(&volRef, &volIterator);
             if (err == errNone)
             {
               // open the database on this volume.
               err = VFSFileOpen(volRef, _f->fileName, vfsModeRead, (FileRef *)&_f->fp);
  
               // successful?
               if (err == errNone) done = true;
             }
             else
               volIterator = vfsIteratorStop;
           }
  
           result = done;
           break;
  
      default:
           break;
    }
  }

  return result;
}

/********************************* EOF ***********************************/
