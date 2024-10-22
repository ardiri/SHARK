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

#define DIRECTORY_SEPARATOR        '\\'

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

/*************************************************************************
 * System Includes
 *************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <direct.h>
#include <winsock.h>

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
  int               argc;          \
  char            **argv;          \
                                   \
  HWND              hWnd;          \
  HINSTANCE         hInst;         \
                                   \
  UINT              timer;         \
  CRITICAL_SECTION  wcs;           \
                                   \
  FILE             *log;

#define g_argc                 (GLOBALS) -> argc
#define g_argv                 (GLOBALS) -> argv

#define g_hWnd                 (GLOBALS) -> hWnd
#define g_hInst                (GLOBALS) -> hInst

#define g_timer                (GLOBALS) -> timer 
#define g_wcs                  (GLOBALS) -> wcs   

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

#define IDS_DEFAULT_APP_NAME_ID 1
#define IDI_DEFAULT_ICON        101

static void    CALLBACK DAL_TimerProc(UINT, UINT, DWORD, DWORD, DWORD);
static LRESULT CALLBACK DAL_WndProc(HWND, UINT, WPARAM, LPARAM);

char **
CommandLineToArgvA(char *CmdLine, int* _argc)
{
  PCHAR*  argv;
  PCHAR   _argv;
  ULONG   len;
  ULONG   argc;
  CHAR    a;
  ULONG   i, j;

  BOOLEAN  in_QM;
  BOOLEAN  in_TEXT;
  BOOLEAN  in_SPACE;

  len = strlen(CmdLine);
  i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

  argv  = (PCHAR*)GlobalAlloc(GMEM_FIXED, i + (len+2)*sizeof(CHAR));
  _argv = (PCHAR)(((PUCHAR)argv)+i);

  argc = 0;
  argv[argc] = _argv;
  in_QM = FALSE;
  in_TEXT = FALSE;
  in_SPACE = TRUE;
  i = 0;
  j = 0;

  while (a = CmdLine[i]) 
  {
    if (in_QM) 
    {
      if (a == '\"') in_QM = FALSE;
      else _argv[j++] = a;
    } 
    else 
    {
      switch(a) 
      {
        case '\"':
             in_QM = TRUE;
             in_TEXT = TRUE;
             if(in_SPACE) argv[argc++] = _argv+j;
             in_SPACE = FALSE;
             break;

        case ' ':
        case '\t':
        case '\n':
        case '\r':
             if (in_TEXT) _argv[j++] = '\0';
             in_TEXT = FALSE;
             in_SPACE = TRUE;
             break;

        default:
             in_TEXT = TRUE;
             if (in_SPACE) argv[argc++] = _argv+j;
             _argv[j++] = a;
             in_SPACE = FALSE;
             break;
      }
    }
    i++;
  }
  _argv[j] = '\0';
  argv[argc] = NULL;

  (*_argc) = argc;
  return argv;
}

void
DAL_ProcessArguments(int argc, char **argv)
{
  // we need to know the applications path
  strcpy(g_path, argv[0]);
  *(strrchr(g_path, '\\')) = 0; // remove the executable name
}

boolean
DAL_Initialize(uint16 width, uint16 height)
{
  boolean  result;
  TCHAR    szTitle[100];
  RECT     rect, clientRect;
  WNDCLASS wc;
  DWORD    winFlags;
  UINT     timerRes;
  TIMECAPS tc;
  uint32   ticksPerFrame;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // lets see if the application is already running
  strcpy(szTitle, g_config.name);
  g_hWnd = FindWindow("APPL", szTitle);
  if (g_hWnd != NULL)
  {
    // bring it to the foreground
    SetForegroundWindow(g_hWnd);
    ShowWindow(g_hWnd, SW_RESTORE);

    goto DAL_INIT_DONE;
  }

  // register class
  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = (WNDPROC)DAL_WndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = g_hInst;
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.lpszMenuName  = 0;
  wc.hIcon         = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_DEFAULT_ICON));
  wc.lpszClassName = "APPL";
  RegisterClass(&wc);

  // create the window
  rect.top    = 0;
  rect.left   = 0;
  rect.right  = APPLICATION_WINDOW_WIDTH  +
                GetSystemMetrics(SM_CXBORDER)    +
                GetSystemMetrics(SM_CXFRAME);     
  rect.bottom = APPLICATION_WINDOW_HEIGHT +
                GetSystemMetrics(SM_CYBORDER)    +
                GetSystemMetrics(SM_CYFRAME)     +
                GetSystemMetrics(SM_CYCAPTION);
  winFlags    = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, TRUE);
  g_hWnd = CreateWindow("APPL", szTitle, winFlags,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        (rect.right - rect.left), (rect.bottom - rect.top),
                        NULL, NULL, g_hInst, NULL);
  if (g_hWnd == NULL) goto DAL_INIT_DONE;

  // center the window on the display
  GetClientRect(GetDesktopWindow(), &clientRect);
  width  = (uint16)(clientRect.right  - clientRect.left);
  height = (uint16)(clientRect.bottom - clientRect.top);

  // center the window on the desktop
  MoveWindow(g_hWnd, 0, 0,
             (rect.right - rect.left), (rect.bottom - rect.top), TRUE);
  GetClientRect(g_hWnd, &clientRect);
  MoveWindow(g_hWnd,
             (width  - clientRect.right)  >> 1,
             (height - clientRect.bottom) >> 1,
             (rect.right - rect.left - (clientRect.right  - width)),
             (rect.bottom - rect.top - (clientRect.bottom - height)), TRUE);

  // display window and set focus
  UpdateWindow(g_hWnd);
  ShowWindow(g_hWnd, SW_SHOW);
  SetFocus(g_hWnd);

  // initialize event loop semaphore
  InitializeCriticalSection(&g_wcs);

  // install a nilevent timer for required fps
  if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) == TIMERR_NOERROR)
  {
    ticksPerFrame = 10000L / MAX(100, MIN(g_config.event.fps, 600));  
    timerRes = MIN(MAX(tc.wPeriodMin, ticksPerFrame), tc.wPeriodMax);
    timeBeginPeriod(timerRes);
    g_timer = timeSetEvent(ticksPerFrame, timerRes,
                           DAL_TimerProc, (DWORD)NULL, TIME_PERIODIC);
  }

  // startup SHARK
  result = _SHARKInitialize(APPLICATION_WIDTH, APPLICATION_HEIGHT);

  // the application is now active
  g_active    = true;

DAL_INIT_DONE:;

  return result;
}

static LRESULT CALLBACK
DAL_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  LRESULT     result;
  HDC         hdc;
  PAINTSTRUCT ps;
  RECT        rect;
  uint16      width, height, x_adj, y_adj;
  int16       x, y;
  uint16      key;
  event       e;
  MSG         msg;
  boolean     processed;

  // default return value
  result    = 0;
  processed = false;

  // lets ensure we are dealing with the right window
  if (g_hWnd == hWnd)
  {
    switch (message)
    { 
      case WM_CREATE:
           processed = true;
           break;

      case WM_GETMINMAXINFO:

           // what are the additional pieces we need to consider
           x_adj = GetSystemMetrics(SM_CXBORDER)    +
                   GetSystemMetrics(SM_CXFRAME);
           y_adj = GetSystemMetrics(SM_CYBORDER)    +
                   GetSystemMetrics(SM_CYFRAME)     +
                   GetSystemMetrics(SM_CYCAPTION);

           // our screen may be smaller than our max - lets find out
           SystemParametersInfo(SPI_GETWORKAREA, 0, (void *)&rect, 0);
           width  = (int16)(rect.right  - rect.left);
           height = (int16)(rect.bottom - rect.top);
           width  = MIN(APPLICATION_WINDOW_WIDTH  + x_adj, width);
           height = MIN(APPLICATION_WINDOW_HEIGHT + y_adj, height);

           ((MINMAXINFO *)lParam)->ptMinTrackSize.x = width;
           ((MINMAXINFO *)lParam)->ptMinTrackSize.y = height;
           ((MINMAXINFO *)lParam)->ptMaxTrackSize.x = width;
           ((MINMAXINFO *)lParam)->ptMaxTrackSize.y = height;

           ((MINMAXINFO *)lParam)->ptMaxSize.x =
             ((MINMAXINFO *)lParam)->ptMaxTrackSize.x;
           ((MINMAXINFO *)lParam)->ptMaxSize.y =
             ((MINMAXINFO *)lParam)->ptMaxTrackSize.y;
           break;

      case WM_SETFOCUS:
      case WM_ACTIVATE:
      case WM_KILLFOCUS:

           if ((message == WM_KILLFOCUS) ||
              ((message == WM_ACTIVATE) && (LOWORD(wParam) == WA_INACTIVE)))
           {
             SetFocus(NULL);
             _SHARKFocusEvent(false);
           }
           else
           if ((message == WM_SETFOCUS) ||
              ((message == WM_ACTIVATE) && (LOWORD(wParam) == WA_ACTIVE)))
           {
             SetFocus(g_hWnd);
             _SHARKFocusEvent(true);
           }
           processed = true;
           break;

      case WM_TIMER:

           // make sure there no other timer events waiting
           while (PeekMessage(&msg, g_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));

           if (g_active)
           {
             memset(&e, 0, sizeof(event));
             e.eType = _nilEvent;
             _SysEnqueueEvent(&e);
           }
           break;

      case WM_LBUTTONDOWN:
      case WM_MOUSEMOVE:

           // lets ensure the mouse is pressed
           if (GetAsyncKeyState(VK_LBUTTON))
           {
             x = (int16)LOWORD(lParam);
             y = (int16)HIWORD(lParam);

#ifdef GFX_SCALING_150

             x = (x * 2) / 3;
             y = (y * 2) / 3;

#endif

             // do we have some valid points?
             if ((x >= 0) && (x < g_width) &&
                 (y >= 0) && (y < g_height))
             {
               _SHARKPenEvent(x, y);
             }
           }
           processed = true;
           break;

      case WM_KEYDOWN:
           key = (uint16)wParam;
           _SHARKKeyEvent(key);
           processed = true;
           break;
     
      case WM_PAINT:
           hdc = BeginPaint(g_hWnd, &ps);
           _LCDRepaint();
           EndPaint(g_hWnd, &ps);
           processed = true;
           break;

      case WM_CLOSE:
           _SysEnqueueQuitEvent();
           processed = true;
           break;

      case WM_DESTROY:
           PostQuitMessage(0);
           g_hWnd = NULL;
           processed = true;
           break;

      default:
           break;
    }
  }
 
  // if we didn't process the event; pass it on to the default handler
  if (!processed)
    result = DefWindowProc(hWnd, message, wParam, lParam);

  return result;
}

static void CALLBACK
DAL_TimerProc(UINT id, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
  event e;

  if (g_active)
  {
    memset(&e, 0, sizeof(event));
    e.eType = _nilEvent;
    _SysEnqueueEvent(&e);
  }

  // post a WM_TIMER event for the event loop to trigger
  PostMessage(g_hWnd, WM_TIMER, 0, 0);
}

void
DAL_InitApplication()
{
  _PreferencesLoad();
}

void
DAL_EventLoop()
{
  event _e;
  MSG   msg;

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

  // main message loop:
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    // dispatch the events in SHARK
    if (_SysEventAvailable() && g_running)
    {
      EnterCriticalSection(&g_wcs);
      _SHARKDispatchEvents();
      LeaveCriticalSection(&g_wcs);
    }

    // application termination condition
    if (!g_running)
      DestroyWindow(g_hWnd);
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
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // shutdown shark
  _SHARKTerminate();

  // shutdown event loop semaphore
  DeleteCriticalSection(&g_wcs);

  // terminate the interval timer
  timeKillEvent(g_timer);
}

// application main-line interface
int WINAPI 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
        LPTSTR lpCmdLine, int nCmdShow)
{
  char **argv;
  int    argc;

  // initialize our globals
  memset(GLOBALS, 0, sizeof(GLOBALS));

  // store a reference to the program arguments
  argv = CommandLineToArgvA(GetCommandLine(), &argc);
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

  // we need to store a copy of this instance
  g_hInst = hInstance;

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

  // free memory used to get program command line
  LocalFree(argv);

  return 0;
}

#undef IDS_DEFAULT_APP_NAME_ID
#undef IDI_DEFAULT_ICON

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
