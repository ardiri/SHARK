/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-file.c
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 * Platform Implementation Structures
 *
 *************************************************************************/

#define BASEDIR       "Programs"
#define FILENAME_SIZE 256
#define DBNAME_SIZE   32

enum
{
  FILE_INVALID = 0,
  FILE_RAM,
  FILE_CARD
};

typedef struct _PalmFile
{
  int    location;
  uint8  fileName[DBNAME_SIZE];

  union
  {
    struct
    {
      uint16    card;
      FileHand  fileH;
    } ram;

    struct
    {
      FileRef   fileRef;
    } vfs;

  } data;

} _PalmFile;

// local functions
static void _FileInitialize() __DAL__;
static void _FileTerminate()  __DAL__;

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

void
_FileInitialize()
{
}

void
_FileTerminate()
{
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

file *
_FindFile(uint8 *fileName)
{
  _PalmFile *f;
  int8       filename[FILENAME_SIZE];
  uint16     volRef;
  uint32     volIterator, version;
  Err        err;

  f = (_PalmFile*)MemPtrNew(sizeof(_PalmFile));
  MemSet(f, sizeof(_PalmFile), 0);
  StrCopy(f->fileName, fileName);

  //
  // RAM search
  //

  // ram filestream database
  f->data.ram.card  = 0;
  f->data.ram.fileH = FileOpen(f->data.ram.card, f->fileName, 0, 0, fileModeReadOnly, &err);
  if ((f->data.ram.fileH != 0) && (err == errNone))
  {
    f->location = FILE_RAM;
    FileClose(f->data.ram.fileH);
  }

  //
  // VFS search
  //

  err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version);
  if ((err == errNone) && (f->location == FILE_INVALID))
  {
    // generate the filename
//  StrPrintF(filename, "/%s/%s/%s", BASEDIR, ApplicationGetName(), fileName);
    StrCopy(filename, "/");
    StrCat(filename, BASEDIR);
    StrCat(filename, "/");
    StrCat(filename, ApplicationGetName());
    StrCat(filename, "/");
    StrCat(filename, f->fileName);

    // loop through all the mounted volumes.
    volIterator = vfsIteratorStart;
    while ((volIterator != vfsIteratorStop) && (f->location == FILE_INVALID))
    {
      err = VFSVolumeEnumerate(&volRef, &volIterator);
      if (err == errNone)
      {
        // look for the database on this volume.
        err = VFSFileOpen(volRef, filename, vfsModeRead, &f->data.vfs.fileRef);

        // did we find the resource?
        if (err == errNone)
        {
          f->location = FILE_CARD;
          VFSFileClose(f->data.vfs.fileRef);
        }
      }
      else
        volIterator = vfsIteratorStop;
    }
  }

  //
  // error handling
  //

  if (f->location == FILE_INVALID)
  {
    MemPtrFree(f);
    f = NULL;
  }

  return (file *)f;
}

boolean
_FileOpen(file *fileP)
{
  boolean    result = false;
  _PalmFile *f      = (_PalmFile *)fileP;
  int8       filename[FILENAME_SIZE];
  Err        err;
  uint16     volRef;
  uint32     volIterator;
  Boolean    done;

  switch (f->location)
  {
    case FILE_RAM:

         f->data.ram.fileH = FileOpen(f->data.ram.card, f->fileName, 0, 0, fileModeReadOnly, &err);
         result = ((f->data.ram.fileH != 0) && (err == errNone));
         break;

    case FILE_CARD:

         done = false;

         // generate the filename
//       StrPrintF(filename, "/%s/%s/%s", BASEDIR, ApplicationGetName(), fileName);
         StrCopy(filename, "/");
         StrCat(filename, BASEDIR);
         StrCat(filename, "/");
         StrCat(filename, ApplicationGetName());
         StrCat(filename, "/");
         StrCat(filename, f->fileName);

         // loop through all the mounted volumes.
         volIterator = vfsIteratorStart;
         while ((volIterator != vfsIteratorStop) && (!done))
         {
           err = VFSVolumeEnumerate(&volRef, &volIterator);
           if (err == errNone)
           {
             // open the database on this volume.
             err = VFSFileOpen(volRef, filename, vfsModeRead, &f->data.vfs.fileRef);

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

  return result;
}

void
_FileSeek(file *fileP, uint32 ofs, file_position position)
{
  _PalmFile *f = (_PalmFile *)fileP;

  switch (f->location)
  {
    case FILE_RAM:

         switch (position)
         {
           case file_beginning:
                FileSeek(f->data.ram.fileH, (uint32)ofs, fileOriginBeginning);
                break;

           case file_current:
                FileSeek(f->data.ram.fileH, (uint32)ofs, fileOriginCurrent);
                break;

           case file_end:
                FileSeek(f->data.ram.fileH, (uint32)ofs, fileOriginEnd);
                break;

           default:
                break;
         }
         break;

    case FILE_CARD:

         switch (position)
         {
           case file_beginning:
                VFSFileSeek(f->data.vfs.fileRef, vfsOriginBeginning, (uint32)ofs);
                break;

           case file_current:
                VFSFileSeek(f->data.vfs.fileRef, vfsOriginCurrent,   (uint32)ofs);
                break;

           case file_end:
                VFSFileSeek(f->data.vfs.fileRef, vfsOriginEnd,       (uint32)ofs);
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }
}

uint32
_FileTell(file *fileP)
{
  uint32     result = 0;
  _PalmFile *f      = (_PalmFile *)fileP;

  switch (f->location)
  {
    case FILE_RAM:
         result = (uint32)FileTell(f->data.ram.fileH, NULL, NULL);
         break;

    case FILE_CARD:
         VFSFileTell(f->data.vfs.fileRef, (uint32 *)&result);
         break;

    default:
         break;
  }

  return result;
}

boolean
_FileEOF(file *fileP)
{
  boolean    result = true;
  _PalmFile *f      = (_PalmFile *)fileP;

  switch (f->location)
  {
    case FILE_RAM:
         result = (FileEOF(f->data.ram.fileH) != 0);
         break;

    case FILE_CARD:
         result = (VFSFileEOF(f->data.vfs.fileRef) != errNone);
         break;

    default:
         break;
  }

  return result;
}

void
_FileRead(file *fileP, void *buf, uint16 objSize, uint16 numObject, uint16 *objRead)
{
  _PalmFile *f = (_PalmFile *)fileP;
  uint32     bytesRead, bytes;

  switch (f->location)
  {
    case FILE_RAM:
         *objRead = FileRead(f->data.ram.fileH, (void *)buf, (uint16)objSize, (uint16)numObject, NULL);
         break;

    case FILE_CARD:
         bytes = ((uint32)objSize * numObject);
         VFSFileRead(f->data.vfs.fileRef, bytes, (void *)buf, &bytesRead);
         *objRead = bytesRead / objSize;
         break;

    default:
         break;
  }
}

uint16
_FileWrite(file *fileP, void *buf, uint16 objSize, uint16 numObject, int16 *err)
{
  uint16     objWritten = 0;
  _PalmFile *f = (_PalmFile *)fileP;
  uint32     bytesWritten, bytes;

  switch (f->location)
  {
    case FILE_RAM:
         objWritten = FileWrite(f->data.ram.fileH, (void *)buf, (uint16)objSize, (uint16)numObject, (Err *)&err);
         break;

    case FILE_CARD:
         bytes = ((uint32)objSize * numObject);
         VFSFileWrite(f->data.vfs.fileRef, bytes, (void *)buf, &bytesWritten);
         objWritten = bytesWritten / objSize;
         break;

    default:
         break;
  }

  return objWritten;
}

void
_FileClose(file *fileP)
{
  _PalmFile *f = (_PalmFile *)fileP;

  switch (f->location)
  {
    case FILE_RAM:
         FileClose(f->data.ram.fileH);
         break;

    case FILE_CARD:
         VFSFileClose(f->data.vfs.fileRef);
         break;

    default:
         break;
  }

  MemPtrFree(f);
}

/********************************* EOF ***********************************/
