/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-file.c
 */

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

uint32
_FileGetSize(file *f)
{
  uint32 result;

  // default return value
  result = 0;

  // entry condition
  if (f == NULL) return result;

  // get the size
  if (_FileOpen(f, FILE_READ_ONLY))
  {
    _FileSeek(f, 0, file_end);
    result = _FileTell(f);
    _FileClose(f);
  }

  return result;
}

boolean
_FileLoadIntoBuffer(file *f, void *buffer, uint32 size)
{
  boolean result;
  uint32  cnt;

  // default return value
  result = false;

  // entry conditions
  if ((f == NULL) || (buffer == NULL) || (size < 1)) return result;

  // lets read it in!
  if (_FileOpen(f, FILE_READ_ONLY))
  {
    _FileSeek(f, 0, file_beginning);
    _FileRead(f, buffer, 1, size, &cnt);
    result = (cnt == size);

    _FileClose(f);
  }

  return result;
}

/********************************* EOF ***********************************/
