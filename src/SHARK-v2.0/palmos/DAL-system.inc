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

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct
{
  void   *sys_app_globals;          // application globals

  uint32  sys_seed;                 // random number seed

  boolean sys_nilEvent_todo;        // priority on nilEvent
  uint16  sys_event_max;
  uint16  sys_event_index;
  event  *sys_event_queue;          // event queue functionality

  uint32  sys_timer_id;
  uint32  sys_timer_freq;

} SysGlobals;

#define g_sys_app_globals      ((SysGlobals *)g -> sys) -> sys_app_globals

#define g_sys_seed             ((SysGlobals *)g -> sys) -> sys_seed

#define g_sys_nilEvent_todo    ((SysGlobals *)g -> sys) -> sys_nilEvent_todo
#define g_sys_event_max        ((SysGlobals *)g -> sys) -> sys_event_max
#define g_sys_event_index      ((SysGlobals *)g -> sys) -> sys_event_index
#define g_sys_event_queue      ((SysGlobals *)g -> sys) -> sys_event_queue

#define g_sys_timer_id         ((SysGlobals *)g -> sys) -> sys_timer_id
#define g_sys_timer_freq       ((SysGlobals *)g -> sys) -> sys_timer_freq

#define GLOBALS_UNINITIALIZED  (g -> sys == NULL)

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static void     _TimerProc(void *);

static boolean  _SysEventAvailable();
static void     _SysFlushEventQueue();
static void     _SysGetEvent(event *);

// initialize the system manager
static boolean
_SysInitialize()
{
  boolean     init_ok;
  uint16      fps;
  SysGlobals *g_lib;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _SysInitialize");
#endif

  // allocate memory for library globals
  g_lib = (SysGlobals *)malloc(sizeof(SysGlobals));
  if (g_lib != NULL) g -> sys = (void *)g_lib;

  // assume everything is ok
  init_ok  = true;
  init_ok &= (g_lib != NULL);

  // allocate the memory chunk we need for event handling
  if (init_ok)
  {
    memset(g_lib, 0, sizeof(SysGlobals));

    g_sys_event_max   = 0;
    g_sys_event_index = 0;
    g_sys_event_queue = (event *)malloc(SYS_QUEUE_SIZE * sizeof(event));
    init_ok = (g_sys_event_queue != NULL); if (!init_ok) goto SYS_INIT_ABORT;
    memset(g_sys_event_queue, 0, SYS_QUEUE_SIZE * sizeof(event));

    _SysFlushEventQueue();
    _SysRandom(_SysGetTime());
  }

  // whats the desired fps the application wants us to use?
  fps = MAX(100, MIN(g_config.display.fps, 600));  // 10 .. 60 fps = limit
  g_sys_timer_freq = 10000L / fps;

  // create the timer
  KALTimerCreate(&g_sys_timer_id, 0, _TimerProc, (void *)g_lib);
  KALTimerSet(g_sys_timer_id, g_sys_timer_freq);

  // no nilEvent required right now
  g_sys_nilEvent_todo = false;

SYS_INIT_ABORT:

  return init_ok;
}

// get the number of seconds since jan 1, 1970
uint32
_SysGetTime()
{
  return (TimGetSeconds() - 2082844800L); // jan 1, 1904 -> 1970 = 24107 days
}

// get the current system date and time
void
_SysGetDateTime(datetime *dt)
{
  DateTimeType time;

  // get the date and time from the system
  TimSecondsToDateTime(TimGetSeconds(), &time);

  // copy over the relvant pieces of information
  dt -> year  = time.year;
  dt -> month = time.month;
  dt -> day   = time.day;
  dt -> hour  = time.hour;
  dt -> min   = time.minute;
  dt -> sec   = time.second;
}

// wait a period of milliseconds
void
_SysDelay(uint32 millisec)
{
  SysTaskDelay((millisec * SysTicksPerSecond()) / 1000);
}

// display a debug message to the developer
void
_SysDebugMessage(char *msg, boolean dialog)
{
  if (dialog)
  {
    // display a dialog
    FrmCustomAlert(DebugAlert, msg, NULL, NULL);
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
  char   name_id[256], code[5];

  strcpy(name_id, "Palm OS [unknown]");
  platform_id = PLATFORM_PALM;

  switch (g -> ftr.comID)
  {
    case 'palm':
    case 'Palm': // Palm
              if (g -> ftr.devID == 'Frg1') strcpy(name_id, "Palm Tungsten|T");
         else if (g -> ftr.devID == 'Frg2') strcpy(name_id, "Palm Tungsten|T2");
         else if (g -> ftr.devID == 'MT64') strcpy(name_id, "Palm Tungsten|C");
         else if (g -> ftr.devID == 'Cct1') strcpy(name_id, "Palm Tungsten|E");
         else if (g -> ftr.devID == 'Zir4') strcpy(name_id, "Palm Tungsten|E2");
         else if (g -> ftr.devID == 'Arz1') strcpy(name_id, "Palm Tungsten|T3");
         else if (g -> ftr.devID == 'TnT5') strcpy(name_id, "Palm Tungsten|T5");
         else if (g -> ftr.devID == 'D051') strcpy(name_id, "Palm Zire 22");
         else if (g -> ftr.devID == 'Zi22') strcpy(name_id, "Palm Zire 31");
         else if (g -> ftr.devID == 'Zpth') strcpy(name_id, "Palm Zire 71");
         else if (g -> ftr.devID == 'Zi72') strcpy(name_id, "Palm Zire 72");
         else if (g -> ftr.devID == 'TunX') strcpy(name_id, "Palm Lifedrive");
         else if (g -> ftr.devID == 'D050') strcpy(name_id, "Palm Tungsten|X");
         else goto UNKNOWN_DEVICE;
         break;

    case 'hspr': // HandSpring
              if (g -> ftr.devID == 'H101') strcpy(name_id, "Handspring Treo 600");
         else if (g -> ftr.devID == 'H102') strcpy(name_id, "Palm Treo 650");
         else goto UNKNOWN_DEVICE;
         break;

    case 'grmn': // Garmin
              if (g -> ftr.devID == '3000') strcpy(name_id, "Garmin iQue 3000");
         else if (g -> ftr.devID == '3200') strcpy(name_id, "Garmin iQue 3200");
         else if (g -> ftr.devID == '3600') strcpy(name_id, "Garmin iQue 3600");
         else if (g -> ftr.devID == '3700') strcpy(name_id, "Garmin iQue 3600a");
         else goto UNKNOWN_DEVICE;
         break;

    case 'sony': // Sony
              if (g -> ftr.devID == 'goha') strcpy(name_id, "Sony Clie TJ25");
         else if (g -> ftr.devID == 'leia') strcpy(name_id, "Sony Clie TJ27");
         else if (g -> ftr.devID == 'goku') strcpy(name_id, "Sony Clie TJ35");
         else if (g -> ftr.devID == 'luke') strcpy(name_id, "Sony Clie TJ37");
         else if (g -> ftr.devID == 'atom') strcpy(name_id, "Sony Clie TH55");
         else if (g -> ftr.devID == 'vrna') strcpy(name_id, "Sony Clie TG50");
         else if (g -> ftr.devID == 'amno') strcpy(name_id, "Sony Clie UX40");
         else if (g -> ftr.devID == 'prmr') strcpy(name_id, "Sony Clie UX50");
         else if (g -> ftr.devID == 'crdb') strcpy(name_id, "Sony Clie NX60/NX70");
         else if (g -> ftr.devID == 'tldo') strcpy(name_id, "Sony Clie NX73V");
         else if (g -> ftr.devID == 'mdrd') strcpy(name_id, "Sony Clie NX80V");
         else if (g -> ftr.devID == 'grnd') strcpy(name_id, "Sony Clie NZ90");
         else if (g -> ftr.devID == 'ancy') strcpy(name_id, "Sony Clie VZ90");
         else goto UNKNOWN_DEVICE;
         break;

    case 'Tpwv': // Tapwave
              if (g -> ftr.devID == 'Rdog') strcpy(name_id, "Tapwave Zodiac");
         else goto UNKNOWN_DEVICE;
         break;

    default:

UNKNOWN_DEVICE:

         strcpy(name_id, "Palm OS [unknown] - ");

         // company id
         code[0] = (char)((g -> ftr.comID >> 24) & 0xff);
         code[1] = (char)((g -> ftr.comID >> 16) & 0xff);
         code[2] = (char)((g -> ftr.comID >>  8) & 0xff);
         code[3] = (char)( g -> ftr.comID        & 0xff);
         code[4] = 0;
         strcat(name_id, code); strcat(name_id, ":");

         // device id
         code[0] = (char)((g -> ftr.devID >> 24) & 0xff);
         code[1] = (char)((g -> ftr.devID >> 16) & 0xff);
         code[2] = (char)((g -> ftr.devID >>  8) & 0xff);
         code[3] = (char)( g -> ftr.devID        & 0xff);
         code[4] = 0;
         strcat(name_id, code); 
         break;
  }

  if (platform != NULL) *platform = platform_id;
  if (name != NULL)     strcpy(name, name_id);
}

void
_SysEnqueueEvent(event *e)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

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
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  g_sys_app_globals = gbls;
}

void *
_SysGetGlobals()
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return NULL;

  return g_sys_app_globals;
}

uint16
_SysRandom(uint32 seed)
{
  uint16 result;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return 0;

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

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // delete the timer
  KALTimerDelete(g_sys_timer_id);

  // clean up memory usage within memory management system
  if (g_sys_event_queue != NULL) free(g_sys_event_queue);
  g_sys_event_queue = (event *)NULL;

  // free memory used by system globals
  if (g -> sys != NULL) free(g -> sys);
  g -> sys = NULL;
}

static void
_TimerProc(void *user)
{
  SysGlobals *g_lib = (SysGlobals *)user;

  // lets register for the next event 
  KALTimerSet(g_lib -> sys_timer_id, g_lib -> sys_timer_freq);

  // we now need to initiate a nilEvent when the eventloop gets a chance
  g_lib -> sys_nilEvent_todo = true;
}

// are there any events available for processing?
static boolean
_SysEventAvailable()
{
  boolean result;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return false;

  // default return value
  result = (g_sys_nilEvent_todo || (g_sys_event_index != g_sys_event_max));

  return result;
}

// flush the event queue
static void
_SysFlushEventQueue()
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  g_sys_event_index   = 0;
  g_sys_event_max     = 0;
  g_sys_nilEvent_todo = false;
}

// get the first available event from the event queue
static void
_SysGetEvent(event *e)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

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

#undef GLOBALS_UNINITIALIZED 

/********************************* EOF ***********************************/
