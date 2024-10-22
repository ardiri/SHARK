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

#define SYS_SECS_TO_FT_MULT       10000000

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define SYS_GLOBALS           g_sys
#define GLOBALS_UNINITIALIZED (SYS_GLOBALS == NULL)
#define GLOBALS_DEFINITION    SysGlobals *g_sys = NULL

// SysGlobals::   platform additions
#define DAL_SYS_GLOBALS  \
  HANDLE            sys_task_thread;    \
  DWORD             sys_task_thread_id; \
                                        \
  CRITICAL_SECTION  sys_mutex;

#define g_sys_task_thread     g_sys -> sys_task_thread
#define g_sys_task_thread_id  g_sys -> sys_task_thread_id

#define g_sys_mutex           g_sys -> sys_mutex

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-system.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static DWORD WINAPI DAL_SysTaskThread(void *);

static boolean
DAL_SysInitialize()
{
  boolean  init_ok;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(SYS_GLOBALS, 0, sizeof(SysGlobals));

  // initialize the system mutex
  InitializeCriticalSection(&g_sys_mutex);

  return init_ok;
}

static uint32
DAL_SysConvertDateTimeToSeconds(datetime *dt)
{
  LONGLONG      t, tbase;
  SYSTEMTIME    st;
  FILETIME      ft;
  LARGE_INTEGER li;

  // copy the information we need into the structure
  st.wYear   = dt -> year;
  st.wMonth  = dt -> month;
  st.wDay    = dt -> day;
  st.wHour   = dt -> hour;
  st.wMinute = dt -> min;
  st.wSecond = dt -> sec;

  // current time
  SystemTimeToFileTime(&st, &ft);
  li.LowPart  = ft.dwLowDateTime;
  li.HighPart = ft.dwHighDateTime;
  t           = li.QuadPart;

  // 1st Jan 1970
  memset(&st, 0, sizeof(st));
  st.wYear  = 1970;
  st.wMonth = 1;
  st.wDay   = 1;
  SystemTimeToFileTime(&st, &ft);
  li.LowPart  = ft.dwLowDateTime;
  li.HighPart = ft.dwHighDateTime;
  tbase       = li.QuadPart;

  // difference between the two times?
  t -= tbase;

  return (uint32)(t / SYS_SECS_TO_FT_MULT);
}

static void
DAL_SysDebugMessageDialog(char *msg)
{
  DAL_SysDialogDialog("SHARK: DEBUG MESSAGE", msg);
}

static void
DAL_SysDialogDialog(char *title, char *msg)
{
  MSGBOXPARAMS mbp;

  // define the message box parameters
  mbp.cbSize             = sizeof(MSGBOXPARAMS);
  mbp.hwndOwner          = g_hWnd;
  mbp.hInstance          = GetModuleHandle(NULL);
  mbp.lpszText           = msg;
  mbp.lpszCaption        = title;
  mbp.dwStyle            = MB_OK | MB_USERICON;
  mbp.dwLanguageId       = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
  mbp.lpfnMsgBoxCallback = NULL;
  mbp.dwContextHelpId    = 0;
  mbp.lpszIcon           = MAKEINTRESOURCE(101);  // IDI_DEFAULT_ICON

  // pop up the message box
  MessageBoxIndirect(&mbp);
//MessageBox(g_hWnd, (const char *)msg, (const char *)title, MB_OK);
}

static void
DAL_SysDelay(uint32 millisec)
{
  Sleep(millisec);
}

static void
DAL_SysDeviceInfo(uint32 *platform, char *name)
{
  OSVERSIONINFO os_info;
  uint32        platform_id;
  char          name_id[256];

  os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  GetVersionEx(&os_info);

  strcpy(name_id, "Windows [unknown]");
  platform_id = PLATFORM_WINDOWS;

  switch (os_info.dwPlatformId)
  {
    case 1:
         switch (os_info.dwMajorVersion)
         {
           case 4:  switch (os_info.dwMinorVersion)
                    {
                      case 0:  strcpy(name_id, "Windows 95");
                               break;
                      case 10: strcpy(name_id, "Windows 98");
                               break;
                      case 90: strcpy(name_id, "Windows Millennium");
                               break;
                      default: break;
                    }
                    break;
           default: break;
         }

    case 2:
         switch (os_info.dwMajorVersion)
         {
           case 3:  strcpy(name_id, "Windows NT 3.51");
                    break;
           case 4:  strcpy(name_id, "Windows NT 4.0");
                    break;
           case 5:  switch (os_info.dwMinorVersion)
                    {
                      case 0:  strcpy(name_id, "Windows 2000");
                               break;
                      case 1:  strcpy(name_id, "Windows XP");
                               break;
                      case 2:  strcpy(name_id, "Windows Server 2003");
                               break;
                      default: break;
                    }
                    break;
/**
           case 5:  switch (os_info.dwMinorVersion)
                    {
                      case 0:  if (os_info.wProductType == 0x0000001) 
                                 strcpy(name_id, "Windows Vista");
                               else
                                 strcpy(name_id, "Windows Server 2008");
                               break;
                      case 1:  if (os_info.wProductType == 0x0000001) 
                                 strcpy(name_id, "Windows 7");
                               else
                                 strcpy(name_id, "Windows Server 2008 R2");
                               break;
                      default: break;
                    }
                    break;
 **/
           default: break;
         }
         break;

    default:
         break;
  }

  // any extra information? service patches etc?
  if (os_info.szCSDVersion[0] != '\0')
  {
    strcat(name_id, ": ");
    strcat(name_id, os_info.szCSDVersion);
  }

  if (platform != NULL) *platform = platform_id;
  if (name != NULL)     strcpy(name, name_id);
}

static void
DAL_SysGetDateTime(datetime *dt)
{
  SYSTEMTIME st;

  // win32 local time
  GetLocalTime(&st);

  // copy the information we need into the structure
  dt -> year  = st.wYear;
  dt -> month = st.wMonth;
  dt -> day   = st.wDay;
  dt -> hour  = st.wHour;
  dt -> min   = st.wMinute;
  dt -> sec   = st.wSecond;
}

static uint32
DAL_SysGetTime()
{
  LONGLONG      t, tbase;
  SYSTEMTIME    st;
  FILETIME      ft;
  LARGE_INTEGER li;

  // current time
  GetSystemTime(&st);
  SystemTimeToFileTime(&st, &ft);
  li.LowPart  = ft.dwLowDateTime;
  li.HighPart = ft.dwHighDateTime;
  t           = li.QuadPart;

  // 1st Jan 1970
  memset(&st, 0, sizeof(st));
  st.wYear  = 1970;
  st.wMonth = 1;
  st.wDay   = 1;
  SystemTimeToFileTime(&st, &ft);
  li.LowPart  = ft.dwLowDateTime;
  li.HighPart = ft.dwHighDateTime;
  tbase       = li.QuadPart;

  // difference between the two times?
  t -= tbase;

  return (uint32)(t / SYS_SECS_TO_FT_MULT);
}

static void
DAL_SysTask(sys_event id, _SysTaskCallback callback, void *user)
{
  // if the previous task is still active - kill it
  if (g_sys_task_active)
  {
    TerminateThread(g_sys_task_thread, 0);
    CloseHandle(g_sys_task_thread);
    g_sys_task_thread = NULL;

    // it is no longer active
    g_sys_task_active = false;
  }

  // are we creating a new thread?
  if (callback != NULL)
  {
    g_sys_task_callback = callback;
    g_sys_task_user     = user;

    // create the background thread
    g_sys_task_thread  = 
      (HANDLE)CreateThread(NULL, 0, DAL_SysTaskThread,
                           (void *)NULL, 0, &g_sys_task_thread_id);

    // now the thread is active
    g_sys_task_active = true;
  }
}

static DWORD WINAPI 
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

  return 0;
}

static void
DAL_SysMutexLock()
{
  EnterCriticalSection(&g_sys_mutex);
}

static void
DAL_SysMutexUnlock()
{
  LeaveCriticalSection(&g_sys_mutex);
}

static void 
DAL_SysTerminate()
{
  // stop any background task that is running
  DAL_SysTask(0, NULL, NULL);

  // destroy the system mutex
  DeleteCriticalSection(&g_sys_mutex);
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef SYS_GLOBALS

#undef SYS_SECS_TO_FT_MULT

/********************************* EOF ***********************************/
