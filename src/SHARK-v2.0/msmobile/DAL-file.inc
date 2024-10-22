/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-file.c
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

typedef struct _file
{
  char   name[MAX_FILENAME_LENGTH];
  uint32 attributes;       // file attributes (directory/file)
  uint32 disk;             // the disk the file exists on

  uint32 useCount;         // DIRECTORY: use count (for memory disposal)
  struct _file *parent;

  FILE  *fp;               // internal file pointer
} _file;

typedef struct _disk
{
  char   name[MAX_FILENAME_LENGTH];
  char   path[MAX_PATH_LENGTH];
} _disk;

static int32   g_active_disk;
static _disk   g_disks[MAX_DISKS];
static uint32  g_diskCount;
static uint32  g_diskCountSystem;

static char    g_enumerationsName[MAX_ENUMERATION_ENTRIES][MAX_FILENAME_LENGTH];
static uint32  g_enumerationsAttributes[MAX_ENUMERATION_ENTRIES];
static uint32  g_enumerationCount;

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static void _FileVolumeChange(uint32 disk);
static void _FileBuildLocalPath(file *dir, char *name, char *path);
static void _FileVolumeRestore();

// initialize the file manager
static boolean
_FileInitialize()
{
  boolean init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _FileInitialize");
#endif

  // initialize disk management variables
  g_active_disk     = -1;
  g_diskCount       = 0;
  g_diskCountSystem = 0;
  memset(g_disks, 0, MAX_DISKS * sizeof(_disk));

  // assume everything is ok
  init_ok = true;

  return init_ok;
}

// define a disk for use at a certain mount point
boolean
_FileDefineDisk(char *name, char *path, uint32 mode)
{
  boolean result;
  uint32  disk, len;
  char    *p, *b, tmp[MAX_PATH_LENGTH], dirName[MAX_PATH_LENGTH];

  // default return value
  result = false;

  // entry condition - has disk already been defined?
  if (_FileLocateDisk(name, NULL)) return result;

  // lets make sure we have enough room for another disk
  if ((g_diskCount + g_diskCountSystem) < MAX_DISKS)
  {
    if (path == NULL)
      sprintf(dirName, "%s%c", g_path, DIRECTORY_SEPARATOR);
    // '\\' defined root?
    else
    if (path[0] == DIRECTORY_SEPARATOR)
      sprintf(dirName, path);

    // '..\\' defined (upper directory)
    else
    if ((path[0] == '.') && (path[1] == '.'))
    {
      p = path;
      strcpy(tmp, g_path);
      b = tmp;

      // this should handle recursivly
      result = strchr(p, DIRECTORY_SEPARATOR) && strchr(b, DIRECTORY_SEPARATOR);
      while (result)
      {
        p = strchr(p, DIRECTORY_SEPARATOR) + 1;
        *((char *)(strrchr(b, DIRECTORY_SEPARATOR))) = 0;

        result = ((p[0] == '.') && (p[1] == '.') &&
                   strchr(p, DIRECTORY_SEPARATOR) &&
                   strchr(b, DIRECTORY_SEPARATOR));
      }

      // have a valid directory now?
      if (p[0] != '.')
        sprintf(dirName, "%s%c%s", b, DIRECTORY_SEPARATOR, p);
    }
    // lets generate the path - based on the current directory
    else
      sprintf(dirName, "%s%c%s", g_path, DIRECTORY_SEPARATOR, path);

    // we must ensure the last char is not a "\"
    len = strlen(dirName);
    while ((len > 0) && (dirName[len-1] == DIRECTORY_SEPARATOR))
    {
      dirName[len-1] = 0;
      len = strlen(dirName);
    }

    // do we need to create the directory? (only if DISK_CLOBBER bit set)
    result = (chdir(dirName) == 0);
    if (!result && (mode & DISK_CLOBBER))
      result = (mkdir(dirName) == 0);

    // were we able to verify the existance of the directory
    if (result)
    {
      if (mode & DISK_SYSTEM)
        disk = MAX_DISKS - 1 - g_diskCountSystem++;
      else
        disk = g_diskCount++;

      strcpy(g_disks[disk].name, name);
      strcpy(g_disks[disk].path, dirName);  // keep the path!
    }
  }

  return result;
}

boolean
_FileLocateDisk(char *name, uint32 *disk)
{
  boolean result;
  uint32  i;

  // default return value
  result = false;

  // entry conditions
  if (name == NULL) return result;

  // lets see if the disk has already been defined (go through all drives)
  i = 0;
  while (i < MAX_DISKS)
  {
    // do we have a match?
    if (strcmp(name, g_disks[i].name) == 0)
    {
      if (disk != NULL) *disk = i;
      result = true;
    }

    i++;
  }

  return result;
}

// enumerate through all the possible disks
boolean
_DiskEnumerate(iterator *it, uint32 *disk, char *name)
{
  boolean result;

  // default return value
  result = false;

  // entry conditions
  if ((it == NULL) || (disk == NULL)) return result;

  // move through our disks
  if ((uint32)*it < g_diskCount)
  {
    *disk = *it;
    if (name != NULL) strcpy(name, g_disks[*it].name);

    // next iteration
    *it    = (iterator)(*it + 1);
    result = true;
  }
  else
    *it = iterator_stop;

  return result;
}

// enumerate through all the files in a specific directory
boolean
_FileEnumerate(uint32 disk, file *dir,
               iterator *it, char *name, uint32 *attributes)
{
  WIN32_FIND_DATA finddata;
  HANDLE          hFile;
  boolean         result;
  uint32          size;
  char            path[MAX_PATH_LENGTH];
  uint16          pathU[MAX_PATH_LENGTH];

  // default return value
  result = false;

  // entry requirements
  if ((it == NULL) ||
      ((dir != NULL) &&
        (((_file *)dir)->disk != disk) &&
        (((_file *)dir)->attributes != FILE_DIRECTORY))) return result;

  // move through our disks
  if ((uint32)*it == iterator_start)
  {
    g_enumerationCount = 0;

    // lets generate the absolute path to this directory
    _FileVolumeChange(disk);
    _FileBuildLocalPath(dir, "*", path);

    // we need to reverse the string; as it isnt correct
    strrev(path);
    size = strlen(path);
    memset(pathU, 0, (MAX_PATH_LENGTH << 1));
    mbstowcs(pathU, path, size); 

    hFile = FindFirstFile(pathU, &finddata);
    if (hFile != INVALID_HANDLE_VALUE)
    {
      do
      {
        // lets ignore the '.' and '..' directories
        if (finddata.cFileName[0] != '.')
        {
          // make a copy of the filename (do wchar -> char conversion)
          wcstombs(g_enumerationsName[g_enumerationCount], 
                   finddata.cFileName, MAX_FILENAME_LENGTH);

          if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            g_enumerationsAttributes[g_enumerationCount] = FILE_DIRECTORY;
          else
            g_enumerationsAttributes[g_enumerationCount] = FILE_STANDARD;
          g_enumerationCount++;
        }
      } while (FindNextFile(hFile, &finddata) &&
               (g_enumerationCount < MAX_ENUMERATION_ENTRIES));

      FindClose(hFile);
    }

    // restore the old directory 
    _FileVolumeRestore();
  }

  // move through our enumeration results
  if ((uint32)*it < g_enumerationCount)
  {
    if (name       != NULL) strcpy(name, g_enumerationsName[*it]);
    if (attributes != NULL) *attributes = g_enumerationsAttributes[*it];

    // next iteration
    *it    = (iterator)(*it + 1);
    result = true;
  }
  else
    *it = iterator_stop;

  return result;
}

// get a file handle to a file in a specific directory
file *
_FileGetHandle(uint32 disk, file *dir, char *name)
{
  _file *result;
  _file *_f;
  uint32 attributes;
  FILE  *f;
  char   path[MAX_PATH_LENGTH];

  // default return value
  result = NULL;

  // entry requirement
  if ((dir != NULL) &&
       (((_file *)dir)->disk != disk) &&
       (((_file *)dir)->attributes != FILE_DIRECTORY))
    return (file *)result;

  // basic validation checks
  if ((name != NULL) && (strcmp(name, "") != 0))
  {
    // lets generate the absolute path to this file
    _FileVolumeChange(disk);
    _FileBuildLocalPath(dir, name, path);

    // we need to reverse the string; as it isnt correct
    strrev(path);

    // is it a directory
    if (chdir(path) == 0)
      attributes = FILE_DIRECTORY;
    else
    {
      // is it a normal file?
      f = fopen(path, "r");
      if (f != NULL)
      {
        fclose(f);
        attributes = FILE_STANDARD;
      }

      // its neither; so, we have no handle
      else
        goto FILE_GET_HANDLE_EXIT;
    }

    // basic properties
    result = (_file *)_MemPtrNew(sizeof(_file), false);
    if (result != NULL)
    {
      result->disk       = disk;
      result->attributes = attributes;
      result->useCount   = 0;
      result->parent     = (_file *)dir;
      strcpy(result->name, name);
      result->fp         = NULL;

      // if we are dealing with directory; we need to inform its being used
      if (dir != NULL)
      {
        _f = (_file *)dir;
        _f->useCount++;
      }
    }

FILE_GET_HANDLE_EXIT:

    // ensure main path is valid
    _FileVolumeRestore();
  }

  return (file *)result;
}

// release the file handle
void
_FileReleaseHandle(file *f)
{
  _file *_f = (_file *)f;

  // entry condition
  if (_f == NULL) return;

  // ensure file closed and release resource used (if right usecount)
  if (_f->useCount == 0)
  {
    if (_f->parent != NULL)
    {
      _f = (_file *)_f->parent;
      _f->useCount--;
    }

    _FileClose((file *)f);
    _MemPtrFree(f);
  }
}

// create a file in a specific directory
boolean
_FileCreate(uint32 disk, file *dir, char *name, uint32 attributes)
{
  boolean  result;
  FILE    *f;
  char     path[MAX_PATH_LENGTH];

  // default return value
  result = false;

  // entry requirement
  if ((dir != NULL) &&
       (((_file *)dir)->disk != disk) &&
       (((_file *)dir)->attributes != FILE_DIRECTORY)) return result;

  // basic validation checks
  if ((name != NULL) && (strcmp(name, "") != 0))
  {
    // lets generate the absolute path to this file
    _FileVolumeChange(disk);
    _FileBuildLocalPath(dir, name, path);

    // we need to reverse the string; as it isnt correct
    strrev(path);

    // now what - directory or normal file?
    switch (attributes)
    {
      case FILE_STANDARD:

           // does the file already exist?
           f = fopen(path, "r");
           if (f != NULL)
           {
             fclose(f);
             goto FILE_CREATE_EXIT; // the file already exists
           }

           // create the file
           f = fopen(path, "w");
           if (f != NULL)
           {
             fclose(f);
             result = true;
           }
           else
             goto FILE_CREATE_EXIT; // unable to create the file

           break;

      case FILE_DIRECTORY:

           // does the directory already exist?
           if (chdir(path) == 0)
             goto FILE_CREATE_EXIT; // the directory already exists

           // create the file
           if (mkdir(path) == 0)
             result = true;
           else
             goto FILE_CREATE_EXIT; // unable to create the directory

           break;

      default:
           goto FILE_CREATE_EXIT;
           break;
    }

FILE_CREATE_EXIT:

    // ensure main path is valid
    _FileVolumeRestore();
  }

  return result;
}

// rename a file in a specific directory
boolean
_FileRename(uint32 disk, file *dir, char *old_name, char *new_name)
{
  boolean  result;
  FILE    *fp;
  char     path_new[MAX_PATH_LENGTH], path_old[MAX_PATH_LENGTH];

  // default return value
  result = false;

  // entry requirement
  if ((dir != NULL) &&
       (((_file *)dir)->disk != disk) &&
       (((_file *)dir)->attributes == FILE_DIRECTORY)) return result;

  // basic validation checks
  if ((old_name != NULL) && (strcmp(old_name, "") != 0) &&
      (new_name != NULL) && (strcmp(new_name, "") != 0))
  {
    // lets generate the absolute path to this file
    _FileVolumeChange(disk);
    _FileBuildLocalPath(dir, old_name, path_old);
    _FileBuildLocalPath(dir, new_name, path_new);

    // we need to reverse the string; as it isnt correct
    strrev(path_old);
    strrev(path_new);

    // rename the file (if it exists), and new name doesn't exit
    fp = fopen(path_old, "r");
    if (fp != NULL)
    {
      fclose(fp);
      fp = fopen(path_new, "r");
      if (fp != NULL)
        fclose(fp);  // destination exists? barf
      else
        result = (rename(path_old, path_new) == 0);
    }

    // ensure main path is valid
    _FileVolumeRestore();
  }

  return result;
}

// delete the file defined by the file handle
boolean
_FileDelete(file *f)
{
  boolean  result;
  _file   *_f = (_file *)f;
  char     path[MAX_PATH_LENGTH];

  // default return value
  result = false;

  // have a valid file reference?
  if (_f != NULL)
  {
    // lets generate the absolute path to this file
    _FileVolumeChange(_f->disk);
    _FileBuildLocalPath((file *)_f->parent, _f->name, path);

    // we need to reverse the string; as it isnt correct
    strrev(path);

    // directory?
    if (_f->attributes == FILE_DIRECTORY)
    {
      // if the directory exists; lets try and remove it
      if (chdir(path) != 0)
        result = (rmdir(path) == 0);
    }
    else
    if (_f->attributes == FILE_STANDARD)
    {
      // is the file open? close it first
      if (_f->fp != NULL)
      {
        fflush((FILE *)_f->fp);
        fclose((FILE *)_f->fp);
        _f->fp = NULL;
      }

      // remove the file (if it exists)
      _f->fp = fopen(path, "r");
      if (_f->fp)
      {
        fclose(_f->fp);
        result = (remove(path) == 0);
      }
    }

    // ensure main path is valid
    _FileVolumeRestore();
  }

  return result;
}

// open the file defined by the file handle using appropriate access mode
boolean
_FileOpen(file *f, uint32 mode)
{
  boolean  result;
  _file   *_f = (_file *)f;
  char     path[MAX_PATH_LENGTH];

  // default return value
  result = false;

  // are we dealing with a valid handle here?
  if (_f != NULL)
  {
    // lets generate the absolute path to this file
    _FileVolumeChange(_f->disk);
    _FileBuildLocalPath((file *)_f->parent, _f->name, path);

    // we need to reverse the string; as it isnt correct
    strrev(path);

    // now what - directory or normal file?
    switch (mode)
    {
      case FILE_READ_ONLY:                 _f->fp = fopen(path, "rb");  break;
      case FILE_READ_WRITE:                _f->fp = fopen(path, "rb+"); break;
      case FILE_READ_WRITE | FILE_CLOBBER: _f->fp = fopen(path, "wb+"); break;
      default: goto FILE_OPEN_EXIT; break;
    }
    result = (_f->fp != NULL);

FILE_OPEN_EXIT:

    // ensure main path is valid
    _FileVolumeRestore();
  }

  return result;
}

// end of file?
boolean
_FileEOF(file *f)
{
  boolean  result;
  _file   *_f = (_file *)f;

  // default return value
  result = true;

  // do we have a valid file reference
  if ((_f != NULL) && (_f->fp != NULL))
    result = (feof((FILE *)_f->fp) != 0);

  return result;
}

// move the file pointer to a specific position in the file
void
_FileSeek(file *f, int32 ofs, file_position position)
{
  _file *_f = (_file *)f;

  // entry condition
  if ((_f == NULL) || (_f->fp == NULL)) return;

  switch (position)
  {
    case file_beginning: fseek((FILE *)_f->fp, (long)ofs, SEEK_SET); break;
    case file_current:   fseek((FILE *)_f->fp, (long)ofs, SEEK_CUR); break;
    case file_end:       fseek((FILE *)_f->fp, (long)ofs, SEEK_END); break;
    default: break;
  }
}

// obtain the file pointer position
uint32
_FileTell(file *f)
{
  uint32  result;
  _file  *_f = (_file *)f;

  // default return value
  result = 0;

  // do we have a valid file reference?
  if ((_f != NULL) && (_f->fp != NULL))
    result = (uint32)ftell((FILE *)_f->fp);

  return result;
}

// read a chunk of information from a file into a buffer
void
_FileRead(file *f, void *buf, uint32 objSize, uint32 numObject, uint32 *objRead)
{
  _file *_f = (_file *)f;
  uint32 cnt;

  // entry condition
  if ((_f == NULL) || (_f->fp == NULL)) return;

  // do the file read
  cnt = (uint32)fread((void *)buf, (size_t)objSize, (size_t)numObject, (FILE *)_f->fp);
  if (objRead != NULL) *objRead = cnt;
}

// write a chunk of information from a buffer to the file
uint32
_FileWrite(file *f, void *buf, uint32 objSize, uint32 numObject)
{
  uint32  result;
  _file  *_f = (_file *)f;

  // default return value
  result = 0;

  // do we have a valid file reference
  if ((_f != NULL) && (_f->fp != NULL))
    result = fwrite((void *)buf, (size_t)objSize, (size_t)numObject, (FILE *)_f->fp);

  return result;
}

// ensure the file cache is empty, all data written to file
void
_FileFlush(file *f)
{
  _file *_f = (_file *)f;

  // do we have a valid file reference?
  if ((_f != NULL) && (_f->fp != NULL))
  {
    // flush contents of the file
    fflush((FILE *)_f->fp);
  }
}

// close the file
void
_FileClose(file *f)
{
  _file *_f = (_file *)f;

  // do we have a valid file reference?
  if ((_f != NULL) && (_f->fp != NULL))
  {
    // close the file
    fflush((FILE *)_f->fp);
    fclose((FILE *)_f->fp);
    _f->fp = NULL;
  }
}

// return the last known file based error
uint32
_FileGetLastError()
{
  // not implemented yet
  return 0;
}

// get the attributes of a given file handle
void
_FileGetAttributes(file *f, uint32 *disk, file **dir,
                   char *path, char *name, uint32 *attributes)
{
  _file *_f = (_file *)f;
  char  *p;

  // entry condition
  if (_f == NULL) return;

  if (disk != NULL)       *disk       = _f->disk;
  if (dir  != NULL)       *dir        = (file *)_f->parent;
  if (name != NULL)        strcpy(name, _f->name);
  if (attributes != NULL) *attributes = _f->attributes;
  if (path != NULL)
  {
    p = (char *)path;
    _FileBuildLocalPath((file *)_f->parent, _f->name, path);
    p += strlen(path);

    *p++ = DIRECTORY_SEPARATOR;
    *p = 0; // terminate the string

    // now, we must reverse the complete path for it to be right
    strrev(path);
  }
}

// given a path, obtain the file handle to a specific file
file *
_FileGetHandleFromPath(uint32 disk, char *path)
{
  _file   *result;
  _file   *f, *dir;
  boolean directory;
  char    *p, _path[MAX_FILENAME_LENGTH];

  // default return value
  result = NULL;

  // verification
  if ((path != NULL) && (strlen(path) > 7))
  {
    // make a local copy of the path (as we modify it)
    strcpy(_path, path);
    p = _path;

    if (p[0] == DIRECTORY_SEPARATOR)
    {
      p++;

      // do we have a DIRECTORY_SEPARATOR in the path?
      if (strchr(p, DIRECTORY_SEPARATOR))
      {
        *((char *)(strchr(p, DIRECTORY_SEPARATOR))) = 0;
        f = (_file *)_FileGetHandle(disk, NULL, p);  // directory in root
        if (f != NULL)
        {
          p += strlen(p) + 1;
          while (*p != 0)
          {
            directory = false;
            if (strchr(p, DIRECTORY_SEPARATOR))
            {
              directory = true;  // we are doing a little hack here
              *((char *)(strchr(p, DIRECTORY_SEPARATOR))) = 0;
            }

            dir = f;
            f   = (_file *)_FileGetHandle(disk, (file *)dir, p); // file in dir
            if (f == NULL)
            {
              _FileReleaseHandleRecursive((file *)dir);
              goto GET_HANDLE_PATH_DONE;
            }

            p += strlen(p);
            if (directory) p++; else break;
          }
        }

        // the last known file; is the file we want to access now
        result = f;
      }
      else
        result = (_file *)_FileGetHandle(disk, NULL, p);  // file in root
    }
  }

GET_HANDLE_PATH_DONE:

  return (file *)result;
}

// release a file's handle, releasing all parent relations if possible
void
_FileReleaseHandleRecursive(file *f)
{
  _file *_f = (_file *)f;
  _file *dir;

  // entry condition
  if (_f == NULL) return;

  // iterate through the heirarchy; cleaning up file handles
  dir = _f->parent;
  while (dir != NULL)
  {
    _FileReleaseHandle((file *)_f);
    _f   = dir;
    dir  = _f->parent;
  }
  _FileReleaseHandle((file *)_f);
}

// terminate the system management
static void
_FileTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _FileTerminate");
#endif
}

// change the disk, current directory
static void
_FileVolumeChange(uint32 disk)
{
  g_active_disk = disk;
}

// build a local file path to be used under disk directory
static void
_FileBuildLocalPath(file *dir, char *name, char *path)
{
  _file *_f;
  char  *p;
  char   p_name[MAX_FILENAME_LENGTH];

  // lets generate the absolute path to this file
  p = (char *)path;
  if (name != NULL)
  {
    strcpy(p_name, name); strrev(p_name);  // use local buffer instead
    sprintf(p, "%s%c", p_name, DIRECTORY_SEPARATOR);
    p += strlen(name) + 1;
  }

  if (dir != NULL)
  {
    _f = (_file *)dir;

    do
    {
      strcpy(p, strrev(_f->name)); strrev(_f->name); // back to normal
      p += strlen(_f->name);

      if (_f->attributes == FILE_DIRECTORY)
      {
        *p++ = DIRECTORY_SEPARATOR;
        _f = _f->parent;
      }
      else
        _f = NULL; // sanity checking
    }
    while (_f != NULL);
  }

  // msmobile doesn't have ability to change local directory, always absolute
  if (g_active_disk != -1)
  {
    strcpy(p, strrev(g_disks[g_active_disk].path)); 
    strrev(g_disks[g_active_disk].path); // back to normal
    p += strlen(g_disks[g_active_disk].path);
  }
  else
    p--; // lets remove the "/"

  *p = 0;
}

// restore the current directory to system original
static void
_FileVolumeRestore()
{
  g_active_disk = -1;
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-file.inc"

/********************************* EOF ***********************************/
