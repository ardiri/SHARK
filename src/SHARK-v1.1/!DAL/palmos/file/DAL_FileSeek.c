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

  switch (_f->location)
  {
    case FILE_RAM:

         switch (position)
         {
           case file_beginning:
                FileSeek((FileHand)_f->fp, (uint32)ofs, fileOriginBeginning);
                break;

           case file_current:
                FileSeek((FileHand)_f->fp, (uint32)ofs, fileOriginCurrent);
                break;

           case file_end:
                FileSeek((FileHand)_f->fp, (uint32)ofs, fileOriginEnd);
                break;

           default:
                break;
         }
         break;

    case FILE_CARD:

         switch (position)
         {
           case file_beginning:
                VFSFileSeek((FileRef)_f->fp, vfsOriginBeginning, (uint32)ofs);
                break;

           case file_current:
                VFSFileSeek((FileRef)_f->fp, vfsOriginCurrent,   (uint32)ofs);
                break;

           case file_end:
                VFSFileSeek((FileRef)_f->fp, vfsOriginEnd,       (uint32)ofs);
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }
}

/********************************* EOF ***********************************/
