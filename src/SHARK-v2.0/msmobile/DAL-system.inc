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

#define DEFAULT_TIMER_ID          1
#define MAX_STRING_BUFFER         128

// -----------------------------------------------------------------------
// compile time options
// -----------------------------------------------------------------------

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

static LONGLONG _SysTimeLong();

static boolean  _SysEventAvailable();
static void     _SysFlushEventQueue();
static void     _SysGetEvent(event *e);

// initialize the system manager
static boolean
_SysInitialize()
{
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

  // whats the desired fps the application wants us to use?
  fps = MAX(100, MIN(g_config.display.fps, 600));  // 10 .. 60 fps = limit

  ticksPerFrame = ((SYS_DEFAULT_TICKS_PER_SEC * 10) / fps);
  g_sys_timerID = 
    SetTimer(g_hWnd, DEFAULT_TIMER_ID, ticksPerFrame, NULL);

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

  // msmobile local time
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
  uint16 *buf;
  uint32 size;

  if (dialog)
  {
    size = strlen(msg) + 1;

    buf = malloc(size << 1);
    memset(buf, 0, (size << 1));
    mbstowcs(buf, msg, strlen(msg)); 
    MessageBox(g_hWnd, buf, TEXT("SHARK: DEBUG MESSAGE"), MB_OK);
    free(buf);
  }
#if (LOG_DISABLE == 0)
  else
   _LOGCreateEntry(msg);
#endif
}

// get some specific device information about the hardware
void
_SysDeviceInfo(uint32 *platform, char *name)
{
  WCHAR         szManufacturer[MAX_STRING_BUFFER];
  TCHAR         szPlatform[MAX_STRING_BUFFER];
  OSVERSIONINFO os_info;
  uint32        platform_id;
  char          name_id[MAX_STRING_BUFFER];

  os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&os_info);

  strcpy(name_id, "Microsoft Mobile [unknown]");
  platform_id = PLATFORM_UNKNOWN;
  switch (os_info.dwMajorVersion)
  {
    case 3:
         // lets assume smartphone
         strcpy(name_id, "Smartphone");
         platform_id = DEVICE_SMARTPHONE;

         // all pocket pc units will override this
         if (SystemParametersInfo(SPI_GETPLATFORMTYPE, 
                                  sizeof(szPlatform), szPlatform, 0))
         {
           if (lstrcmp(szPlatform, TEXT("PocketPC")) == 0)
           {
             strcpy(name_id, "Pocket PC");
             platform_id = DEVICE_POCKET_PC;
           }
         }         
         break;

    case 4:

         // lets assume smartphone
         strcpy(name_id, "Smartphone 2003");
         platform_id = DEVICE_SMARTPHONE_2003;

         // all pocket pc units will override this
         if (SystemParametersInfo(SPI_GETPLATFORMTYPE, 
                                  sizeof(szPlatform), szPlatform, 0))
         {
           if (lstrcmp(szPlatform, TEXT("PocketPC")) == 0)
           {
             strcpy(name_id, "Pocket PC 2003");
             platform_id = DEVICE_POCKET_PC_2003;
           }
         }         

         // second edition?
         if (os_info.dwMinorVersion > 20)
         {
           strcat(name_id, " SE");
         }
         break;

    case 5:

         // lets assume smartphone
         strcpy(name_id, "Smartphone 2005");
         platform_id = DEVICE_SMARTPHONE_2005;

         // all pocket pc units will override this
         if (SystemParametersInfo(SPI_GETPLATFORMTYPE, 
                                  sizeof(szPlatform), szPlatform, 0))
         {
           if (lstrcmp(szPlatform, TEXT("PocketPC")) == 0)
           {
             strcpy(name_id, "Pocket PC 2005");
             platform_id = DEVICE_POCKET_PC_2005;
           }
         }         
         break;

    default:
         break;
  }

  // can we get any other information about the device (OEM)
  if (SystemParametersInfo(SPI_GETOEMINFO, 
                           sizeof(szManufacturer), szManufacturer, 0))
  {
    char sName[MAX_STRING_BUFFER];
    wcstombs(sName, szManufacturer, MAX_STRING_BUFFER);

    // append the OEM information
    strcat(name_id, ": ");
    strcat(name_id, sName);
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
    // we MUST execute the appGoEvent first, ignore nilEvents until then
    g_sys_nilEvent_todo =
      (g_sys_event_queue[g_sys_event_index].eType != _appGoEvent);
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

  // terminate the timer
  KillTimer(g_hWnd, g_sys_timerID);
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
