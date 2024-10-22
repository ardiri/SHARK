/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-system.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define SYS_GLOBALS           g_sys
#define GLOBALS_UNINITIALIZED (SYS_GLOBALS == NULL)
#define GLOBALS_DEFINITION    SysGlobals *g_sys = NULL

// SysGlobals::   platform additions
#define DAL_SYS_GLOBALS  \
  pthread_t         sys_task_thread; \
                                     \
  pthread_mutex_t   sys_mutex; 

#define g_sys_task_thread      (SYS_GLOBALS) -> sys_task_thread

#define g_sys_mutex            (SYS_GLOBALS) -> sys_mutex

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-system.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static void *DAL_SysTaskThread(void *);

static boolean
DAL_SysInitialize()
{
  boolean init_ok;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(SYS_GLOBALS, 0, sizeof(SysGlobals));

  // initialize the system mutex
  pthread_mutex_init(&g_sys_mutex, NULL);

  return init_ok;
}

static uint32
DAL_SysConvertDateTimeToSeconds(datetime *dt)
{
  struct tm st;

  // copy the information we need into the structure
  st.tm_year = dt -> year  - 1900;
  st.tm_mon  = dt -> month - 1;
  st.tm_mday = dt -> day;
  st.tm_hour = dt -> hour;
  st.tm_min  = dt -> min;
  st.tm_sec  = dt -> sec;

  return (uint32)mktime(&st);
}

static void
DAL_SysDebugMessageDialog(char *msg)
{
  DAL_SysDialogDialog("SHARK DEBUG MESSAGE", msg);
}

static void
DAL_SysDialogDialog(char *title, char *msg)
{
}

static void
DAL_SysDelay(uint32 millisec)
{
  usleep(millisec * 1000L);
}

static void
DAL_SysDeviceInfo(uint32 *platform, char *name)
{
  uint32            platform_id;
  char              name_id[256];
  deviceinfo_data_t data;

  strcpy(name_id, "QNX [unknown]");
  platform_id = PLATFORM_QNX;

  // query the operating system for information
  deviceinfo_get_data(&data);

  // build the operation system version string
  sprintf(name_id, "%s [%s]", data.device_os, data.scm_bundle);

  // release the resources used in this request
  deviceinfo_free_data(&data);

  if (platform != NULL) *platform = platform_id;
  if (name != NULL)     strcpy(name, name_id);
}

static void
DAL_SysGetDateTime(datetime *dt)
{
  time_t     now;
  struct tm *st;

  // system system time
  time(&now);
  st = localtime(&now);

  // copy the information we need into the structure
  dt -> year  = st -> tm_year + 1900;
  dt -> month = st -> tm_mon  + 1;
  dt -> day   = st -> tm_mday;
  dt -> hour  = st -> tm_hour;
  dt -> min   = st -> tm_min;
  dt -> sec   = st -> tm_sec;
}

static uint32
DAL_SysGetTime()
{
  return (uint32)time(NULL);
}

static void
DAL_SysTask(sys_event id, _SysTaskCallback callback, void *user)
{
  // if the previous task is still active - kill it
  if (g_sys_task_active)
  {
    pthread_cancel(g_sys_task_thread);
    pthread_join(g_sys_task_thread, NULL);

    // it is no longer active
    g_sys_task_active = false;
  }

  // are we creating a new thread?
  if (callback != NULL)
  {
    g_sys_task_callback = callback;
    g_sys_task_user     = user;

    // create the background thread
    pthread_create(&g_sys_task_thread, NULL, DAL_SysTaskThread, (void *)id);

    // now the thread is active
    g_sys_task_active = true;
  }
}

static void *
DAL_SysTaskThread(void *data)
{
  event e;

  // we can only execute a callback that is defined
  if (g_sys_task_callback != NULL)
  {
    // execute the user task
    g_sys_task_callback(g_sys_task_user);

    // post user event, indicating job is done
    memset(&e, 0, sizeof(event));
    e.eType = (sys_event)data;
    _SysEnqueueEvent(&e);
  }

  // now the task is no longer active
  g_sys_task_active = false;

  return NULL;
}

static void
DAL_SysMutexLock()
{
  pthread_mutex_lock(&g_sys_mutex);
}

static void
DAL_SysMutexUnlock()
{
  pthread_mutex_unlock(&g_sys_mutex);
}

static void 
DAL_SysTerminate()
{
  // stop any background task that is running
  DAL_SysTask(0, NULL, NULL);

  // destroy the system mutex
  pthread_mutex_destroy(&g_sys_mutex);
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef SYS_GLOBALS

/********************************* EOF ***********************************/
