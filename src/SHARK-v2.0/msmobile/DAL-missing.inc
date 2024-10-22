/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-missing.inc
 */

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static char *
strrev(char *str)
{
  int32 cnt, len;
  char *tmp, *p;

  if (str != NULL)
  {
    len = strlen(str);
    tmp = (char *)malloc(len+1);

    // copy the string in the reverse order
    cnt = len;
    p = tmp; str += len;
    do
    {
      *p++ = *--str;
    } while (--cnt);
    *p = 0;

    // copy the new string over the old string
    strcpy(str, tmp);
    free(tmp);
  }

  return str;
}

static int32
rename(char *src, char *dst)
{ 
  int32  result;
  uint32 size;
  uint16 srcU[MAX_PATH], dstU[MAX_PATH];

  result = -1;

  // ensure a clean slate
  memset(srcU, 0, (MAX_PATH << 1));
  memset(dstU, 0, (MAX_PATH << 1));

  // remove the file
  size = strlen(src); 
  mbstowcs(srcU, src, size); 
  size = strlen(dst);
  mbstowcs(dstU, dst, size); 
  if (DeleteAndRenameFile(dstU, srcU) != 0) result = 0;

  return result;
}

static int32
remove(char *path)
{ 
  int32  result;
  uint32 size;
  uint16 pathU[MAX_PATH];

  result = -1;

  // ensure a clean slate
  memset(pathU, 0, (MAX_PATH << 1));

  // remove the file
  size = strlen(path);
  mbstowcs(pathU, path, size); 
  if (DeleteFile(pathU) != 0) result = 0;

  return result;
}

static int32
chdir(char *path)
{
  int32           result;
  uint32          size;
  WIN32_FIND_DATA finddata; 
  HANDLE          hFile; 
  uint16          pathU[MAX_PATH];

  result = -1;

  // ensure a clean slate
  memset(pathU, 0, (MAX_PATH << 1));

  // lets see if it it really a directory
  size = strlen(path);
  mbstowcs(pathU, path, size); 

  // lets try and find the file
  hFile = FindFirstFile(pathU, &finddata);
  if (hFile != INVALID_HANDLE_VALUE)
  { 
    if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      result = 0;

    FindClose(hFile);
  }
    
  return result;
}

static int32
mkdir(char *path)
{ 
  int32  result;
  uint32 size;
  uint16 pathU[MAX_PATH];

  result = -1;

  // ensure a clean slate
  memset(pathU, 0, (MAX_PATH << 1));

  // remove the file
  size = strlen(path);
  mbstowcs(pathU, path, size); 
  if (CreateDirectory(pathU, NULL) != 0) result = 0;

  return result;
}

static int32
rmdir(char *path)
{ 
  int32  result;
  uint32 size;
  uint16 pathU[MAX_PATH];

  result = -1;

  // ensure a clean slate
  memset(pathU, 0, (MAX_PATH << 1));

  // remove the file
  size = strlen(path);
  mbstowcs(pathU, path, size); 
  if (RemoveDirectory(pathU) != 0) result = 0;

  return result;
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

/********************************* EOF ***********************************/
