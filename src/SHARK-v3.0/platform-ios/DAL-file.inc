/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-file.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

#define MKDIR_MODE (S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#define mkdir(s) mkdir(s, MKDIR_MODE)

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define FILE_GLOBALS          g_file
#define GLOBALS_UNINITIALIZED (FILE_GLOBALS == NULL)
#define GLOBALS_DEFINITION    FileGlobals *g_file = NULL

// _file:: platform additions
#define DAL_FILE \
  FILE      *fp;

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-file.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
DAL_FileInitialize()
{
  boolean init_ok;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(FILE_GLOBALS, 0, sizeof(FileGlobals));

  return init_ok;
}

static void
DAL_FileClose(_file *f)
{
  // entry requirement
  if (f -> fp == NULL) return;

  // close the file
  fflush(f -> fp);
  fclose(f -> fp);

  f -> fp = NULL;
}

static boolean
DAL_FileCreate(uint32 disk, file *dir, char *name, uint32 attributes)
{
  boolean  result;
  FILE    *f;
  char     path[MAX_PATH_LENGTH], cwd[MAX_PATH_LENGTH];

  // default return value
  result = false;

  // we must save the current working directory 
  getcwd(cwd, MAX_PATH_LENGTH);

  // obtain absolute file path
  _FileVolumeChange(disk);
  _FileBuildLocalPath(dir, name, path);

  switch (attributes)
  {
    case FILE_STANDARD:

         // does the file already exist?
         f = fopen(path, "r");
         if (f != NULL)
         {
           fclose(f);
           goto FILE_CREATE_DONE; 
         }

         // create the file
         f = fopen(path, "w");
         if (f != NULL)
         {
           fclose(f);
           result = true;
         }
         break;

    case FILE_DIRECTORY:

         // does the directory already exist?
         if (chdir(path) == 0) goto FILE_CREATE_DONE; 

         // create the directory
         result = (mkdir(path) == 0);
         break;
  }

FILE_CREATE_DONE:;

  // restore the volume
  _FileVolumeRestore();

  // restore the current working directory 
  chdir(cwd);

  return result;
}

static boolean
DAL_FileDelete(_file *f)
{
  boolean result;
  char    path[MAX_PATH_LENGTH];

  // default return value
  result = false;

  // obtain absolute file path
  _FileVolumeChange(f -> disk);
  _FileBuildLocalPath((file *)f -> parent, f -> name, path);

  // directory?
  if (f -> attributes == FILE_DIRECTORY)
    result = (rmdir(path) == 0);

  // normal file
  else
    result = (remove(path) == 0);

  // restore the volume
  _FileVolumeRestore();

  return result;
}

static void
DAL_FileEnumeration(uint32 disk, file *dir)
{
  DIR           *d;
  struct dirent *dp;
  uint32         index;    
  char           path[MAX_PATH_LENGTH];

  // obtain absolute file path
  _FileVolumeChange(disk);
  _FileBuildLocalPath(dir, NULL, path);

  d = opendir(path);
  if (d != NULL)
  {
    dp = readdir(d);

    index = 0;
    while ((dp != NULL) && (index < MAX_ENUMERATION_ENTRIES))
    {
      // lets ignore the '.' and '..' directories
      if (dp -> d_name[0] != '.')
      {
        strncpy(g_file_fileName[index], dp -> d_name, MAX_FILENAME_LENGTH);
        if ((dp -> d_type & DT_DIR) != 0)
          g_file_fileAttributes[index] = FILE_DIRECTORY;
        else
          g_file_fileAttributes[index] = FILE_STANDARD;
        index++;
      }
      dp = readdir(d);
    }
    closedir(d);

    g_file_fileCount = index;
  }

  // restore the volume
  _FileVolumeRestore();
}

static boolean
DAL_FileEOF(_file *f)
{
  boolean result;

  // default return value
  result = false;

  // entry requirement
  if (f -> fp == NULL) return result;

  result = feof(f -> fp);

  return result;
}

static boolean
DAL_FileGetHandle(_file *f)
{
  boolean result;
  FILE   *_f;
  uint32  attributes;
  char    path[MAX_PATH_LENGTH], cwd[MAX_PATH_LENGTH];

  // default return value
  result = false;

  // we must save the current working directory 
  getcwd(cwd, MAX_PATH_LENGTH);

  // obtain absolute file path
  _FileVolumeChange(f -> disk);
  _FileBuildLocalPath((file *)f -> parent, f -> name, path);

  // is it a directory?
  if (chdir(path) == 0)
    attributes = FILE_DIRECTORY;

  // is it a normal file?
  else
  {
    _f = fopen(path, "r");
    if (_f != NULL)
    { 
      fclose(_f);
      attributes = FILE_STANDARD;
    }
    else
      goto FILE_GET_HANDLE_EXIT;
  }

  // initialize the correct properties
  f -> attributes = attributes;
  f -> fp         = NULL;

  // we now have a valid file or directory
  result = true;

FILE_GET_HANDLE_EXIT:;

  // restore the volume
  _FileVolumeRestore();

  // restore the current working directory 
  chdir(cwd);

  return result;
}

static boolean
DAL_FileLocatePath(char *dirName, uint32 area, char *name, boolean create)
{
  boolean  result;
  char     cwd[MAX_PATH_LENGTH];

  // default return value
  result = false;

  // we must save the current working directory 
  getcwd(cwd, MAX_PATH_LENGTH);

  // executable path (basic)
  // 
  // .../iPhone/program.app/application

  // which area are we looking within?
  switch (area)
  {
    case PATH_PROGRAM:
    case PATH_COMMON:

         // program directory (platform specific)
         strcpy(dirName, g_path);

         result = true;
         break;
 
    case PATH_EXTERNAL_MEDIA:
 
         // there is no external media on the iPhone
         result = false;

         break;

    //
    // internal use only
    //

    case PATH_RESOURCE:

         // within program directory 
         strcpy(dirName, g_path);
         result = true;
         break;

    case PATH_PREFERENCE:

         // there is no capability to create preference files on the iPhone
         result = false;

    default:
         goto FILE_LOCATE_PATH_DONE;
         break;
  }

  // lets validate the directory exists
  result = (chdir(dirName) == 0);
  if (!result) goto FILE_LOCATE_PATH_DONE;

  // do we need a directory within the area?
  if (name != NULL)
  {
    // try to open this path
    sprintf(dirName, "%s%c%s", dirName, DIRECTORY_SEPARATOR, name); 
    result = (chdir(dirName) == 0);

    // special case: create directory if we have to
    if ((!result) && create)
      result = (mkdir(dirName) == 0);
  }

FILE_LOCATE_PATH_DONE:;

  // restore the current working directory 
  chdir(cwd);

  return result;
}

static boolean
DAL_FileOpen(_file *f, uint32 mode)
{
  boolean result;
  char    path[MAX_PATH_LENGTH];

  // default return value
  result = false;

  // obtain absolute file path
  _FileVolumeChange(f -> disk);
  _FileBuildLocalPath((file *)f -> parent, f -> name, path);
 
  // open the file
  switch (mode)
  {
    case FILE_READ_ONLY:   f -> fp = fopen(path, "rb");  break;
    case FILE_READ_WRITE:  f -> fp = fopen(path, "rb+"); break;
    case FILE_READ_WRITE | 
         FILE_CLOBBER:     f -> fp = fopen(path, "wb+"); break;
  }

  // restore the volume
  _FileVolumeRestore();

  // if we have a file handle, the file is open
  result = (f -> fp != NULL);

  return result;
}

static error
DAL_FileRead(_file *f, void *buf, uint32 size, uint32 *cnt)
{
  error result;
  int amt;

  // default return value
  result = -1;

  // entry requirement
  if (f -> fp == NULL) return result;

  // do the file read
  amt = fread((void *)buf, 1, (size_t)size, (FILE *)f -> fp);
  if (cnt != NULL) *cnt = (uint32)amt;
  result = errNone;

  return result;
}

static void
DAL_FileReleaseHandle(_file *f)
{
}

static boolean
DAL_FileRename(uint32 disk, file *dir, char *old_name, char *new_name)
{
  boolean  result;
  char     patho[MAX_PATH_LENGTH], pathn[MAX_PATH_LENGTH];
  char     cwd[MAX_PATH_LENGTH];

  // default return value
  result = false;

  // we must save the current working directory 
  getcwd(cwd, MAX_PATH_LENGTH);

  // obtain absolute file paths for both files
  _FileVolumeChange(disk);
  _FileBuildLocalPath(dir, old_name, patho);
  _FileBuildLocalPath(dir, new_name, pathn);

  // perform the rename
  result = (rename(patho, pathn) == 0); 

  // restore the volume
  _FileVolumeRestore();

  // restore the current working directory 
  chdir(cwd);

  return result;
}

static void
DAL_FileSeek(_file *f, int32 ofs, file_position position)
{
  // entry requirement
  if (f -> fp == NULL) return;

  switch (position)
  {
    case file_beginning: fseek((FILE *)f -> fp, (long)ofs, SEEK_SET); break;
    case file_current:   fseek((FILE *)f -> fp, (long)ofs, SEEK_CUR); break;
    case file_end:       fseek((FILE *)f -> fp, (long)ofs, SEEK_END); break;
    default: break;
  }
}

static uint32
DAL_FileTell(_file *f)
{
  uint32  result;

  // default return value
  result = 0;

  // entry requirement
  if (f -> fp == NULL) return result;

  result = (uint32)ftell((FILE *)f -> fp);

  return result;
}

static error
DAL_FileWrite(_file *f, void *buf, uint32 size, uint32 *cnt)
{
  error result;
  int   amt;

  // default return value
  result = -1;

  // entry requirement
  if (f -> fp == NULL) return result;

  // do the file write
  amt = fwrite((void *)buf, 1, (size_t)size, (FILE *)f -> fp);
  if (cnt != NULL) *cnt = (uint32)amt;
  result = errNone;

  return result;
}

void 
DAL_FileTerminate()
{
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef FILE_GLOBALS

#undef mkdir
#undef MKDIR_MODE

/********************************* EOF ***********************************/
