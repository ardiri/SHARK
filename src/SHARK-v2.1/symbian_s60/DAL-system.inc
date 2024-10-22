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

// -----------------------------------------------------------------------
// compile time options
// -----------------------------------------------------------------------

/*************************************************************************
 *
 * Global Variables
 * 
 *************************************************************************/

typedef struct
{
  void   *app_globals;          // application globals

  uint32  seed;                 // random number seed

  boolean nilEvent_todo;        // priority on nilEvent
  uint16  event_max;
  uint16  event_index;
  event  *event_queue;          // event queue functionality
} SysGlobalsType;

#define GLOBALS_DEFINE_SYS SysGlobalsType *g_sys;
#define GLOBALS_ACCESS_SYS GLOBALS_ACCESS; \
                           SysGlobalsType *g_sys = (SysGlobalsType *)g->sys;

#define g_sys_app_globals    g_sys->app_globals
#define g_sys_seed           g_sys->seed
#define g_sys_nilEvent_todo  g_sys->nilEvent_todo
#define g_sys_event_max      g_sys->event_max
#define g_sys_event_index    g_sys->event_index
#define g_sys_event_queue    g_sys->event_queue

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static boolean  _SysEventAvailable();
static void     _SysFlushEventQueue();
static void     _SysGetEvent(event *e);

// initialize the system manager
static boolean
_SysInitialize()
{
  boolean init_ok;
  GLOBALS_ACCESS;
  GLOBALS_DEFINE_SYS;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _SysInitialize");
#endif

  // assume everything is ok
  init_ok = true;

  // globals:: allocate appropriate memory
  g_sys = (SysGlobalsType *)malloc(sizeof(SysGlobalsType));
  init_ok = (g_sys != NULL); if (!init_ok) goto SYS_INIT_ABORT;
  memset(g_sys, 0, sizeof(SysGlobalsType));
  g->sys = g_sys;

  // allocate the memory chunk we need for event handling
  g_sys_event_max   = 0;
  g_sys_event_index = 0;
  g_sys_event_queue = (event *)malloc(SYS_QUEUE_SIZE * sizeof(event));
  init_ok = (g_sys_event_queue != NULL); if (!init_ok) goto SYS_INIT_ABORT;
  memset(g_sys_event_queue, 0, SYS_QUEUE_SIZE * sizeof(event));

  _SysFlushEventQueue();
  _SysRandom(_SysGetTime());

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
  TRequestStatus timerStatus;
  RTimer         timer;

  timer.CreateLocal();
  timer.After(timerStatus,TTimeIntervalMicroSeconds32(millisec * 1000));
  User::WaitForRequest(timerStatus);
  timer.Close();
}

// display a debug message to the developer
void
_SysDebugMessage(char *msg, boolean dialog)
{
  TBuf<256> text;

  if (dialog)
  {
    TPtrC8 p((const TUint8 *)msg);
    text.Copy(p);

    CEikonEnv::Static()->InfoWinL(_L("SHARK DEBUG"), text);
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
  TInt   mUid, mColors, mCPU;

  strcpy(name_id, "Symbian S60 - [unknown]");
  platform_id = PLATFORM_SYMBIAN_S60;

  // device reference
  // http://www.newlc.com/article.php3?id_article=161

  HAL::Get(HALData::EMachineUid, mUid);
  switch (mUid)
  {
    case 0x10200f97:
         strcpy(name_id, "Symbian S60 - Nokia 3230");
         platform_id = DEVICE_NOKIA_3230;
         break;

    case 0x101f466a:
         HAL::Get(HALData::EDisplayColors, mColors);
         if (mColors == 0x1000)
         {
           strcpy(name_id, "Symbian S60 - Nokia 3650");
           platform_id = DEVICE_NOKIA_3650;
         }
         else
         {
           strcpy(name_id, "Symbian S60 - Nokia 3660");
           platform_id = DEVICE_NOKIA_3660;
         }
         break;

    case 0x101fb3f4:
         strcpy(name_id, "Symbian S60 - Nokia 6260");
         platform_id = DEVICE_NOKIA_6260;
         break;

    case 0x101fb3dd:
         strcpy(name_id, "Symbian S60 - Nokia 6600");
         platform_id = DEVICE_NOKIA_6600;
         break;

    case 0x101f3ee3:
         strcpy(name_id, "Symbian S60 - Nokia 6620");
         platform_id = DEVICE_NOKIA_6620;
         break;

    case 0x101fbb55:
         strcpy(name_id, "Symbian S60 - Nokia 6630");
         platform_id = DEVICE_NOKIA_6630;
         break;

    case 0x101fb3f3:
/**
 ** 6670/7610 = same device?
 **
         strcpy(name_id, "Symbian S60 - Nokia 7610");
         platform_id = DEVICE_NOKIA_7610;
 **/
         strcpy(name_id, "Symbian S60 - Nokia 6670");
         platform_id = DEVICE_NOKIA_6670;
         break;

    case 0x10200f99:
         strcpy(name_id, "Symbian S60 - Nokia 6680");
         platform_id = DEVICE_NOKIA_6680;
         break;

    case 0x10200f9c:
         strcpy(name_id, "Symbian S60 - Nokia 6681");
         platform_id = DEVICE_NOKIA_6681;
         break;

    case 0x10200f9b:
         strcpy(name_id, "Symbian S60 - Nokia 6682");
         platform_id = DEVICE_NOKIA_6682;
         break;

    case 0x101f4fc3:
         strcpy(name_id, "Symbian S60 - Nokia 7650");
         platform_id = DEVICE_NOKIA_7650;
         break;

    case 0x101f8c19: 
         strcpy(name_id, "Symbian S60 - Nokia N-Gage");
         platform_id = DEVICE_NOKIA_NGAGE;
         break;

    case 0x101fb2b1: 
         strcpy(name_id, "Symbian S60 - Nokia N-Gage QD");
         platform_id = DEVICE_NOKIA_NGAGE_QD;
         break;

    case 0x10200f9a:
         strcpy(name_id, "Symbian S60 - Nokia N70");
         platform_id = DEVICE_NOKIA_N70;
         break;

    case 0x10200f98:
         strcpy(name_id, "Symbian S60 - Nokia N90");
         platform_id = DEVICE_NOKIA_N90;
         break;

    case 0x101fa031: 
         strcpy(name_id, "Symbian S60 - Sendo-X");
         platform_id = DEVICE_SENDO_X;
         break;

    case 0x101f9071: 
         strcpy(name_id, "Symbian S60 - Siemens SX1");
         platform_id = DEVICE_SIEMENS_SX1;
         break;

    case 0x101fe7b7: 
         strcpy(name_id, "Symbian S60 - Samsung SGH D730");
         platform_id = DEVICE_SIEMENS_SX1;
         break;

    default:
         sprintf(name_id, "Symbian S60 - [unknown] 0x%08lx", (uint32)mUid);
         break;
         break;  
  }

  // lets throw on the CPU speed while we can
  HAL::Get(HALData::ECPUSpeed, mCPU);
  {
    char cpuSpeed[32];
    sprintf(cpuSpeed, "%d Mhz", (uint16)(mCPU / 1000));

    strcat(name_id, " @ ");
    strcat(name_id, cpuSpeed);
  }

  if (platform != NULL) *platform = platform_id;
  if (name != NULL)     strcpy(name, name_id);
}

void
_SysEnqueueEvent(event *e)
{
  GLOBALS_ACCESS_SYS;

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
  GLOBALS_ACCESS_SYS;

  g_sys_app_globals = gbls;
}

void *
_SysGetGlobals()
{
  GLOBALS_ACCESS_SYS;

  return g_sys_app_globals;
}

uint16
_SysRandom(uint32 seed)
{
  uint16 result;
  GLOBALS_ACCESS_SYS;

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
  GLOBALS_ACCESS_SYS;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _SysTerminate");
#endif

  if (g_sys)
  {
    // clean up memory usage within memory management system
    if (g_sys_event_queue != NULL) free(g_sys_event_queue);
    g_sys_event_queue = (event *)NULL;
  }

  // globals:: clean up memory
  if (g_sys) free(g_sys); g_sys = NULL;
  g->sys = g_sys;
}

// are there any events available for processing?
static boolean
_SysEventAvailable()
{
  boolean result;
  GLOBALS_ACCESS_SYS;

  // default return value
  result = (g_sys_nilEvent_todo || (g_sys_event_index != g_sys_event_max));

  return result;
}

// flush the event queue
static void
_SysFlushEventQueue()
{
  GLOBALS_ACCESS_SYS;

  g_sys_event_index   = 0;
  g_sys_event_max     = 0;
  g_sys_nilEvent_todo = false;
}

// get the first available event from the event queue
static void
_SysGetEvent(event *e)
{
  GLOBALS_ACCESS_SYS;

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

#undef GLOBALS_DEFINE_SYS
#undef GLOBALS_ACCESS_SYS

#undef g_sys_app_globals
#undef g_sys_seed
#undef g_sys_nilEvent_todo
#undef g_sys_event_max
#undef g_sys_event_index
#undef g_sys_event_queue

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-system.inc"

/********************************* EOF ***********************************/
