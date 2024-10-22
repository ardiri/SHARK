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

#define SYS_DEFAULT_TICKS_PER_SEC kEventDurationSecond 

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

static EventLoopTimerRef g_sys_timerID;    // timer

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static void     _TimerProc(EventLoopTimerRef inTimer, void *inUserData);

static boolean  _SysEventAvailable();
static void     _SysFlushEventQueue();
static void     _SysGetEvent(event *e);

// initialize the system manager
static boolean
_SysInitialize()
{
  uint16             fps;
  EventTimerInterval ticksPerFrame;
  boolean            init_ok;

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
  InstallEventLoopTimer(GetCurrentEventLoop(), 0, ticksPerFrame,
                        NewEventLoopTimerUPP(_TimerProc), NULL, &g_sys_timerID);

  // no nilEvent required right now
  g_sys_nilEvent_todo = false;

SYS_INIT_ABORT:

  return init_ok;
}

// get the number of seconds since jan 1, 1970
uint32
_SysGetTime()
{
  return (uint32)(time(NULL));
}

// get the current system date and time
void
_SysGetDateTime(datetime *dt)
{
  time_t     now;
  struct tm *st;

  // system system time
  time(&now);
  st = gmtime(&now);

  // copy the information we need into the structure
  dt->year  = st->tm_year + 1900;
  dt->month = st->tm_mon + 1;
  dt->day   = st->tm_mday;
  dt->hour  = st->tm_hour;
  dt->min   = st->tm_min;
  dt->sec   = st->tm_sec;
}

// wait a period of milliseconds
void
_SysDelay(uint32 millisec)
{
  uint32 ticks = millisec / 16; // approx tick = 1/60th of second
  Delay(ticks, NULL);
}

// display a debug message to the developer
void
_SysDebugMessage(char *msg, boolean dialog)
{
  if (dialog)
  {
    DialogRef       theItem;
    DialogItemIndex itemIndex;
    CFStringRef     msgU; 

    msgU = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
    CreateStandardAlert(kAlertStopAlert, 
                        CFSTR("SHARK DEBUG MESSAGE"), msgU, NULL, &theItem);
    RunStandardAlert(theItem, NULL, &itemIndex); 
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
  uint32 platform_id;
  char   name_id[256];

  strcpy(name_id, "Mac OS X");
  platform_id = PLATFORM_MACOSX | PROVIDER_APPLE;

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
  RemoveEventLoopTimer(g_sys_timerID);
}

// the timer callback procedure, triggers a _nilEvent
static void
_TimerProc(EventLoopTimerRef inTimer, void *inUserData)
{
  event e;

  if (g_active)
  {
    memset(&e, 0, sizeof(event));
    e.eType = _nilEvent;
    _SysEnqueueEvent(&e);
  }

  // platform independent 'processing' of events
  DAL_DispatchEvents();
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
