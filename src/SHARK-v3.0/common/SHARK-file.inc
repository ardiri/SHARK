/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-file.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

typedef struct _file
{
  uint32 disk;
  uint32 attributes;
  char   name[MAX_FILENAME_LENGTH];

  uint32 useCount;
  struct _file *parent;

#if defined(DAL_FILE)
  DAL_FILE
#endif

} _file;

typedef struct _disk
{
  char  name[MAX_FILENAME_LENGTH];
  char  path[MAX_PATH_LENGTH];

#if defined(DAL_DISK)
  DAL_DISK
#endif
} _disk;

typedef struct FileGlobals
{
  void *struct_prefix;

  uint32  file_active_disk;

  uint32  file_diskCount;
  uint32  file_diskCountSys;
  _disk   file_disks[MAX_DISKS];

  uint32  file_fileCount;
  char    file_fileName[MAX_ENUMERATION_ENTRIES][MAX_FILENAME_LENGTH];
  uint32  file_fileAttributes[MAX_ENUMERATION_ENTRIES];

  uint32  file_diskIndex;
  uint32  file_fileIndex;  // used by enumeration code

#if defined(DAL_FILE_GLOBALS)
  DAL_FILE_GLOBALS
#endif

} FileGlobals;

GLOBALS_DEFINITION;

#define g_file_active_disk     (FILE_GLOBALS) -> file_active_disk

#define g_file_disks           (FILE_GLOBALS) -> file_disks
#define g_file_diskCount       (FILE_GLOBALS) -> file_diskCount
#define g_file_diskCountSys    (FILE_GLOBALS) -> file_diskCountSys

#define g_file_fileCount       (FILE_GLOBALS) -> file_fileCount
#define g_file_fileName        (FILE_GLOBALS) -> file_fileName
#define g_file_fileAttributes  (FILE_GLOBALS) -> file_fileAttributes

#define g_file_diskIndex       (FILE_GLOBALS) -> file_diskIndex
#define g_file_fileIndex       (FILE_GLOBALS) -> file_fileIndex

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

static boolean  DAL_FileInitialize();
static void     DAL_FileClose(_file *);
static boolean  DAL_FileCreate(uint32, file *, char *, uint32);
static boolean  DAL_FileDelete(_file *);
static void     DAL_FileEnumeration(uint32, file *);
static boolean  DAL_FileEOF(_file *);
static boolean  DAL_FileGetHandle(_file *);
static boolean  DAL_FileLocatePath(char *, uint32, char *, boolean);
static boolean  DAL_FileOpen(_file *, uint32);
static error    DAL_FileRead(_file *, void *, uint32, uint32 *);
static void     DAL_FileReleaseHandle(_file *);
static boolean  DAL_FileRename(uint32, file *, char *, char *);
static void     DAL_FileSeek(_file *, int32, file_position);
static uint32   DAL_FileTell(_file *);
static error    DAL_FileWrite(_file *, void *, uint32, uint32 *);
static void     DAL_FileTerminate();

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
_FileInitialize()
{
  boolean init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _FileInitialize");
#endif

  // allocate memory for globals
  FILE_GLOBALS = malloc(sizeof(FileGlobals));
  if (GLOBALS_UNINITIALIZED) return false;

  // assume everything is ok
  init_ok =  true;
  init_ok &= DAL_FileInitialize();

  // did we initialize ok?
  if (init_ok)
  {
    // initialize disk management variables
    g_file_diskCount    = 0;
    g_file_diskCountSys = 0;
    memset(g_file_disks, 0, MAX_DISKS * sizeof(_disk));

    g_file_active_disk  = DISK_INVALID;
  }

  return init_ok;
}

boolean
_DiskEnumerate(iterator *it, uint32 *disk, char *name)
{
  boolean result;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirement
  if ((it == NULL) || (disk == NULL)) return result;

  // move through our disks
  switch (*it)
  {
    case iterator_start:

         // activate the iteration 
         g_file_diskIndex = 0;
         *it = iterator_active;

    case iterator_active:

         // are there more entries to iterate through
         if (g_file_diskIndex < g_file_diskCount)
         {
           // get the disk index and name if required
           *disk = g_file_diskIndex;
           if (name != NULL) strcpy(name, g_file_disks[*disk].name);

           // next iteration
           g_file_diskIndex++;
           result = true;
         }

         // no more entries, terminate iteration
         else
           *it = iterator_active;

         break;

    case iterator_stop:
         // this should never be called!
         break;
  }

  return result;
}

boolean
_FileDefineDisk(char *id, uint32 area, char *name, uint32 mode)
{
  boolean result;
  uint32  disk;
  boolean create;
  char    dirName[MAX_PATH_LENGTH];

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirement - has disk already been defined?
  if (_FileLocateDisk(id) != DISK_INVALID) return result;

  // lets make sure we have enough room for another disk
  if ((g_file_diskCount + g_file_diskCountSys) < MAX_DISKS)
  {
    // try to locate the area we need
    create = ((mode & DISK_CLOBBER) != 0);
    if (DAL_FileLocatePath(dirName, area, name, create))
    {
      switch (DISK_TYPE(mode))
      {
        case DISK_BASIC:
             disk = g_file_diskCount++;
             break;

        case DISK_SYSTEM:
             disk = MAX_DISKS - ++g_file_diskCountSys;
             break;

        default:
             goto FILE_DEFINE_DISK_DONE;
             break;
      }  

      // store the path for future use
      strcpy(g_file_disks[disk].name, id);
      strcpy(g_file_disks[disk].path, dirName); 
      result = true;
    }
  }

FILE_DEFINE_DISK_DONE:;

  return result;
}

uint32
_FileLocateDisk(char *id)
{
  uint32 result;
  uint32 i;

  // default return value
  result = DISK_INVALID;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirement
  if (id == NULL) return result;

  // lets see if the disk has already been defined (go through all drives)
  i = 0;
  while (i < MAX_DISKS)
  {
    // do we have a match?
    if (strcmp(id, g_file_disks[i].name) == 0) result = i;
    i++;
  }

  return result;
}

static void
_FileBuildLocalPath(file *dir, char *name, char *path)
{
  _file  *_f;
  char   *p;
  char    p_name[MAX_FILENAME_LENGTH];
  char    p_path[MAX_PATH_LENGTH];

  // entry requirement
  if (path == NULL) return;

  // lets ensure the string is empty
  strcpy(path, "");

  // lets generate the absolute path to this file
  p = (char *)path;
  if (name != NULL)
  {
    strcpy(p_name, name); strrev(p_name);
    sprintf(p, "%s%c", p_name, DIRECTORY_SEPARATOR);
    p += strlen(name) + 1;
  }

  // is this file part of a directory? we must show the path
  if (dir != NULL)
  {
    _f = (_file *)dir;
    do
    {
      strcpy(p_name, _f -> name); strrev(p_name);
      strcpy(p, p_name);
      p += strlen(_f -> name);

      if (_f -> attributes == FILE_DIRECTORY)
      {
        *p++ = DIRECTORY_SEPARATOR;
        _f = _f -> parent;
      }
      else
        _f = NULL; // sanity checking
    }
    while (_f != NULL);
  }

  // lets include the path from the disk
  if (g_file_active_disk != DISK_INVALID)
  {
    strcpy(p_path, g_file_disks[g_file_active_disk].path); strrev(p_path);
    strcpy(p, p_path);
    p += strlen(p_path);
  }
  else
    (*--p) = 0; // should never happen!

  // we need to reverse the path to its correct form
  strrev(path);
}

void
_FileClose(file *f)
{
  _file *_f = (_file *)f;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry requirement: we cannot work on directories
  if ((_f == NULL) || (_f -> attributes == FILE_DIRECTORY)) return;

  // pass to underlying hardware engine
  DAL_FileClose(_f);
}

boolean
_FileCreate(uint32 disk, file *dir, char *name, uint32 attributes)
{
  boolean result;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirement
  if (name == NULL) return result;

  // entry requirement:: we cannot have a directory separator in name
  if (strchr(name, DIRECTORY_SEPARATOR) != NULL) return result;

  // query underlying hardware engine
  result = DAL_FileCreate(disk, dir, name, attributes);

  return result;
}

boolean
_FileDelete(file *f)
{
  boolean   result;
  _file    *_f = (_file *)f;
  iterator  it;
  char      filename[MAX_FILENAME_LENGTH];
  file     *e;
  boolean   ok;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirements
  if (_f == NULL) return result;

  // directory? - we need to recursively zap everything
  if (_f -> attributes == FILE_DIRECTORY)
  {
    it = iterator_start;  
    ok = _FileEnumerate(_f -> disk, (file *)_f, &it, filename, NULL);
    while (ok && (it != iterator_stop))
    {
      // get a handle to the file in the directory
      e = _FileGetHandle(_f -> disk, (file *)_f, filename);
      if (e != NULL)
      {
        // delete it (recursively)
        _FileDelete(e);
        _FileReleaseHandle(e);
      }
 
      // process the next item
      ok = _FileEnumerate(_f -> disk, (file *)_f, &it, filename, NULL);
    }
  }

  // lets ensure the file is closed
  _FileClose((file *)_f);

  // query underlying hardware engine
  result = DAL_FileDelete(_f);

  return result;
}

boolean
_FileEnumerate(uint32 disk, file *dir,
               iterator *it, char *name, uint32 *attributes)
{
  boolean result;
  uint32  idx;
  _file  *_d = (_file *)dir;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirement
  if ((it == NULL) || (disk == DISK_INVALID)) return result;

  // entry requirement #2: check dir is a directory
  if ((_d != NULL) && (_d -> attributes != FILE_DIRECTORY)) return result;

  // move through our disks
  switch (*it)
  {
    case iterator_start:

         // query the underlying platform layer 
         DAL_FileEnumeration(disk, dir);

         // activate the iteration 
         g_file_fileIndex = 0;
         *it = iterator_active;

    case iterator_active:

         // are there more entries to iterate through
         if (g_file_fileIndex < g_file_fileCount)
         {
           // get the file index, name and attributes if required
           idx = g_file_fileIndex;
           if (name       != NULL) strcpy(name, g_file_fileName[idx]);
           if (attributes != NULL) *attributes = g_file_fileAttributes[idx];

           // next iteration
           g_file_fileIndex++;
           result = true;
         }

         // no more entries, terminate iteration
         else
           *it = iterator_active;

         break;

    case iterator_stop:
         // this should never be called!
         break;
  }

  return result;
}

boolean
_FileEOF(file *f)
{
  boolean result;
  _file *_f = (_file *)f;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirement: we cannot work on directories
  if ((_f == NULL) || (_f -> attributes == FILE_DIRECTORY)) return result;

  // query underlying hardware engine
  result = DAL_FileEOF(_f);

  return result;
}

void
_FileGetAttributes(file *f, uint32 *disk,
                   file **dir, char *name, uint32 *attributes)
{
  _file *_f = (_file *)f;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry requirement
  if (_f == NULL) return;

  if (disk != NULL)       *disk       = _f -> disk;
  if (dir  != NULL)       *dir        = (file *)_f -> parent;
  if (name != NULL)        strcpy(name, _f -> name);
  if (attributes != NULL) *attributes = _f -> attributes;
}

file *
_FileGetHandle(uint32 disk, file *dir, char *name)
{
  _file *result;
  _file *_f;

  // default return value
  result = NULL;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return (file *)result;

  // entry requirement
  if ((disk == DISK_INVALID) ||
      ((dir != NULL) &&
       (((_file *)dir) -> disk != disk) &&
       (((_file *)dir) -> attributes != FILE_DIRECTORY)))
    return (file *)result;

  // entry requirement:: we cannot have a directory separator in name
  if (strchr(name, DIRECTORY_SEPARATOR) != NULL) return (file *)result;

  // basic properties
  result = (_file *)malloc(sizeof(_file));
  if (result != NULL)
  {
    memset(result, 0, sizeof(_file));

    result -> disk       = disk;
    result -> useCount   = 0;
    result -> parent     = (_file *)dir;
    strcpy(result -> name, name);

    // if we are dealing with directory; increment use count
    if (dir != NULL)
    {
      _f = (_file *)dir;
      _f -> useCount++;
    }

    // query underlying hardware 
    if (!DAL_FileGetHandle(result))
    {
      free(result);
      result = NULL;
    }
  }

  return (file *)result;
}

boolean
_FileOpen(file *f, uint32 mode)
{
  boolean result;
  _file *_f = (_file *)f;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirement: we cannot work on directories
  if ((_f == NULL) || (_f -> attributes == FILE_DIRECTORY)) return result;

  // query underlying hardware engine
  result = DAL_FileOpen(_f, mode);

  return result;
}

error
_FileRead(file *f, void *buf, uint32 size, uint32 *cnt)
{
  error result;
  _file *_f = (_file *)f;

  // default return value
  result = -1;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirement: we cannot work on directories
  if ((_f == NULL) || (_f -> attributes == FILE_DIRECTORY)) return result;

  // pass to underlying hardware engine
  result = DAL_FileRead(_f, buf, size, cnt);

  return result;
}

void
_FileReleaseHandle(file *f)
{
  _file *_f = (_file *)f;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry requirement
  if (_f == NULL) return;

  // we can only release a resource when it is no longer being used
  if (_f -> useCount == 0)
  {
    if (_f -> parent != NULL)
    {
      _f = (_file *)_f->parent;
      _f -> useCount--;
    }

    // lets ensure the file is closed
    _FileClose(f);

    // pass to underlying hardware engine
    DAL_FileReleaseHandle(_f);

    free(f);
  }
  else
  {
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry("[ERROR]\t_FileReleaseHandle() - usage count not zero");
#endif
  }
}

boolean
_FileRename(uint32 disk, file *dir, char *old_name, char *new_name)
{
  boolean  result;
  file    *fo, *fn;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirements
  if (((old_name != NULL) && (strcmp(old_name, "") == 0)) ||
      ((new_name != NULL) && (strcmp(new_name, "") == 0))) return result;

  // lets ensure the old_name file exists and new_name does not
  fo = _FileGetHandle(disk, dir, old_name);
  if (fo != NULL)
  {
    fn = _FileGetHandle(disk, dir, new_name);
    if (fn != NULL)
      _FileReleaseHandle(fn);
    else 
      result = true;

    _FileReleaseHandle(fo);
  }

  // query underlying hardware engine
  if (result)
    result = DAL_FileRename(disk, dir, old_name, new_name);

  return result;
}

void
_FileSeek(file *f, int32 ofs, file_position position)
{
  _file *_f = (_file *)f;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry requirement: we cannot work on directories
  if ((_f == NULL) || (_f -> attributes == FILE_DIRECTORY)) return;

  // pass to underlying hardware engine
  DAL_FileSeek(_f, ofs, position);
}

uint32
_FileTell(file *f)
{
  uint32 result;
  _file *_f = (_file *)f;

  // default return value
  result = 0;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirement: we cannot work on directories
  if ((_f == NULL) || (_f -> attributes == FILE_DIRECTORY)) return result;

  // query underlying hardware engine
  result = DAL_FileTell(_f);

  return result;
}

error 
_FileWrite(file *f, void *buf, uint32 size, uint32 *cnt)
{
  error result;
  _file *_f = (_file *)f;

  // default return value
  result = -1;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry requirement: we cannot work on directories
  if ((_f == NULL) || (_f -> attributes == FILE_DIRECTORY)) return result;

  // query underlying hardware engine
  result = DAL_FileWrite(_f, buf, size, cnt);

  return result;
}

static void
_FileVolumeChange(uint32 disk)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry requirements
  if (disk == DISK_INVALID) return;

  g_file_active_disk = disk;
}

static void
_FileVolumeRestore()
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  g_file_active_disk = DISK_INVALID;
}

static void
_FileTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _FileTerminate");
#endif

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  DAL_FileTerminate();

  // free memory for globals
  free(FILE_GLOBALS); 
  FILE_GLOBALS = NULL;
}

/********************************* EOF ***********************************/
