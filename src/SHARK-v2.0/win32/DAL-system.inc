/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-system.c
 */

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

#define SYS_QUEUE_SIZE            128

#define SYS_SECS_TO_FT_MULT       10000000
#define SYS_DEFAULT_TICKS_PER_SEC 1000      // 1 tick = 10ms

// -----------------------------------------------------------------------
// compile time options
// -----------------------------------------------------------------------

// SYS_USE_WM_TIMER_EVENTS
//
// _TimerProc()
//   -> used for generating nilEvent's for animation generation, there
//      *may* be extra WM_TIMER events posted by the operating system
//      and it could throw out the timing mechanism.
//
//      = 0 - means dont use WM_TIMER events for nilEvent generation
//      = 1 - means use WM_TIMER events for nilEvent generation

   #define SYS_USE_WM_TIMER_EVENTS 0

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

static void   *g_sys_app_globals;          // application globals

static uint32  g_sys_seed;                 // random number seed

static boolean g_sys_nilEvent_todo;        // priority on nilEvent
static uint16  g_sys_event_max;
static uint16  g_sys_event_index;
static event  *g_sys_event_queue;          // event queue functionality

static UINT    g_sys_timerID;              // timer

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static LONGLONG      _SysTimeLong();
static void CALLBACK _TimerProc(UINT id, UINT msg, DWORD dwUser, 
                                DWORD dw1, DWORD dw2);

static boolean  _SysEventAvailable();
static void     _SysFlushEventQueue();
static void     _SysGetEvent(event *e);

// initialize the system manager
static boolean
_SysInitialize()
{
  UINT     timerRes;
  TIMECAPS tc;
  uint16   fps;
  uint32   ticksPerFrame;
  boolean  init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _SysInitialize");
#endif

  // assume everything is ok
  init_ok = true;

  // allocate the memory chunk we need for event handling
  g_sys_event_max   = 0;
  g_sys_event_index = 0;
  g_sys_event_queue = (event *)malloc(SYS_QUEUE_SIZE * sizeof(event));
  init_ok = (g_sys_event_queue != NULL); if (!init_ok) goto SYS_INIT_ABORT;
  memset(g_sys_event_queue, 0, SYS_QUEUE_SIZE * sizeof(event));

  _SysFlushEventQueue();
  _SysRandom(_SysGetTime());

  // install a nilevent timer for required fps
  if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) == TIMERR_NOERROR)
  {
    // whats the desired fps the application wants us to use?
    fps = MAX(100, MIN(g_config.display.fps, 600));  // 10 .. 60 fps = limit

    ticksPerFrame = ((SYS_DEFAULT_TICKS_PER_SEC * 10) / fps);
    timerRes = MIN(MAX(tc.wPeriodMin, ticksPerFrame), tc.wPeriodMax);
    timeBeginPeriod(timerRes);
    g_sys_timerID =
      timeSetEvent(ticksPerFrame, timerRes, _TimerProc, (DWORD)NULL, TIME_PERIODIC);
  }

  // no nilEvent required right now
  g_sys_nilEvent_todo = false;

SYS_INIT_ABORT:

  return init_ok;
}

// get the number of seconds since jan 1, 1970
uint32
_SysGetTime()
{
  return (uint32)(_SysTimeLong() / SYS_SECS_TO_FT_MULT);
}

// get the current system date and time
void
_SysGetDateTime(datetime *dt)
{
  SYSTEMTIME st;

  // win32 local time
  GetLocalTime(&st);

  // copy the information we need into the structure
  dt->year  = st.wYear;
  dt->month = st.wMonth;
  dt->day   = st.wDay;
  dt->hour  = st.wHour;
  dt->min   = st.wMinute;
  dt->sec   = st.wSecond;
}

// wait a period of milliseconds
void
_SysDelay(uint32 millisec)
{
  Sleep(millisec);
}

// display a debug message to the developer
void
_SysDebugMessage(char *msg, boolean dialog)
{
  if (dialog)
    MessageBox(g_hWnd, (const char *)msg, TEXT("SHARK: DEBUG MESSAGE"), MB_OK);
#if (LOG_DISABLE == 0)
  else
   _LOGCreateEntry(msg);
#endif
}

// get some specific device information about the hardware
void
_SysDeviceInfo(uint32 *platform, char *name)
{
  OSVERSIONINFO os_info;
  uint32        platform_id;
  char          name_id[256];

  os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&os_info);

  strcpy(name_id, "Windows [unknown]");
  platform_id = PLATFORM_WIN32 | PROVIDER_MICROSOFT;
  switch (os_info.dwPlatformId)
  {
    case 1:
         switch (os_info.dwMajorVersion)
         {
           case 4:  switch (os_info.dwMinorVersion)
                    {
                      case 0:  strcpy(name_id, "Windows 95");
                               platform_id = DEVICE_WINDOWS_95;
                               break;
                      case 10: strcpy(name_id, "Windows 98");
                               platform_id = DEVICE_WINDOWS_98;
                               break;
                      case 90: strcpy(name_id, "Windows Millennium");
                               platform_id = DEVICE_WINDOWS_ME;
                               break;
                      default: strcpy(name_id, "Windows [unknown]");
                               break;
                    }
                    break;
           default: break;
         }
         break;

    case 2:
         switch (os_info.dwMajorVersion)
         {
           case 3:  strcpy(name_id, "Windows NT 3.51");
                    platform_id = DEVICE_WINDOWS_NT_351;
                    break;
           case 4:  strcpy(name_id, "Windows NT 4.0");
                    platform_id = DEVICE_WINDOWS_NT_4;
                    break;
           case 5:  switch (os_info.dwMinorVersion)
                    {
                      case 0:  strcpy(name_id, "Windows 2000");
                               platform_id = DEVICE_WINDOWS_2000;
                               break;
                      case 1:  strcpy(name_id, "Windows XP");
                               platform_id = DEVICE_WINDOWS_XP;
                               break;
                      case 2:  strcpy(name_id, "Windows Server 2003");
                               platform_id = DEVICE_WINDOWS_2003;
                               break;
                      default: strcpy(name_id, "Windows [unknown]");
                               break;
                    }
                    break;
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

void
_SysEnqueueEvent(event *e)
{
  // emulate timer as best as possible - post nilEvent first
  if (e->eType == _nilEvent)
  {
    g_sys_nilEvent_todo = true;
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
}

void
_SysSetGlobals(void *gbls)
{
  g_sys_app_globals = gbls;
}

void *
_SysGetGlobals()
{
  return g_sys_app_globals;
}

uint16
_SysRandom(uint32 seed)
{
  uint16 result;

  // default return value
  result = (uint16)seed;

  // platform independent random number generator
  if (seed != 0) g_sys_seed = seed;
  g_sys_seed = (0x015A4E35L * g_sys_seed) + 1;
  result = (uint16)(g_sys_seed >> 16);

  return result;
}

// terminate the system management
static void
_SysTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _SysTerminate");
#endif

  // clean up memory usage within memory management system
  if (g_sys_event_queue != NULL) free(g_sys_event_queue);
  g_sys_event_queue = (event *)NULL;

  // terminate the high resolution timer
  timeKillEvent(g_sys_timerID);
}

// get the system time as a LONGLONG
static LONGLONG
_SysTimeLong()
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

  return t;
}

// the timer callback procedure, triggers a _nilEvent
static void CALLBACK
_TimerProc(UINT id, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
#if (SYS_USE_WM_TIMER_EVENTS == 0)
  event e;

  if (g_active)
  {
    memset(&e, 0, sizeof(event));
    e.eType = _nilEvent;
    _SysEnqueueEvent(&e);
  }
#endif

  // post a WM_TIMER event for the event loop to trigger
  PostMessage(g_hWnd, WM_TIMER, 0, 0);
}

// are there any events available for processing?
static boolean
_SysEventAvailable()
{
  boolean result;

  // default return value
  result = (g_sys_nilEvent_todo || (g_sys_event_index != g_sys_event_max));

  return result;
}

// flush the event queue
static void
_SysFlushEventQueue()
{
  g_sys_event_index   = 0;
  g_sys_event_max     = 0;
  g_sys_nilEvent_todo = false;
}

// get the first available event from the event queue
static void
_SysGetEvent(event *e)
{
  // emulate timer as best as possible - post nilEvent first
  if (g_sys_nilEvent_todo)
  {
    memset(e, 0, sizeof(event));
    e->eType = _nilEvent;

    g_sys_nilEvent_todo = false;
  }

  // traditional events
  else
  {
    // get event from queue at current index
    memcpy(e, &g_sys_event_queue[g_sys_event_index], sizeof(event));

    // move index along, or wrap around if needed
    g_sys_event_index++;
    if (g_sys_event_index >= SYS_QUEUE_SIZE) g_sys_event_index = 0;
  }
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-system.inc"

/********************************* EOF ***********************************/
