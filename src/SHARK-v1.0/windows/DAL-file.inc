/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)windows/DAL-file.c
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
};

typedef struct _WindowsFile
{
  int    location;
  char   fileName[FILENAME_SIZE];

  union
  {
    struct
    {
      FILE     *file;
    } ram;

  } data;

} _WindowsFile;

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
  _WindowsFile *f;

  f = (_WindowsFile *)malloc(sizeof(_WindowsFile));
  memset(f, 0, sizeof(_WindowsFile));

#ifdef _WIN32_WCE
  sprintf(f->fileName, "/%s/%s/%s", BASEDIR, ApplicationGetName(), fileName);
#else
  sprintf(f->fileName, "\\%s\\%s\\%s", BASEDIR, ApplicationGetName(), fileName);
#endif

  //
  // RAM search
  //

  // ram database
  f->data.ram.file = fopen(f->fileName, "rb");
  if (f->data.ram.file != NULL)
  {
    f->location = FILE_RAM;
    fclose(f->data.ram.file);
  }

  //
  // error handling
  //

  if (f->location == FILE_INVALID)
  {
    free(f); f = NULL;
  }

  return (file *)f;
}

boolean
_FileOpen(file *fileP)
{
  _WindowsFile *f      = (_WindowsFile *)fileP;
  boolean       result = false;

  switch (f->location)
  {
    case FILE_RAM:
         f->data.ram.file = fopen(f->fileName, "rb");
         result = (f->data.ram.file != NULL);
         break;

    default:
         break;
  }

  return result;
}

void
_FileSeek(file *fileP, uint32 ofs, file_position position)
{
  _WindowsFile *f = (_WindowsFile *)fileP;

  switch (f->location)
  {
    case FILE_RAM:

         switch (position)
         {
           case file_beginning:
                fseek(f->data.ram.file, (long)ofs, SEEK_SET);
                break;

           case file_current:
                fseek(f->data.ram.file, (long)ofs, SEEK_CUR);
                break;

           case file_end:
                fseek(f->data.ram.file, (long)ofs, SEEK_END);
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
  _WindowsFile *f      = (_WindowsFile *)fileP;
  uint32        result = 0;

  switch (f->location)
  {
    case FILE_RAM:
         result = (uint32)ftell(f->data.ram.file);
         break;

    default:
         break;
  }

  return result;
}

boolean
_FileEOF(file *fileP)
{
  _WindowsFile *f      = (_WindowsFile *)fileP;
  boolean       result = true;

  switch (f->location)
  {
    case FILE_RAM:
         result = (feof(f->data.ram.file) != 0);
         break;

    default:
         break;
  }

  return result;
}

void
_FileRead(file *fileP, void *buf, uint16 objSize, uint16 numObject, uint16 *objRead)
{
  _WindowsFile *f = (_WindowsFile *)fileP;

  switch (f->location)
  {
    case FILE_RAM:
         *objRead = fread((void *)buf, (size_t)objSize, (size_t)numObject, f->data.ram.file);
         break;

    default:
         break;
  }
}

uint16
_FileWrite(file *fileP, void *buf, uint16 objSize, uint16 numObject, int16 *err)
{
  uint16 objWritten = 0;
  _WindowsFile *f   = (_WindowsFile *)fileP;

  switch (f->location)
  {
    case FILE_RAM:
         objWritten = fwrite((void *)buf, (size_t)objSize, (size_t)numObject, f->data.ram.file);
         break;

    default:
         break;
  }

  return objWritten;
}

void
_FileClose(file *fileP)
{
  _WindowsFile *f = (_WindowsFile *)fileP;

  switch (f->location)
  {
    case FILE_RAM:
         if (f->data.ram.file != NULL)
         {
           fclose(f->data.ram.file);
         }
         break;

    default:
         break;
  }

  free(f);
}

/********************************* EOF ***********************************/
