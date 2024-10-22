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

// PALMPRE_SDL_PIXEL_DOUBLE
// perform pixel doubling at a hardware level - this allows for using a
// resolution of 320x160 and have it shown at 480x320 on the device screen
//
// 0 - ensure native resolution
// 1 - ensure pixel doubling resolution

#ifndef PALMPRE_SDL_PIXEL_DOUBLE
#define PALMPRE_SDL_PIXEL_DOUBLE 0
#endif

/*************************************************************************
 * Constants
 *************************************************************************/

#define DIRECTORY_SEPARATOR        '/'

#define APPLICATION_LOG_FILE       "application-runtime.txt"

#if (PALMPRE_SDL_PIXEL_DOUBLE == 1)

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

/*************************************************************************
 * System Includes
 *************************************************************************/

#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <netdb.h>
#include <signal.h>
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

#include "PDL_types.h"

PDL_Err (*PDL_Init)(unsigned int);
PDL_Err (*PDL_GetOSVersion)(PDL_OSVersion *);
PDL_Err (*PDL_SetOrientation)(PDL_Orientation);
PDL_Err (*PDL_ScreenTimeoutEnable)(PDL_bool);
PDL_Err (*PDL_NotifyMusicPlaying)(PDL_bool);
PDL_Err (*PDL_GetDataFilePath)(const char *, char *, int);
void    (*PDL_Quit)(void);

#define pthread_yield sched_yield

#define SHARK_INTERNAL

#include "../SHARK.h"
#include "../SHARK-core.h"

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define GLOBALS               (Globals *)(&g)
#define GLOBALS_UNINITIALIZED (GLOBALS == NULL)
#define GLOBALS_DEFINITION    Globals g

#define DAL_GLOBALS \
  int           argc;          \
  char        **argv;          \
                               \
  void         *pdl_lib;       \
                               \
  int           timer_ms;      \
                               \
  FILE         *log;         

#define g_argc                 (GLOBALS) -> argc
#define g_argv                 (GLOBALS) -> argv

#define g_pdl_lib              (GLOBALS) -> pdl_lib
#define g_timer_ms             (GLOBALS) -> timer_ms

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
    fprintf(g_log, "company    \t:: %s\n", g_config.company);
    fprintf(g_log, "application\t:: %s\n", g_config.name);
    fprintf(g_log, "\n");

    _SysDeviceInfo(&platform, platform_name);
    fprintf(g_log, "PLATFORM:\n");
    fprintf(g_log, "---------\n");
    fprintf(g_log, "platform id\t:: %s\n", platform_name);
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

static void DAL_TimerProc(int);

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
  boolean          result;
  struct sigaction action;
  struct itimerval timer;
  Uint32           flags;
  int              err;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // initialize SDL
  flags  = SDL_INIT_AUDIO | SDL_INIT_VIDEO;
#if (ACCELEROMETER_DISABLE == 0)
  flags |= SDL_INIT_JOYSTICK;
#endif
  err = SDL_Init(flags);
  if (err == -1) goto DAL_Initialize_DONE;

  // initialize the PDL library
  err = PDL_NOERROR;
  if (PDL_Init) err = PDL_Init(0);
  if (err == -1) goto DAL_Initialize_DONE;

  // whats the desired fps the application wants us to use?
  g_timer_ms = 10000L / MAX(100, MIN(g_config.event.fps, 600)); // 10..60fps

  // initialize the interval timer
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = DAL_TimerProc;
  action.sa_flags   = SA_RESTART;
  sigemptyset(&action.sa_mask);
  sigaction(SIGALRM, &action, NULL);

  // start the interval timer
  memset(&timer, 0, sizeof(struct itimerval));
  timer.it_value.tv_sec     =  g_timer_ms / 1000;
  timer.it_value.tv_usec    = (g_timer_ms % 1000) * 1000;
  timer.it_interval.tv_sec  = timer.it_value.tv_sec;
  timer.it_interval.tv_usec = timer.it_value.tv_usec;
  setitimer(ITIMER_REAL, &timer, NULL);
  
#if (PLATFORM_ORIENTATION == 1)
  // intialize the orientation mode
  if (PDL_SetOrientation) PDL_SetOrientation(PDL_ORIENTATION_270);
#endif

  // disable a few system level things
  if (PDL_ScreenTimeoutEnable) PDL_ScreenTimeoutEnable(PDL_FALSE);

  // startup SHARK
  result = _SHARKInitialize(APPLICATION_WIDTH, APPLICATION_HEIGHT);

DAL_Initialize_DONE:;

  return result;
}

static void 
DAL_TimerProc(int sig)
{
  struct itimerval timer;
  event e;

  // we only post nilevents if we are active
  if (g_active) 
  {
    memset(&e, 0, sizeof(event));
    e.eType = _nilEvent;
    _SysEnqueueEvent(&e);
  }

  // start the interval timer
  memset(&timer, 0, sizeof(struct itimerval));
  timer.it_value.tv_sec     =  g_timer_ms / 1000;
  timer.it_value.tv_usec    = (g_timer_ms % 1000) * 1000;
  timer.it_interval.tv_sec  = timer.it_value.tv_sec;
  timer.it_interval.tv_usec = timer.it_value.tv_usec;
  setitimer(ITIMER_REAL, &timer, NULL);
}

void
DAL_InitApplication()
{
  _PreferencesLoad();
}

void
DAL_EventLoop()
{
  SDL_Event e;
  event     _e;
  int       x, y;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // basic initialization (and, loading screen)
  memset(&_e, 0, sizeof(event));
  _e.eType = _appStartEvent;
  _SHARKHandleEvent(&_e);
  
  // application is ready to startup - can do stuff like loading here
  memset(&_e, 0, sizeof(event));
  _e.eType = _appGoEvent;
  _SHARKHandleEvent(&_e);
  
  // we must manually give the app focus
  _SHARKFocusEvent(true);

  // while the application is still running
  while (g_running)
  {
    // lets check if there are any events pending
    if (SDL_PollEvent(&e))
    {
      // which event was received?     
      switch (e.type)
      {
        case SDL_VIDEOEXPOSE:
             // repaint the display
             _LCDRepaint();
             break;

        case SDL_MOUSEBUTTONDOWN:
             x = e.button.x;
             y = e.button.y;

             // do we have some valid points?
             if ((x >= 0) && (x < g_width) &&
                 (y >= 0) && (y < g_height))
             {
               _SHARKPenEvent(x, y);
             }
             break;

        case SDL_MOUSEMOTION:

             // only interested in when the mouse is down
             if (e.motion.state == SDL_RELEASED) break;

             x = e.motion.x;
             y = e.motion.y;

             // do we have some valid points?
             if ((x >= 0) && (x < g_width) &&
                 (y >= 0) && (y < g_height))
             {
               _SHARKPenEvent(x, y);
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

    // dispatch the events in SHARK
    if (_SysEventAvailable() && g_running)
      _SHARKDispatchEvents();

    // let the operating system do other things
    if (g_active) SDL_Delay(1); else SDL_Delay(100);
  }
}

void
DAL_QuitApplication()
{
  _PreferencesSave();
}

void
DAL_Terminate()
{
  struct itimerval timer;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // shutdown shark
  _SHARKTerminate();

  // terminate the interval timer
  memset(&timer, 0, sizeof(struct itimerval));
  setitimer(ITIMER_REAL, &timer, NULL);

  // re-enable a few system level things
  if (PDL_ScreenTimeoutEnable) PDL_ScreenTimeoutEnable(PDL_TRUE);

  // shutdown the PDL library
  if (PDL_Quit) PDL_Quit();

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

  // dynamically load the PDL library
  g_pdl_lib = dlopen("libpdl.so", RTLD_LAZY);
  if (g_pdl_lib)
  {
    PDL_Init                = dlsym(g_pdl_lib, "PDL_Init");
    PDL_GetOSVersion        = dlsym(g_pdl_lib, "PDL_GetOSVersion");
    PDL_SetOrientation      = dlsym(g_pdl_lib, "PDL_SetOrientation");
    PDL_ScreenTimeoutEnable = dlsym(g_pdl_lib, "PDL_ScreenTimeoutEnable");
    PDL_NotifyMusicPlaying  = dlsym(g_pdl_lib, "PDL_NotifyMusicPlaying");
    PDL_GetDataFilePath     = dlsym(g_pdl_lib, "PDL_GetDataFilePath");
    PDL_Quit                = dlsym(g_pdl_lib, "PDL_Quit");
  }
  else
  { 
    PDL_Init                = NULL;
    PDL_GetOSVersion        = NULL;
    PDL_SetOrientation      = NULL;
    PDL_ScreenTimeoutEnable = NULL;
    PDL_NotifyMusicPlaying  = NULL;
    PDL_GetDataFilePath     = NULL;
    PDL_Quit                = NULL;
  }

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

  // dynamically unload the PDL library
  dlclose(g_pdl_lib);
  g_pdl_lib = NULL;

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
