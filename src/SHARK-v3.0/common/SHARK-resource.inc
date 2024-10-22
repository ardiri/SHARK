/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-resource.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

#define RES_DISK_ID        "-RESOURCE-"

#define FILE_READ_CHUNK_SIZE  8192

/*************************************************************************
 * Component Variables
 *************************************************************************/

typedef struct ResGlobals
{
  void *struct_prefix;

  uint32   res_disk;

#if defined(DAL_RES_GLOBALS)
  DAL_RES_GLOBALS
#endif

} ResGlobals;

GLOBALS_DEFINITION;

#define g_res_disk             (RES_GLOBALS) -> res_disk  

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

static boolean DAL_ResourceInitialize();
static void   *DAL_ResourceLoad(resource id);
static uint32  DAL_ResourceSize(resource id);
static void    DAL_ResourceTerminate();

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
_ResourceInitialize()
{
  boolean init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _ResourceInitialize");
#endif

  // allocate memory for globals
  RES_GLOBALS = malloc(sizeof(ResGlobals));
  if (GLOBALS_UNINITIALIZED) return false;

  // assume everything is ok
  init_ok =  true;
  init_ok &= DAL_ResourceInitialize();

  // if everything is ok, define "alternative" resource directory
  if (init_ok)
  {
    _FileDefineDisk(RES_DISK_ID, PATH_RESOURCE, NULL, DISK_SYSTEM);
    g_res_disk = _FileLocateDisk(RES_DISK_ID); 
  }

  return init_ok;
}

void *
_ResourceLock(resource id)
{
  void    *result;
  char     name[MAX_FILENAME_LENGTH];
  file    *f;
  uint32   size;
  uint32   cnt, num, amt, max;
  char    *tmp, *ptr;

  // default return value
  result = NULL;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;
 
  // do we have a resource disk defined?
  if (g_res_disk != DISK_INVALID)
  {
    // find: xxxx.bin or xxxx.xxx
    sprintf(name, "xxxx%04x.bin", id);
    f = _FileGetHandle(g_res_disk, NULL, name);
    if (f == NULL)
      sprintf(name, "xxxx%04x.xxx", id);

    f = _FileGetHandle(g_res_disk, NULL, name);
    if (f != NULL)
    {
      if (_FileOpen(f, FILE_READ_ONLY))
      {
        _FileSeek(f, 0, file_end);
        size = _FileTell(f);
        _FileSeek(f, 0, file_beginning);

        result = _MemPtrNew(size, true);
        if (result != NULL)
        {
          // try to read the file contents
          max = size;
          tmp = _MemPtrNew(MIN(FILE_READ_CHUNK_SIZE, max), false);
          if (tmp != NULL)
          {
            amt = cnt = 0;
            ptr = result;
            while (max > 0)
            {
              num = MIN(FILE_READ_CHUNK_SIZE, max);
              _FileRead(f, tmp, num, &amt);
              if (amt != num) break;

              _MemSemaphore(true);
              _MemMove(ptr, tmp, num); ptr += num; max -= num;
              _MemSemaphore(false);

              cnt += amt;
            }

            // release our temporary buffer
            _MemPtrFree(tmp);

            // was the resource loaded correctly?
            if (cnt != size) 
            {
              _MemPtrFree(result); 
              result = NULL;
            }
          }
        }  
        _FileClose(f);
      }

      _FileReleaseHandle(f);
    }

    if (result != NULL) goto RESOURCE_LOCK_DONE;
  }
 
  // query the underlying hardware platform
  result = DAL_ResourceLoad(id);

RESOURCE_LOCK_DONE:;

  return result;
}

void
_ResourceUnlock(void *p)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // free the memory used by the resource
  _MemPtrFree(p);
}

uint32
_ResourceSize(resource id)
{
  uint32 result;
  char   name[MAX_FILENAME_LENGTH];
  file  *f;

  // default return value
  result = 0;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;
 
  // do we have a resource disk defined?
  if (g_res_disk != DISK_INVALID)
  {
    // find: xxxx.bin or xxxx.xxx
    sprintf(name, "xxxx%04x.bin", id);
    f = _FileGetHandle(g_res_disk, NULL, name);
    if (f == NULL)
      sprintf(name, "xxxx%04x.xxx", id);

    f = _FileGetHandle(g_res_disk, NULL, name);
    if (f != NULL)
    {
      if (_FileOpen(f, FILE_READ_ONLY))
      {
        _FileSeek(f, 0, file_end);
        result = _FileTell(f);
        _FileClose(f);
      }
      _FileReleaseHandle(f);

      goto RESOURCE_SIZE_DONE;
    }
  }

  // query the underlying hardware platform
  result = DAL_ResourceSize(id);

RESOURCE_SIZE_DONE:;

  return result;
}

static void
_ResourceTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _ResourceTerminate");
#endif

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  DAL_ResourceTerminate();

  // free memory for globals
  free(RES_GLOBALS); 
  RES_GLOBALS = NULL;
}

#undef FILE_READ_CHUNK_SIZE
#undef RES_DISK_ID

/********************************* EOF ***********************************/
