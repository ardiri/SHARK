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

// BBX_HIRES
// _LCD sub-system to use 1024x768 instead of the default 480x320 resolution
//
// 0 - 480x320 resolution
// 1 - 1024x768 resolution

#ifndef BBX_HIRES
#define BBX_HIRES        0
#endif

// BBX_PIXEL_DOUBLE
// perform pixel doubling at a hardware level - this allows for using a
// resolution of 320x160 and have it shown at 480x320 on the device screen
//
// 0 - ensure native resolution
// 1 - ensure pixel doubling resolution

#ifndef BBX_PIXEL_DOUBLE
#define BBX_PIXEL_DOUBLE 0
#endif

/*************************************************************************
 * Constants
 *************************************************************************/

#define DIRECTORY_SEPARATOR        '/'

#if (BBX_HIRES == 1)

#if (BBX_PIXEL_DOUBLE == 1)
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

#if (BBX_PIXEL_DOUBLE == 1)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>

#include <bps/bps.h>
#include <bps/event.h>
#include <bps/screen.h>
#include <bps/navigator.h>
#include <bps/orientation.h>
#include <bps/virtualkeyboard.h>
#include <bps/deviceinfo.h>
#include <bps/sensor.h>
#include <screen/screen.h>
#include <input/screen_helpers.h>
#include <sys/keycodes.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/asound.h>
#include <sys/asoundlib.h>
#undef true

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
  int                          argc;        \
  char                       **argv;        \
                                            \
  int                          channel;     \
  int                          domain;      \
                                            \
  screen_context_t             screen;      \
  screen_window_t              window;      \
                                            \
  int16                        penX;        \
  int16                        penY;        \
  boolean                      penDown;     \
                                            \
  int16                        accelX;      \
  int16                        accelY;      \
  int16                        accelZ;      \
                                            \
  boolean                      started;     \
                                            \
  int                          timer_ms;    \
  pthread_t                    app_thread;  \
                                            \
  FILE                        *log;

#define g_argc                 (GLOBALS) -> argc
#define g_argv                 (GLOBALS) -> argv

#define g_channel              (GLOBALS) -> channel
#define g_domain               (GLOBALS) -> domain

#define g_screen               (GLOBALS) -> screen
#define g_window               (GLOBALS) -> window

#define g_penX                 (GLOBALS) -> penX
#define g_penY                 (GLOBALS) -> penY
#define g_penDown              (GLOBALS) -> penDown

#if (ACCELEROMETER_DISABLE == 0)
#define g_accelX               (GLOBALS) -> accelX
#define g_accelY               (GLOBALS) -> accelY
#define g_accelZ               (GLOBALS) -> accelZ
#endif

#define g_started              (GLOBALS) -> started

#define g_timer_ms             (GLOBALS) -> timer_ms
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

  // open the log file for writing
  g_log = stdout;
  if (g_log != NULL)
  {
    // all logging is to console
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
}
#endif

/*************************************************************************
 * Platform Dependent Implementation
 *************************************************************************/

static void  DAL_TimerProc(int);
static void *DAL_AppThread(void *);

void
DAL_ProcessArguments(int argc, char **argv)
{
  // get the current working directory
  getcwd(g_path, MAX_PATH_LENGTH);
}

boolean
DAL_Initialize(uint16 width, uint16 height)
{
  boolean          result;
  struct sigaction action;
  struct itimerval timer;
  int              param, angle, pair[2];

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
//if (GLOBALS_UNINITIALIZED) return result;

  // we need to register a domain for nilEvent handling
  g_channel = bps_channel_get_active();
  g_domain  = bps_register_domain();

  // whats the desired fps the application wants us to use?
  g_timer_ms = 10000L / MAX(100, MIN(g_config.event.fps, 600)); // 10..60fps

  // initialize the interval timer
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = DAL_TimerProc;
  action.sa_flags   = 0;
  sigemptyset(&action.sa_mask);
  sigaction(SIGALRM, &action, NULL);

  // schedule the first interval (timer)
  memset(&timer, 0, sizeof(struct itimerval));
  timer.it_value.tv_sec  =  g_timer_ms / 1000;
  timer.it_value.tv_usec = (g_timer_ms % 1000) * 1000;
  setitimer(ITIMER_REAL, &timer, NULL);

  // lets obtain information on the device orientation/angle
  orientation_get(NULL, &angle);

  // create the screen context
  screen_create_context(&g_screen, SCREEN_APPLICATION_CONTEXT);

  // create the window
  screen_create_window(&g_window, g_screen);

  // disable automatic standby mode
  param = SCREEN_IDLE_MODE_KEEP_AWAKE;
  screen_set_window_property_iv(g_window, SCREEN_PROPERTY_IDLE_MODE, &param);

  // define the screen usage
  param = SCREEN_USAGE_WRITE | SCREEN_USAGE_NATIVE;
  screen_set_window_property_iv(g_window,
                                SCREEN_PROPERTY_USAGE, &param);

  // define the expectations on the display (16bit, 565)
  param = SCREEN_FORMAT_RGB565;
  screen_set_window_property_iv(g_window,
                                SCREEN_PROPERTY_FORMAT, &param);

  // define the resolution of the window we are expecting
  pair[0] = width;
  pair[1] = height;
  screen_set_window_property_iv(g_window,
                                SCREEN_PROPERTY_BUFFER_SIZE, pair);

  // ensure the window is at the correct rotation
#if (PLATFORM_ORIENTATION == 1)
  if ((angle % 180) != 0) angle = 0;  
#else
  if ((angle % 180) == 0) angle = 90; 
#endif
  screen_set_window_property_iv(g_window,
                                SCREEN_PROPERTY_ROTATION, &angle);

  // we haven't started our application yet
  g_started = false;

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

  // initialization
  screen_request_events(g_screen);
  navigator_request_events(0);
  virtualkeyboard_request_events(0);

  // we want the application to stick to this orientation while running
  navigator_rotation_lock(true);

  return result;
}

static void
DAL_TimerProc(int sig)
{
  struct       itimerval timer;
  bps_event_t *e;

  // schedule a request for a nilEvent to be generated
  bps_event_create(&e, g_domain, 0, NULL, NULL);
  bps_channel_push_event(g_channel, e);

  // schedule the next interval (timer)
  memset(&timer, 0, sizeof(struct itimerval));
  timer.it_value.tv_sec  =  g_timer_ms / 1000;
  timer.it_value.tv_usec = (g_timer_ms % 1000) * 1000;
  setitimer(ITIMER_REAL, &timer, NULL);
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
  g_started = true;
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
  event           _e;
  bps_event_t    *event_bps;
  int             event_domain;
  int             event_id;
  screen_event_t  screen_event;
  mtouch_event_t  touch_event;
  int             state, key, pos[2];
#if (ACCELEROMETER_DISABLE == 0)
  float           x, y, z;
#endif

  // GLOBALS_PROTECTION:
//if (GLOBALS_UNINITIALIZED) return;

  // while the application is still running
  while (g_running)
  {
    // handle events
    event_bps = NULL;
    bps_get_event(&event_bps, -1); // wait for an event

    if (event_bps != NULL)
    {
      event_domain = bps_event_get_domain(event_bps);
  
      if (event_domain == g_domain)
      {
        // we do not post nil events when we are not active
        if (!g_active) goto NIL_EVENT_ABORT;

        memset(&_e, 0, sizeof(event));
        _e.eType = _nilEvent;
        _SysEnqueueEvent(&_e);
      }
      else
      if (event_domain == screen_get_domain())
      {
        // we do not post screen events when we are not active
        if (!g_active) goto NIL_EVENT_ABORT;

        screen_event = screen_event_get_event(event_bps);

        // identify the type the screen event
        screen_get_event_property_iv(screen_event,
                                     SCREEN_PROPERTY_TYPE, &event_id);

        switch (event_id)
        {
          case SCREEN_EVENT_POINTER:

               // identify the position and which buttons are being used
               screen_get_event_property_iv(screen_event,
                                            SCREEN_PROPERTY_SOURCE_POSITION, 
                                            pos);
               screen_get_event_property_iv(screen_event, 
                                            SCREEN_PROPERTY_BUTTONS, &state); 

               // lock the system mutex
               DAL_SysMutexLock();

               // determine if the mouse is down or not
               g_penDown = ((state & SCREEN_LEFT_MOUSE_BUTTON) != 0);

               // unlock the system mutex
               DAL_SysMutexUnlock();

               // do we have some valid points?
               if ((pos[0] >= 0) && (pos[0] < g_width) &&
                   (pos[1] >= 0) && (pos[1] < g_height))
               {
                 // we only care about different points
                 if ((g_penX != (int16)pos[0]) ||
                     (g_penY != (int16)pos[1]))
                 {
                   // lock the system mutex
                   DAL_SysMutexLock();

                   g_penX = (int16)pos[0];
                   g_penY = (int16)pos[1];
  
                   // unlock the system mutex
                   DAL_SysMutexUnlock();

                   if (g_penDown)
                     _SHARKPenEvent((int16)pos[0], (int16)pos[1]);
                 }
               }
               break;

          case SCREEN_EVENT_MTOUCH_TOUCH:

               // get the information about the touch event
               screen_get_mtouch_event(screen_event, &touch_event, 0);

               // we only care about the first touch event/contact
               if (touch_event.contact_id == 0)
               {
                 // do we have some valid points?
                 if ((touch_event.x >= 0) && (touch_event.x < g_width) &&
                     (touch_event.y >= 0) && (touch_event.y < g_height))
                 {
                   // lock the system mutex
                   DAL_SysMutexLock();

                   // we know the pen is down
                   g_penDown = true;

                   // store the values for future use
                   g_penX = (int16)touch_event.x;
                   g_penY = (int16)touch_event.y;

                   // unlock the system mutex
                   DAL_SysMutexUnlock();

                   _SHARKPenEvent((int16)touch_event.x, (int16)touch_event.y);
                 }
               }
               break;

          case SCREEN_EVENT_MTOUCH_MOVE:

               // get the information about the touch event
               screen_get_mtouch_event(screen_event, &touch_event, 0);

               // we only care about the first touch event/contact
               if (touch_event.contact_id == 0)
               {
                 // do we have some valid points?
                 if ((touch_event.x >= 0) && (touch_event.x < g_width) &&
                     (touch_event.y >= 0) && (touch_event.y < g_height))
                 {
                   // we only care about different points
                   if ((g_penX != (int16)touch_event.x) &&
                       (g_penY != (int16)touch_event.y))
                   {
                     // lock the system mutex
                     DAL_SysMutexLock();

                     // we know the pen is down
                     g_penDown = true;

                     // store the values for future use
                     g_penX  = (int16)touch_event.x;
                     g_penY  = (int16)touch_event.y;

                     // unlock the system mutex
                     DAL_SysMutexUnlock();

                     _SHARKPenEvent((int16)touch_event.x, (int16)touch_event.y);
                   }
                 }
               }
               break;

          case SCREEN_EVENT_MTOUCH_RELEASE:

               // get the information about the touch event
               screen_get_mtouch_event(screen_event, &touch_event, 0);

               // we only care about the first touch event/contact
               if (touch_event.contact_id == 0)
               {
                 // do we have some valid points?
                 if ((touch_event.x >= 0) && (touch_event.x < g_width) &&
                     (touch_event.y >= 0) && (touch_event.y < g_height))
                 {
                   // lock the system mutex
                   DAL_SysMutexLock();

                   // the pen is no longer being held down
                   g_penDown = false;

                   // store the values for future use
                   g_penX = (int16)touch_event.x;
                   g_penY = (int16)touch_event.y;

                   // unlock the system mutex
                   DAL_SysMutexUnlock();
                 }
               }
               break;

          case SCREEN_EVENT_KEYBOARD:

               // get the information about the keyboard event
               screen_get_event_property_iv(screen_event,
                                            SCREEN_PROPERTY_KEY_FLAGS, 
                                            &state);
               screen_get_event_property_iv(screen_event,
                                            SCREEN_PROPERTY_KEY_SYM, &key);

               // we only care about DOWN/REPEAT cases
               if ((state & (KEY_DOWN || KEY_REPEAT)) != 0)
                 _SHARKKeyEvent((uint16)key);

               break;

          default:

               break;
        }
      }
      else
      if (event_domain == navigator_get_domain())
      {
        event_id = bps_event_get_code(event_bps);

        switch (event_id)
        {
          case NAVIGATOR_SWIPE_DOWN:

               // create a menu key event 
               memset(&_e, 0, sizeof(event));
               _e.eType = _keyEvent;
               _e.data.key.chr = _key_menu;
               _SysEnqueueEvent(&_e);

               break;

          case NAVIGATOR_EXIT:

               // create a shutdown request
               memset(&_e, 0, sizeof(event));
               _e.eType = _appStopEvent;
               _SysEnqueueEvent(&_e);

               break;

          case NAVIGATOR_WINDOW_ACTIVE:

               // enable event processing (the only mode to startup)
               if (g_started) _SHARKFocusEvent(true);

               break;

          case NAVIGATOR_WINDOW_INACTIVE:

               // enable event processing (the only mode to startup)
               if (g_started) _SHARKFocusEvent(false);

               break;

          case NAVIGATOR_WINDOW_STATE:

               switch (navigator_event_get_window_state(event_bps))
               {
                 case NAVIGATOR_WINDOW_FULLSCREEN:

                      // enable event processing (the only mode to startup)
                      if (g_started) _SHARKFocusEvent(true);

                      break;

                 case NAVIGATOR_WINDOW_THUMBNAIL:
                 case NAVIGATOR_WINDOW_INVISIBLE:

                      // disable event processing
                      if (g_started) _SHARKFocusEvent(false);

                      break;

                 default:
                      break;
               }
               break;

          default:
               break;
        }
      }

#if (ACCELEROMETER_DISABLE == 0)
      else
      if (event_domain == sensor_get_domain())
      {
        event_id = bps_event_get_code(event_bps);

        switch (event_id)
        {
          case SENSOR_ACCELEROMETER_READING:

               // read the value from the sensor
               sensor_event_get_xyz(event_bps, &x, &y, &z);

               // lock the system mutex
               DAL_SysMutexLock();

               // store these values for later use
               g_accelX = (int16)(x * 256);
               g_accelY = (int16)(y * 256);
               g_accelZ = (int16)(z * 256);

printf("%d %d %d\n", g_accelX, g_accelY, g_accelZ);

               // unlock the system mutex
               DAL_SysMutexUnlock();

               break;

          default:
               break;
        }
      }
#endif

NIL_EVENT_ABORT:;

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
  struct itimerval timer;

  // GLOBALS_PROTECTION:
//if (GLOBALS_UNINITIALIZED) return;

  // shutdown shark
  _SHARKTerminate();

  // terminate the interval timer
  memset(&timer, 0, sizeof(struct itimerval));
  setitimer(ITIMER_REAL, &timer, NULL);

  // termination
  screen_stop_events(g_screen);

  // destroy the window
  screen_destroy_window(g_window);

  // destroy the screen context
  screen_destroy_context(g_screen);
}

int 
main(int argc, char **argv)
{
  // initialize our globals
  memset(GLOBALS, 0, sizeof(GLOBALS));

  // obtain the application configuration parameters
  memset(&g_config, 0, sizeof(application_config));
  ApplicationConfig(&g_config);

  // store a reference to the program arguments
  g_argc = argc;
  g_argv = argv;
  DAL_ProcessArguments(argc, argv);

#if (LOG_DISABLE == 0)
  // initialize the logging engine
  DAL_LogInitialize();
#endif

  // blackberry platform services: initialize
  bps_initialize();

#if (ACCELEROMETER_DISABLE == 0)
  // lets register for the accelerometer (30fps)
  sensor_request_events(SENSOR_TYPE_ACCELEROMETER);
  sensor_set_rate(SENSOR_TYPE_ACCELEROMETER, 1000000L / 30);
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

  // blackberry platform services: shutdown
  bps_shutdown();

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

#undef DIRECTORY_SEPARATOR 

/********************************* EOF ***********************************/
