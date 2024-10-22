/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL.c
 */

#include "../SHARK-cfg.h"

/*************************************************************************
 * Configuration - GENERIC
 *************************************************************************/

// LOG_DISABLE
// SHARK logs internally to either the application console or a log file
// that contains debugging information to assist with the development of
// the applications - to ensure performance is maintained, this option
// should be disabled when building versions for distribution.
//
// 1 - disable SHARK internal logging
// 0 -  enable SHARK internal logging

#ifndef LOG_DISABLE
#define LOG_DISABLE 1
#endif

// ACCELEROMETER_DISABLE
// disable the use of the accelerometer - an optimization to prevent an
// uneccessary thread and processing, giving applications more CPU time.
//
// 1 - disable accelerometer use
// 0 -  enable accelerometer use

#ifndef ACCELEROMETER_DISABLE
#define ACCELEROMETER_DISABLE 0
#endif

// PLATFORM_ORIENTATION
// define the requirements for application orientation; either portrait
// or landscape, on square screens, it is assumed the default orientation
// will be sufficient.
//
// 0 - ensure application is portrait
// 1 - ensure application is landscape

#ifndef PLATFORM_ORIENTATION
#define PLATFORM_ORIENTATION 1
#endif

/*************************************************************************
 * Configuration
 *************************************************************************/

// WEBOS_HIRES
// _LCD sub-system to use 1024x768 instead of the default 480x320 resolution
//
// 0 - 480x320 resolution
// 1 - 1024x768 resolution

#ifndef WEBOS_HIRES
#define WEBOS_HIRES 0
#endif

// WEBOS_PIXEL_DOUBLE
// perform pixel doubling of the framebuffer window at hardware level.
//
// 0 - ensure native resolution
// 1 - ensure pixel doubling resolution

#ifndef WEBOS_PIXEL_DOUBLE
#define WEBOS_PIXEL_DOUBLE 0
#endif

/*************************************************************************
 * Constants
 *************************************************************************/

#define DIRECTORY_SEPARATOR        '/'

#define APPLICATION_LOG_FILE       "application-runtime.txt"

#if (WEBOS_HIRES == 1)

#if (WEBOS_PIXEL_DOUBLE == 1)
#if (PLATFORM_ORIENTATION == 1)
#define APPLICATION_WIDTH          512 
#define APPLICATION_HEIGHT         384
#else
#define APPLICATION_WIDTH          384 
#define APPLICATION_HEIGHT         512
#endif
#else
#if (PLATFORM_ORIENTATION == 1)
#define APPLICATION_WIDTH          1024
#define APPLICATION_HEIGHT         768
#else
#define APPLICATION_WIDTH          768
#define APPLICATION_HEIGHT         1024
#endif
#endif

#else

#if (WEBOS_PIXEL_DOUBLE == 1)
#if (PLATFORM_ORIENTATION == 1)
#define APPLICATION_WIDTH          240 
#define APPLICATION_HEIGHT         160
#else
#define APPLICATION_WIDTH          160 
#define APPLICATION_HEIGHT         240
#endif
#else
#if (PLATFORM_ORIENTATION == 1)
#define APPLICATION_WIDTH          480
#define APPLICATION_HEIGHT         320
#else
#define APPLICATION_WIDTH          320
#define APPLICATION_HEIGHT         480
#endif
#endif

#endif

/*************************************************************************
 * System Includes
 *************************************************************************/

#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <dlfcn.h>

#include "SDL.h"

#include "PDL.h"

#define pthread_yield sched_yield

#define SHARK_INTERNAL

#include "../SHARK.h"
#include "../SHARK-core.h"

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define GLOBALS                (Globals *)(&g)
#define GLOBALS_UNINITIALIZED  (GLOBALS == NULL)
#define GLOBALS_DEFINITION     Globals g

#define DAL_GLOBALS \
  int                          argc;          \
  char                       **argv;          \
                                              \
  int16                        penX;          \
  int16                        penY;          \
  boolean                      penDown;       \
                                              \
  int                          timer_ms;      \
  SDL_TimerID                  timer_id;      \
                                              \
  pthread_t                    app_thread;    \
                                              \
  FILE                        *log;         

#define g_argc                 (GLOBALS) -> argc
#define g_argv                 (GLOBALS) -> argv

#define g_penX                 (GLOBALS) -> penX
#define g_penY                 (GLOBALS) -> penY
#define g_penDown              (GLOBALS) -> penDown

#define g_timer_ms             (GLOBALS) -> timer_ms
#define g_timer_id             (GLOBALS) -> timer_id

#define g_app_thread           (GLOBALS) -> app_thread

#define g_log                  (GLOBALS) -> log

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK.inc"

/*************************************************************************
 * Logging
 *************************************************************************/

#if (LOG_DISABLE == 0)
static void
DAL_LogInitialize()
{
  uint32 platform;
  char   platform_name[256];
  char   log_path[MAX_PATH_LENGTH];

  // whats the path to the log file?
  sprintf(log_path, "%s%c%s", 
          g_path, DIRECTORY_SEPARATOR, APPLICATION_LOG_FILE);

  // open the log file for writing
  g_log = fopen(log_path, "w");
  if (g_log != NULL)
  {
    fprintf(g_log, "SOFTWARE:\n");
    fprintf(g_log, "---------\n");
    fprintf(g_log, "company         :: %s\n", g_config.company);
    fprintf(g_log, "application     :: %s\n", g_config.name);
    fprintf(g_log, "\n");

    _SysDeviceInfo(&platform, platform_name);
    fprintf(g_log, "PLATFORM:\n");
    fprintf(g_log, "---------\n");
    fprintf(g_log, "platform id     :: %s\n", platform_name);
    fprintf(g_log, "\n");

    fflush(g_log);
  }
}

static void
DAL_LogCreateEntry(char *str)
{
  if (g_log != NULL) 
  {
    fprintf(g_log, "%s\n", str);
    fflush(g_log);
  }
}

static void
DAL_LogTerminate()
{
  if (g_log != NULL) fclose(g_log);
  g_log = NULL;
}
#endif

/*************************************************************************
 * Platform Dependent Implementation
 *************************************************************************/

static int   DAL_TimerProc(int interval, void* param);
static void *DAL_AppThread(void *);

void
DAL_ProcessArguments(int argc, char **argv)
{
  char *p, *g_path_p;

  // get the current working directory
  g_path_p = getcwd(g_path, MAX_PATH_LENGTH);

  // we need to know the applications path
  p = (char *)argv[0];
  if (*p != '/')
  {
         if (strncmp(p, "./",  2) == 0) p++;
    else if (strncmp(p, "../", 3) == 0) { p += 3; p = strchr(p, '/'); }
    if ((g_path[strlen(g_path)-1] != '/') && (*p != '/')) strcat(g_path, "/");
    strcat(g_path, p);  // join the current path + argv[0] name together
  }
  else
    strcpy(g_path, p);  // argv[0] was absolute path
  *(strrchr(g_path, '/')) = 0; // remove the executable name
}

boolean
DAL_Initialize(uint16 width, uint16 height)
{
  boolean result;
  Uint32  flags;
  int     err;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // initialize SDL
  flags  = SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER;
#if (ACCELEROMETER_DISABLE == 0)
  flags |= SDL_INIT_JOYSTICK;
#endif
  err = SDL_Init(flags);
  if (err == -1) goto DAL_Initialize_DONE;

  // initialize the PDL library
  err = PDL_Init(0);
  if (err == -1) goto DAL_Initialize_DONE;

  // whats the desired fps the application wants us to use?
  g_timer_ms = 10000L / MAX(100, MIN(g_config.event.fps, 600)); // 10..60fps

  // initialize the interval timer
  g_timer_id = 
    SDL_AddTimer(g_timer_ms, (SDL_NewTimerCallback)DAL_TimerProc, NULL);  

#if (PLATFORM_ORIENTATION == 1)
  // intialize the orientation mode
  PDL_SetOrientation(PDL_ORIENTATION_270);
#endif

  // disable a few system level things
  PDL_ScreenTimeoutEnable(PDL_FALSE);

  // startup SHARK
  result = _SHARKInitialize(width, height);

  // lock the system mutex
  DAL_SysMutexLock();

  // initialize our pen handling
  g_penX    = width  >> 1;
  g_penY    = height >> 1;
  g_penDown = false;

  // unlock the system mutex
  DAL_SysMutexUnlock();

DAL_Initialize_DONE:;

  return result;
}

static int 
DAL_TimerProc(int interval, void* param)
{
  SDL_Event event;
  SDL_UserEvent userevent;

  userevent.type = SDL_USEREVENT;
  userevent.code = 0;
  userevent.data1 = NULL;
  userevent.data2 = NULL;

  event.type = SDL_USEREVENT;
  event.user = userevent;
  SDL_PushEvent(&event);

  return(interval);
}

static void *
DAL_AppThread(void *user_data)
{
  event _e;

  // basic initialization (and, loading screen)
  memset(&_e, 0, sizeof(event));
  _e.eType = _appStartEvent;
  _SHARKHandleEvent(&_e);

  // application is ready to startup - can do stuff like loading here
  memset(&_e, 0, sizeof(event));
  _e.eType = _appGoEvent;
  _SHARKHandleEvent(&_e);

  // we must manually give the app focus - now we have started
  _SHARKFocusEvent(true);

  while (g_running)
  {
    // dispatch the events in SHARK
    if (_SysEventAvailable())
      _SHARKDispatchEvents();

    // give operating system time to do other tasks
    pthread_yield();
  }

  // dispatch any remaining events in SHARK
  while (_SysEventAvailable())
    _SHARKDispatchEvents();

  return NULL;
}

void
DAL_InitApplication()
{
  _PreferencesLoad();

  // create the application thread (differnet from UI thread)
  pthread_create(&g_app_thread, NULL,
                 DAL_AppThread, (void *)GLOBALS);
}

void
DAL_EventLoop()
{
  SDL_Event e;
  event     _e;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // while the application is still running
  while (g_running)
  {
    // lets check if there are any events pending
    if (SDL_PollEvent(&e))
    {
      // which event was received?     
      switch (e.type)
      {
        case SDL_USEREVENT: 
             // we only post nilevents if we are active
             if (g_active) 
             {
               memset(&_e, 0, sizeof(event));
               _e.eType = _nilEvent;
               _SysEnqueueEvent(&_e);
             }
             break;

        case SDL_VIDEOEXPOSE:
             // repaint the display
             _LCDRepaint();
             break;

        case SDL_MOUSEBUTTONDOWN:

             // do we have some valid points?
             if ((e.button.x >= 0) && (e.button.x < g_width) &&
                 (e.button.y >= 0) && (e.button.y < g_height))
             {
               // lock the system mutex
               DAL_SysMutexLock();

               // we know the mouse is down
               g_penDown = true;

               // store the values for future use
               g_penX = (int16)e.button.x;
               g_penY = (int16)e.button.y;

               // unlock the system mutex
               DAL_SysMutexUnlock();

               _SHARKPenEvent((int16)e.button.x, (int16)e.button.y);
             }

             break;

        case SDL_MOUSEMOTION:
             // do we have some valid points?
             if ((e.motion.x >= 0) && (e.motion.x < g_width) &&
                 (e.motion.y >= 0) && (e.motion.y < g_height))
             {
               // we only care about different points
               if (((g_penX != (int16)e.motion.x) || 
                    (g_penY != (int16)e.motion.y)) &&
                    (e.motion.state != SDL_RELEASED))
               {
                 // lock the system mutex
                 DAL_SysMutexLock();

                 // we know the mouse is down
                 g_penDown = true;

                 // store the values for future use
                 g_penX = (int16)e.motion.x;
                 g_penY = (int16)e.motion.y;

                 // unlock the system mutex
                 DAL_SysMutexUnlock();

                 _SHARKPenEvent((int16)e.motion.x, (int16)e.motion.y);
               }
             }
             break;

        case SDL_MOUSEBUTTONUP:
             // do we have some valid points?
             if ((e.button.x >= 0) && (e.button.x < g_width) &&
                 (e.button.y >= 0) && (e.button.y < g_height))
             {
               // lock the system mutex
               DAL_SysMutexLock();

               // we know the mouse is up
               g_penDown = false;

               // store the values for future use
               g_penX = (int16)e.button.x;
               g_penY = (int16)e.button.y;

               // unlock the system mutex
               DAL_SysMutexUnlock();
             }
             break;

        case SDL_KEYDOWN:
             _SHARKKeyEvent((uint16)e.key.keysym.sym);
             break;

        case SDL_ACTIVEEVENT:
             if (e.active.state == SDL_APPINPUTFOCUS)
               _SHARKFocusEvent(e.active.gain == 1);
             break;

        case SDL_QUIT:
             memset(&_e, 0, sizeof(event));
             _e.eType = _appStopEvent;
             _SysEnqueueEvent(&_e);
             break;

        default:
             break;
      }
    }

    // give operating system chance to do something else
    pthread_yield();
  }
}

void
DAL_QuitApplication()
{
  // cancel the application thread (wait for it to finish)
  pthread_cancel(g_app_thread);
  pthread_join(g_app_thread, NULL);

  _PreferencesSave();
}

void
DAL_Terminate()
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // shutdown shark
  _SHARKTerminate();

  // terminate the interval timer
  SDL_RemoveTimer(g_timer_id);

  // re-enable a few system level things
  PDL_ScreenTimeoutEnable(PDL_TRUE);

  // shutdown the PDL library
  PDL_Quit();

  // terminate SDL
  SDL_Quit();
}

int 
main(int argc, char **argv)
{
  int i;

  // initialize our globals
  memset(GLOBALS, 0, sizeof(GLOBALS));

  // store a reference to the program arguments
  g_argc = argc;
  g_argv = argv;
  DAL_ProcessArguments(argc, argv);

  // obtain the application configuration parameters
  memset(&g_config, 0, sizeof(application_config));
  ApplicationConfig(&g_config);
  {
    for (i=0; i<strlen(g_config.name); i++) 
    {
      // we just need to ensure our directories are going to be lowercase
      if ((g_config.name[i] >= 'A') && (g_config.name[i] <= 'Z'))
        g_config.name[i] |= 0x20;
    }
  }

#if (LOG_DISABLE == 0)
  // initialize the logging engine
  DAL_LogInitialize();
#endif

  // initialize the platform
  g_running = true;
  if (DAL_Initialize(APPLICATION_WIDTH, APPLICATION_HEIGHT))
  {
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(":: - application initialize -\n");
#endif

    if (ApplicationInitialize())
    {
      g_initialized = true;

      DAL_InitApplication();
      DAL_EventLoop();
      DAL_QuitApplication();

      g_initialized = false;
    }
    ApplicationTerminate();

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry("\n:: - application terminate -\n");
#endif
  }

  // shutdown the platform
  DAL_Terminate();

#if (LOG_DISABLE == 0)
  // shutdown the logging engine
  DAL_LogTerminate();
#endif

  return 0;
}

#undef GLOBALS_UNINITIALIZED
#undef GLOBALS_DEFINITION

/*************************************************************************
 * Local Dependencies
 *************************************************************************/

#include "DAL-missing.inc"

#include "DAL-audio.inc"
#include "DAL-display.inc"
#include "DAL-file.inc"
#include "DAL-library.inc"
#include "DAL-input.inc"
#include "DAL-memory.inc"
#include "DAL-preferences.inc"
#include "DAL-resource.inc"
#include "DAL-system.inc"
#include "DAL-socket.inc"

#undef GLOBALS 

#undef APPLICATION_HEIGHT
#undef APPLICATION_WIDTH

#undef APPLICATION_LOG_FILE
#undef DIRECTORY_SEPARATOR

/********************************* EOF ***********************************/
