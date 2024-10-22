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
  NSAutoreleasePool *pool;
  NSDateComponents  *cmp;
  NSCalendar        *cal;
  NSDate            *dte;
  uint32             x;

  pool = [ [ NSAutoreleasePool alloc ] init ];

  // do the work
  cmp = [ [ NSDateComponents alloc ] init ];
  cal = [ NSCalendar currentCalendar ];

  [ cmp setYear:   dt -> year  ];
  [ cmp setMonth:  dt -> month ];
  [ cmp setDay:    dt -> day   ];
  [ cmp setHour:   dt -> hour  ];
  [ cmp setMinute: dt -> min   ];
  [ cmp setSecond: dt -> sec   ];

  dte = [ cal dateFromComponents: cmp ];
  x   = [ dte timeIntervalSince1970 ];

  [ pool release ];

  return x;
}

static void
DAL_SysDebugMessageDialog(char *msg)
{
  NSArray  *arr;
  NSString *str = [ [NSString alloc] initWithUTF8String:msg ];

  // initialize our arguments
  arr = [ [ NSArray alloc ] initWithObjects: @"SHARK DEBUG MESSAGE", str, NULL ];

  // show a modal alert in the main ui thread (blocking)
  [ g_view performSelectorOnMainThread:@selector(showDialogAlert:)
           withObject:arr waitUntilDone: YES];

  [ str release ];
  [ arr release ];
}

static void
DAL_SysDialogDialog(char *title, char *msg)
{
  NSArray  *arr;
  NSString *str = [ [NSString alloc] initWithUTF8String:msg ];
  NSString *tle = [ [NSString alloc] initWithUTF8String:title ];

  // initialize our arguments
  arr = [ [ NSArray alloc ] initWithObjects: tle, str, NULL ];

  // show a modal alert in the main ui thread (blocking)
  [ g_view performSelectorOnMainThread:@selector(showDialogAlert:)
           withObject:arr waitUntilDone: YES];

  [ tle release ];
  [ str release ];
  [ arr release ];
}

static void
DAL_SysDelay(uint32 millisec)
{
  usleep(millisec * 1000L);
}

static void
DAL_SysDeviceInfo(uint32 *platform, char *name)
{
  NSString* modelName;
  NSString* version;
  uint32    platform_id;
  char      name_id[256];

  // get the model name and version from the current device
  modelName = [[[ UIDevice currentDevice ] model         ] copy ];
  version   = [[[ UIDevice currentDevice ] systemVersion ] copy ];

  sprintf(name_id, "%s [version %s]", 
                   [ modelName UTF8String ], 
                   [ version UTF8String   ]);
  platform_id = PLATFORM_MACOSX | PLATFORM_EMBEDDED;

  [ modelName release ];
  [ version   release ];

  // copy the string details to the parameters passed in
  if (platform != NULL) *platform = platform_id;
  if (name != NULL)     strcpy(name, name_id);
}

static void
DAL_SysGetDateTime(datetime *dt)
{
  NSAutoreleasePool *pool;
  NSCalendar        *cal;
  NSDateComponents  *cmp;
  NSDate            *now;

  pool = [ [ NSAutoreleasePool alloc ] init ];

  // get the date break down
  now = [ NSDate date ];
  cal = [ NSCalendar currentCalendar ];
  cmp = [ cal components:(NSYearCalendarUnit   |
                          NSMonthCalendarUnit  |
                          NSDayCalendarUnit    |
                          NSHourCalendarUnit   |
                          NSMinuteCalendarUnit |
                          NSSecondCalendarUnit) fromDate: now ];

  dt -> year  = [ cmp year   ];
  dt -> month = [ cmp month  ];
  dt -> day   = [ cmp day    ];
  dt -> hour  = [ cmp hour   ];
  dt -> min   = [ cmp minute ];
  dt -> sec   = [ cmp second ];

  [ pool release ];
}

static uint32
DAL_SysGetTime()
{
  return (uint32)[ [ NSDate date ] timeIntervalSince1970];
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
