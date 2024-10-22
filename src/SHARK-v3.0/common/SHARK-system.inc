/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-system.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

#define SYS_QUEUE_SIZE         1024

/*************************************************************************
 * Component Variables
 *************************************************************************/

typedef struct SysGlobals
{
  void *struct_prefix;

  void             *sys_app_globals;

  uint16            sys_nilEvent_todo;
  uint16            sys_event_max;
  uint16            sys_event_index;
  event            *sys_event_queue; 

  boolean           sys_task_active;
  _SysTaskCallback  sys_task_callback; \
  void             *sys_task_user;

#if defined(DAL_SYS_GLOBALS)
  DAL_SYS_GLOBALS
#endif

} SysGlobals;

GLOBALS_DEFINITION;

#define g_sys_app_globals      (SYS_GLOBALS) -> sys_app_globals
#define g_sys_nilEvent_todo    (SYS_GLOBALS) -> sys_nilEvent_todo
#define g_sys_event_max        (SYS_GLOBALS) -> sys_event_max
#define g_sys_event_index      (SYS_GLOBALS) -> sys_event_index
#define g_sys_event_queue      (SYS_GLOBALS) -> sys_event_queue

#define g_sys_task_active      (SYS_GLOBALS) -> sys_task_active
#define g_sys_task_callback    (SYS_GLOBALS) -> sys_task_callback
#define g_sys_task_user        (SYS_GLOBALS) -> sys_task_user

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

static boolean DAL_SysInitialize();
static uint32  DAL_SysConvertDateTimeToSeconds(datetime *);
static void    DAL_SysDebugMessageDialog(char *);
static void    DAL_SysDialogDialog(char *, char *);
static void    DAL_SysDelay(uint32);
static void    DAL_SysDeviceInfo(uint32 *, char *);
static void    DAL_SysGetDateTime(datetime *);
static uint32  DAL_SysGetTime();
static void    DAL_SysTask(sys_event, _SysTaskCallback, void *);
static void    DAL_SysMutexLock();
static void    DAL_SysMutexUnlock();
static void    DAL_SysTerminate();

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
_SysInitialize()
{
  boolean init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _SysInitialize");
#endif

  // allocate memory for globals
  SYS_GLOBALS = malloc(sizeof(SysGlobals));
  if (GLOBALS_UNINITIALIZED) return false;

  // assume everything is ok
  init_ok =  true;
  init_ok &= DAL_SysInitialize();

  // did we initialize ok?
  if (init_ok)
  {
    g_sys_event_max   = 0;
    g_sys_event_index = 0;
    g_sys_event_queue = (event *)malloc(SYS_QUEUE_SIZE * sizeof(event));
    init_ok = (g_sys_event_queue != NULL); if (!init_ok) goto SYS_INIT_DONE;
    memset(g_sys_event_queue, 0, SYS_QUEUE_SIZE * sizeof(event));

    // no nilEvent required right now
    g_sys_nilEvent_todo = 0;
  }

SYS_INIT_DONE:;

  return init_ok;
}

uint32
_SysConvertDateTimeToSeconds(datetime *dt)
{
  uint32 result;

  // default return value
  result = 0;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // query underlying hardware layer
  result = DAL_SysConvertDateTimeToSeconds(dt);

  return result;
}

void
_SysDebugMessage(char *msg, boolean dialog)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // display a dialog
  if (dialog)
    DAL_SysDebugMessageDialog(msg);
  else
  {
#if (LOG_DISABLE == 0)
  // LOGGING EVENT:
  _SHARKLogCreateEntry(msg);
#endif
  }
}

void
_SysDialog(char *title, char *msg)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // display a dialog
  DAL_SysDialogDialog(title, msg);
}

static void
_SysDelay(uint32 millisec)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // query underlying hardware layer
  DAL_SysDelay(millisec);
}

void
_SysDeviceInfo(uint32 *platform, char *name)
{
  // GLOBALS_PROTECTION:
//if (GLOBALS_UNINITIALIZED) return;

  // query underlying hardware layer
  DAL_SysDeviceInfo(platform, name);
}

void
_SysEnqueueEvent(event *e)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // lock the system mutex 
  DAL_SysMutexLock();

  // special nilEvent handling
  if (e -> eType == _nilEvent)
  {
    g_sys_nilEvent_todo++;
  }

  // traditional events
  else
  {
    // save event in queue at 'max' index
    memcpy(&g_sys_event_queue[g_sys_event_max], e, sizeof(event));

    // move index along, or wrap around if needed
    g_sys_event_max++;
    if (g_sys_event_max >= SYS_QUEUE_SIZE) g_sys_event_max = 0;
  }

  // release the lock
  DAL_SysMutexUnlock();
}

static boolean
_SysEventAvailable()
{
  boolean result;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // lock the system mutex 
  DAL_SysMutexLock();

  // is there a nilevent or a real event in the system?
  result  = (g_sys_event_index != g_sys_event_max);
  result |= (g_sys_nilEvent_todo > 0);

  // release the lock
  DAL_SysMutexUnlock();

  return result;
}

void
_SysFlushEventQueue(int16 eType)
{
  int i; 

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // lock the system mutex 
  DAL_SysMutexLock();

  // process all ourstanding events
  i = g_sys_event_index;
  while (i != g_sys_event_max)
  {
    // if we find an event of the requested type; flush it out
    if (g_sys_event_queue[i].eType == eType)
      g_sys_event_queue[i].eType = _nilEvent;

    // move index along, or wrap around if needed
    i++;
    if (i >= SYS_QUEUE_SIZE) i = 0;
  }

  // release the lock
  DAL_SysMutexUnlock();
}

void
_SysGetDateTime(datetime *dt)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // query underlying hardware layer
  DAL_SysGetDateTime(dt);
}

static boolean
_SysGetEvent(event *e)
{
  boolean result;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // lock the system mutex 
  DAL_SysMutexLock();

  // do we have a nilEvent to pass through?
  if (g_sys_nilEvent_todo > 0)
  {
    memset(e, 0, sizeof(event));
    e -> eType = _nilEvent;
    e -> data.nil.outstanding = g_sys_nilEvent_todo - 1;
    g_sys_nilEvent_todo = 0;

    // we have an event :) we can post it
    result = true;
  }
  else

  // do we have an event to pass through?
  if (g_sys_event_index != g_sys_event_max)
  {
    // we may have removed an event from the queue (flush)
    if (g_sys_event_queue[g_sys_event_index].eType != _nilEvent)
    {
      // get event from queue at current index
      memcpy(e, &g_sys_event_queue[g_sys_event_index], sizeof(event));

      // we have an event :) we can post it
      result = true;
    }

    // move index along, or wrap around if needed
    g_sys_event_index++;
    if (g_sys_event_index >= SYS_QUEUE_SIZE) g_sys_event_index = 0;
  }

  // release the lock
  DAL_SysMutexUnlock();

  return result;
}

void *
_SysGetGlobals()
{
  void *result;

  // default return value
  result = NULL;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  result = g_sys_app_globals;

  return result;
}

uint32
_SysGetTime()
{
  uint32 result;

  // default return value
  result = 0;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // query underlying hardware layer
  result = DAL_SysGetTime();

  return result;
}

void
_SysSetGlobals(void *gbls)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  g_sys_app_globals = gbls;
}

void
_SysTask(sys_event id, _SysTaskCallback callback, void *user)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry requirements
  if (callback == NULL) return;

  // query underlying hardware layer
  DAL_SysTask(id, callback, user);
}

static void
_SysTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _SysTerminate");
#endif

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  DAL_SysTerminate();

  // free memory for globals
  free(SYS_GLOBALS); 
  SYS_GLOBALS = NULL;
}

#undef SYS_QUEUE_SIZE

/********************************* EOF ***********************************/
