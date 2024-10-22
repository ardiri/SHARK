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
// SHARK logs internally to the iPod/iPhone console - which can be viewed
// under xcode when doing development and debugging. it should be disabled
// when building versions for distribution.
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

/*************************************************************************
 * Configuration
 *************************************************************************/

#define GFX_SCALING_150

/*************************************************************************
 * Constants
 *************************************************************************/

#define DIRECTORY_SEPARATOR        '/'

#define APPLICATION_LOG_FILE       "application-runtime.txt"

#define APPLICATION_WIDTH          480
#define APPLICATION_HEIGHT         320

#ifdef GFX_SCALING_150

#define APPLICATION_WINDOW_WIDTH   720
#define APPLICATION_WINDOW_HEIGHT  480

#else

#define APPLICATION_WINDOW_WIDTH   APPLICATION_WIDTH
#define APPLICATION_WINDOW_HEIGHT  APPLICATION_HEIGHT

#endif

// options: special
#define OPT_NO_PAUSE_ON_FOCUS_LOST 0x0001

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
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <alsa/asoundlib.h>

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
  uint32        options;       \
                               \
  Display      *display;       \
  int           screen;        \
                               \
  Window        window;        \
  Atom          exit;          \
  char          title_str[64]; \
  char         *title_list[1]; \
  XTextProperty title;         \
                               \
  int           timer_ms;      \
                               \
  boolean       pendown;       \
                               \
  FILE         *log;

#define g_argc                 (GLOBALS) -> argc
#define g_argv                 (GLOBALS) -> argv

#define g_options              (GLOBALS) -> options

#define g_display              (GLOBALS) -> display
#define g_screen               (GLOBALS) -> screen

#define g_window               (GLOBALS) -> window
#define g_exit                 (GLOBALS) -> exit
#define g_title_str            (GLOBALS) -> title_str
#define g_title_list           (GLOBALS) -> title_list
#define g_title                (GLOBALS) -> title

#define g_timer_ms             (GLOBALS) -> timer_ms

#define g_pendown              (GLOBALS) -> pendown

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

static void DAL_TimerProc(int);

void
DAL_ProcessArguments(int argc, char **argv)
{
  char *p, *g_path_p;
  int   i;

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

  g_options = 0;
  for (i=1; i<argc; i++)
  {
    if (strcmp(argv[i], "--noPauseOnFocusLost") == 0) 
      g_options |= OPT_NO_PAUSE_ON_FOCUS_LOST;
  }
}

boolean
DAL_Initialize(uint16 width, uint16 height)
{
  boolean              result;
  XSetWindowAttributes windowAttr;
  unsigned long        windowMask, eventMask;
  XSizeHints           windowHints;
  XWMHints             windowManagerHints;
  int                  x, y, depth;
  struct sigaction     action;
  struct itimerval     timer;
  XEvent               e;
  XWindowAttributes    wA;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // open the display
  g_display = XOpenDisplay(NULL);
  if (g_display == NULL) goto DAL_Initialize_DONE;

  // get a handle on the default screen
  g_screen = DefaultScreen(g_display);

  // create the window
  windowAttr.border_pixel      = BlackPixel(g_display, g_screen);
  windowAttr.background_pixel  = BlackPixel(g_display, g_screen);
  windowAttr.override_redirect = False;
  windowMask = CWBackPixel | CWBorderPixel;

  // we want the window to be in the center of the display
  x = (DisplayWidth(g_display, g_screen)  - width)  >> 1;
  y = (DisplayHeight(g_display, g_screen) - height) >> 1;
  depth = DefaultDepth(g_display, g_screen);

  g_window =
    XCreateWindow(g_display, RootWindow(g_display, g_screen),
                  x, y, width, height, 1, depth,
                  InputOutput, CopyFromParent, windowMask, &windowAttr);

  // configure window and window manager hints
  windowManagerHints.initial_state = NormalState;
  windowManagerHints.flags         = StateHint;
  XSetWMHints(g_display, g_window, &windowManagerHints);

  windowHints.flags      = PPosition | PSize;
  windowHints.x          = x;
  windowHints.y          = y;
  windowHints.width      = width;
  windowHints.height     = height;
  XSetWMNormalHints(g_display, g_window, &windowHints);

  // lets register for WM_DELETE_WINDOW
  g_exit = XInternAtom(g_display, "WM_DELETE_WINDOW", True);
  XSetWMProtocols(g_display, g_window, &g_exit, 1);

  // set the window title
  strcpy(g_title_str, g_config.name);
  g_title_list[0] = g_title_str;
  XStringListToTextProperty((char **)&g_title_list, 1, &g_title);
  XSetWMName(g_display, g_window, &g_title);

  // what events do we want to know about
  eventMask  = 0;
  eventMask |= ButtonPressMask;     // mouse down event
  eventMask |= ButtonReleaseMask;   // mouse up event
  eventMask |= PointerMotionMask;   // mouse motion
  eventMask |= KeyPressMask;        // key down
  eventMask |= KeyReleaseMask;      // key up
  eventMask |= FocusChangeMask;     // focus in/out
  eventMask |= ExposureMask;        // expose / redraw
  eventMask |= StructureNotifyMask; // struture notify
  XSelectInput(g_display, g_window, eventMask);

  // draw the window
  XMapWindow(g_display, g_window);
  for(;;)
  {
    XNextEvent(g_display, &e);
    if (e.type == MapNotify) break;
  }
  XGetWindowAttributes(g_display, g_window, &wA);

  // prevent the window from resizing
  windowHints.flags      = PMinSize | PMaxSize;
  windowHints.min_width  = wA.width;
  windowHints.min_height = wA.height;
  windowHints.max_width  = wA.width;
  windowHints.max_height = wA.height;
  XSetWMNormalHints(g_display, g_window, &windowHints);

  // whats the desired fps the application wants us to use?
  g_timer_ms = 10000L / MAX(100, MIN(g_config.event.fps, 600)); // 10..60fps

  // initialize the interval timer
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = DAL_TimerProc;
  action.sa_flags   = SA_RESTART;
  sigemptyset(&action.sa_mask);
  sigaction(SIGALRM, &action, NULL);

  // schedule the first interval (timer)
  memset(&timer, 0, sizeof(struct itimerval));
  timer.it_value.tv_sec     =  g_timer_ms / 1000;
  timer.it_value.tv_usec    = (g_timer_ms % 1000) * 1000;
  setitimer(ITIMER_REAL, &timer, NULL);
  
  // configure the initial pen state
  g_pendown = false;

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

  // schedule the next interval (timer)
  memset(&timer, 0, sizeof(struct itimerval));
  timer.it_value.tv_sec     =  g_timer_ms / 1000;
  timer.it_value.tv_usec    = (g_timer_ms % 1000) * 1000;
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
  XEvent            e;
  event             _e;
  KeySym            key;
  int               x, y;
#ifdef GFX_SCALING_150
  int               _x, _y;
  XWindowAttributes wA;
#endif

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
  
  // while the application is still running
  while (g_running)
  {
    struct timeval timeout;
    fd_set rset;

    // configure our timeout variables
    timeout.tv_sec  = 0;
    timeout.tv_usec = (g_timer_ms * 500);  // half a nilEvent :)
   
    // prepare our connection timeout ioctl
    FD_ZERO(&rset);
    FD_SET(ConnectionNumber(g_display), &rset);

    // lets check if there are any events pending
    while (XPending(g_display) != 0)
    {
      XNextEvent(g_display, &e);

      // which event was received?     
      switch (e.type)
      {
        case Expose:
             // lets wait for all the expose events to come through
             while (XCheckTypedEvent(g_display, Expose, &e));

             // repaint the display
             _LCDRepaint();
             break;

        case ButtonRelease:
             g_pendown = false;
             break;

        case ButtonPress:
             x = ((XButtonEvent *)&e) -> x;
             y = ((XButtonEvent *)&e) -> y;

#ifdef GFX_SCALING_150

             XGetWindowAttributes(g_display, g_window, &wA);
             _x = (wA.width  - APPLICATION_WINDOW_WIDTH)  >> 1;
             _y = (wA.height - APPLICATION_WINDOW_HEIGHT) >> 1;

             x = ((x - _x) * 2) / 3;
             y = ((y - _y) * 2) / 3;

#endif

             // do we have some valid points?
             if ((x >= 0) && (x < g_width) &&
                 (y >= 0) && (y < g_height))
             {
               g_pendown = true;
               _SHARKPenEvent(x, y);
             }
             break;

        case MotionNotify:
             x = ((XButtonEvent *)&e) -> x;
             y = ((XButtonEvent *)&e) -> y;

#ifdef GFX_SCALING_150

             XGetWindowAttributes(g_display, g_window, &wA);
             _x = (wA.width  - APPLICATION_WINDOW_WIDTH)  >> 1;
             _y = (wA.height - APPLICATION_WINDOW_HEIGHT) >> 1;

             x = ((x - _x) * 2) / 3;
             y = ((y - _y) * 2) / 3;

#endif

             // do we have some valid points?
             if ((x >= 0) && (x < g_width) &&
                 (y >= 0) && (y < g_height))
             {
               if (g_pendown)
                 _SHARKPenEvent(x, y);
             }
             break;

        case KeyPress:
             XLookupString((XKeyEvent *)&e, NULL, 0, &key, NULL);
             _SHARKKeyEvent((uint16)key);
             break;

        case FocusIn:
             _SHARKFocusEvent(true);
             break;

        case FocusOut:
             // only if we want to capture focus out events
             if ((g_options & OPT_NO_PAUSE_ON_FOCUS_LOST) == 0)
               _SHARKFocusEvent(false);
             break;

        case ClientMessage:
             if ((((XClientMessageEvent *)&e)->data.l[0]) == g_exit)
             {
               memset(&_e, 0, sizeof(event));
               _e.eType = _appStopEvent;
               _SysEnqueueEvent(&_e);
             }
             break;

        default:
             break;
      }
    }

    // dispatch the events in SHARK
    if (_SysEventAvailable() && g_running)
      _SHARKDispatchEvents();

    // if nothing else to do; give wait for event to come through
    else
      select(ConnectionNumber(g_display)+1, &rset, NULL, NULL, &timeout);
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

  // entry condition
  if (g_display == NULL) return;

  // shutdown shark
  _SHARKTerminate();

  // terminate the interval timer
  memset(&timer, 0, sizeof(struct itimerval));
  setitimer(ITIMER_REAL, &timer, NULL);

  // flush event buffer
  XFlush(g_display);

  // destroy the window 
  XDestroyWindow(g_display, g_window);

  // close the display
  XCloseDisplay(g_display);
}

int 
main(int argc, char **argv)
{
  // initialize our globals
  memset(GLOBALS, 0, sizeof(GLOBALS));

  // store a reference to the program arguments
  g_argc = argc;
  g_argv = argv;
  DAL_ProcessArguments(argc, argv);

  // obtain the application configuration parameters
  memset(&g_config, 0, sizeof(application_config));
  ApplicationConfig(&g_config);

#if (LOG_DISABLE == 0)
  // initialize the logging engine
  DAL_LogInitialize();
#endif

  // initialize the platform
  g_running = true;
  if (DAL_Initialize(APPLICATION_WINDOW_WIDTH, APPLICATION_WINDOW_HEIGHT))
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

#undef APPLICATION_WINDOW_HEIGHT
#undef APPLICATION_WINDOW_WIDTH
#undef APPLICATION_HEIGHT
#undef APPLICATION_WIDTH

#undef APPLICATION_LOG_FILE
#undef DIRECTORY_SEPARATOR

/********************************* EOF ***********************************/
