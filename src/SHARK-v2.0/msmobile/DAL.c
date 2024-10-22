/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL.c
 */

// system includes
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <aygshell.h>

// SHARK:: macro overrides
#ifdef INPUT_KEYBOARD
#undef INPUT_KEYBOARD
#endif

// SHARK includes
#include "../SHARK.h"
#include "../SHARK-prv.h"

#define IDS_DEFAULT_APP_NAME_ID 1
#define MAX_LOADSTRING          100

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

static HWND               g_hWnd;
static HINSTANCE          g_hInst;
static boolean            g_active;
static boolean            g_initialized;
static SHACTIVATEINFO     g_shai;

#if (LOG_DISABLE == 0)
static FILE              *g_logfile;
#endif

// globally shared information
static uint8              g_lcd_a;
static uint8              g_lcd_r;
static uint8              g_lcd_g;
static uint8              g_lcd_b;
static uint32             g_lcd_r_mask;
static uint32             g_lcd_g_mask;
static uint32             g_lcd_b_mask;

#define LCD_DEPTH (uint8)(((g_lcd_a+g_lcd_r+g_lcd_g+g_lcd_b) + 15) & ~0xf)

static coord              g_width;
static coord              g_width_rb;
static coord              g_height;
static coord              g_height_rb;

static char               g_path[256];
static application_config g_config;

/*************************************************************************
 *
 * Local Dependencies
 *
 *************************************************************************/

#if (LOG_DISABLE == 0)
static void _LOGInitialize();
static void _LOGCreateEntry(char *str);
static void _LOGTerminate();
#endif

static boolean DAL_HandleEvent(event *e);
static void    DAL_DispatchEvents();

// any routines that are missing from the base libraries defined here
#include "DAL-missing.inc"

#include "DAL-memory.inc"
#include "DAL-display.inc"
#include "DAL-system.inc"
#include "DAL-library.inc"
#include "DAL-input.inc"
#include "DAL-preferences.inc"
#include "DAL-file.inc"
#include "DAL-audio.inc"
#include "DAL-resource.inc"

/*************************************************************************
 *
 * Platform Depending Application Framework
 *
 *************************************************************************/

#if (LOG_DISABLE == 0)
// initialize the logging process
static void
_LOGInitialize()
{
  uint32 platform;
  char   platform_name[256];
  char   fileName[256];
  uint32 input;

  // application logfile file name 
  sprintf(fileName, "%s%capplication-runtime.txt", 
          g_path, DIRECTORY_SEPARATOR);

  g_logfile = fopen(fileName, "w");
  if (g_logfile != NULL)
  {
    fprintf(g_logfile, "APPLICATION_LOG_FILE:\n");
    fprintf(g_logfile, "---------------------\n");
    fprintf(g_logfile, "\n");

    _SysDeviceInfo(&platform, platform_name);
    fprintf(g_logfile, "PLATFORM:\n");
    fprintf(g_logfile, "platform id\t:: %s\n", platform_name);
    fprintf(g_logfile, "\n");

    input = _InputSupported();
    fprintf(g_logfile, "INPUT SUPPORTED:\n");
    fprintf(g_logfile, "stylus / mouse\t:: ");
    fprintf(g_logfile, (input & INPUT_STYLUS) ? "yes\n" : "no\n");
    fprintf(g_logfile, "digital keys\t:: ");
    fprintf(g_logfile, (input & INPUT_KEYS) ? "yes\n" : "no\n");
    fprintf(g_logfile, "analog control\t:: ");
    fprintf(g_logfile, (input & INPUT_KEYS_ANALOG) ? "yes\n" : "no\n");
    fprintf(g_logfile, "phonepad\t:: ");
    fprintf(g_logfile, (input & INPUT_PHONEPAD) ? "yes\n" : "no\n");
    fprintf(g_logfile, "keyboard\t:: ");
    fprintf(g_logfile, (input & INPUT_KEYBOARD) ? "yes\n" : "no\n");
    fprintf(g_logfile, "\n");

    fprintf(g_logfile, "SOFTWARE:\n");
    fprintf(g_logfile, "company    \t:: %s\n", g_config.company);
    fprintf(g_logfile, "application\t:: %s\n", g_config.name);
    fprintf(g_logfile, "\n");

    fprintf(g_logfile, "DISPLAY:\n");
    fprintf(g_logfile, "display depth\t:: %d\n", LCD_DEPTH);
    fprintf(g_logfile, "display RGB\t:: %d%d%d\n", g_lcd_r, g_lcd_g, g_lcd_b);
    fprintf(g_logfile, "display R mask\t:: 0x%08lx\n", g_lcd_r_mask);
    fprintf(g_logfile, "display G mask\t:: 0x%08lx\n", g_lcd_g_mask);
    fprintf(g_logfile, "display B mask\t:: 0x%08lx\n", g_lcd_b_mask);
    fprintf(g_logfile, "\n");

    fprintf(g_logfile, "AUDIO STREAMING:\n");
    if (_SndStreamAvailable())
    {
      uint32 rates;
      rates = _SndStreamRatesSupported();

      fprintf(g_logfile, "available  \t:: yes\n");
      fprintf(g_logfile, "rates      \t:: ");

      if (rates & SND_STREAM_8KHZ)   fprintf(g_logfile, "8Khz ");
      if (rates & SND_STREAM_11KHZ)  fprintf(g_logfile, "11Khz ");
      if (rates & SND_STREAM_16KHZ)  fprintf(g_logfile, "16Khz ");
      if (rates & SND_STREAM_22KHZ)  fprintf(g_logfile, "22Khz ");
      if (rates & SND_STREAM_44KHZ)  fprintf(g_logfile, "44Khz ");
      if (rates & SND_STREAM_MONO)   fprintf(g_logfile, "[mono] ");
      if (rates & SND_STREAM_STEREO) fprintf(g_logfile, "[stereo] ");
      if (rates == 0) fprintf(g_logfile, "- none -");
      fprintf(g_logfile, "\n");
    }
    else
      fprintf(g_logfile, "available  \t:: no\n");
  }
}

// create an entry in the log file
static void
_LOGCreateEntry(char *str)
{
  if (g_logfile != NULL)
  {
    fprintf(g_logfile, "%s\n", str);
    fflush(g_logfile);
  }
}

// terminate the logging process; close log file
static void
_LOGTerminate()
{
  if (g_logfile != NULL)
  {
    fprintf(g_logfile, "\n");
    fprintf(g_logfile, "- END\n");
    fclose(g_logfile);
  }
}
#endif

static boolean
DAL_HandleEvent(event *e)
{
  boolean processed;

  processed = _LibHandleEvent(e);
  if (!processed) 
    processed = ApplicationHandleEvent(e);

  return processed;
}

static void
DAL_DispatchEvents()
{
  event  _e;
  uint32 eType;

  // platform independent 'processing' of events
  while (_SysEventAvailable() && (g_hWnd != NULL))
  {
    _SysGetEvent(&_e);
    eType = _e.eType;
    DAL_HandleEvent(&_e);

    // application termination condition
    if (eType == _appStopEvent)
    {
      DestroyWindow(g_hWnd);
      g_hWnd = NULL;
    }
  }
}

static boolean
GlobalsInitialize()
{
  boolean result;

  // initialize all sub-components
  result = true;
  result &= _MemInitialize();  // REQ: must be done first
  result &= _LCDInitialize();  // REQ: must be done second
  result &= _SysInitialize();
  result &= _LibInitialize();
  result &= _InputInitialize();
  result &= _PreferencesInitialize();
  result &= _ResourceInitialize();

  // these two components are "optional"
  _FileInitialize();
  _SndInitialize();

  // initialize the default orientation state based on window resolution
  _LCDSetOrientation(lcd_rotate0);

  // load any additional libraries (all optional)
  if (g_config.gfx.active)
    result &= (_GfxInitialize(&g_config.gfx.format) || !g_config.gfx.required);
  if ((g_config.font.active) && (g_config.gfx.active))
    result &= (_FntInitialize(&g_config.gfx.format,
                               g_config.font.encoding,
                               g_config.font.style) || !g_config.font.required);
  if (g_config.sfx.active)
    result &= (_SfxInitialize(g_config.sfx.rate,
                              g_config.sfx.format,
                              g_config.sfx.type,
                              g_config.sfx.frames,
                              g_config.sfx.mix_count) || !g_config.sfx.required);

  return result;
}

static void
GlobalsTerminate()
{
  // shutdown any additional libraries
  if (g_config.sfx.active)   _SfxTerminate();
  if ((g_config.font.active) &&
      (g_config.gfx.active)) _FntTerminate();
  if  (g_config.gfx.active)  _GfxTerminate();

  _ResourceTerminate();
  _SndTerminate();
  _FileTerminate();
  _PreferencesTerminate();
  _InputTerminate();
  _LibTerminate();
  _SysTerminate();
  _LCDTerminate();             // REQ: must be done second last
  _MemTerminate();             // REQ: must be done last
}

static LRESULT CALLBACK
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  HDC         hdc;
  PAINTSTRUCT ps;
  LRESULT     result;
  event       e;
  boolean     processed;
  RECT        rect;
  uint16      width, height;
  MSG         msg;
  DWORD       shFlags;

  // default return value
  result    = 0;
  processed = false;

  // shell flags
  shFlags = SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON;

  // lets ensure we are dealing with the right window handle
  if (g_hWnd == hWnd)
  {
    switch (message)
    {
      case WM_CREATE:
           processed = true;
           break;

      case WM_SIZE:
           GetClientRect(g_hWnd, &rect);
           width  = (coord)(rect.right  - rect.left);
           height = (coord)(rect.bottom - rect.top);

           // this occurs if there is a minimize:: 0 0 window size
           if ((width == 0) || (height == 0)) goto DAL_SIZE_ABORT;

           // we only want to do something if something has changed
           if ((width != g_width) || (height != g_height))
           {
             g_active = false;

             // set the new properties
             g_width     = width;
             g_width_rb  = (uint16)(((g_width  + 3) & ~3) * (LCD_DEPTH >> 3));
             g_height    = height;
             g_height_rb = (uint16)(((g_height + 3) & ~3) * (LCD_DEPTH >> 3));

             // we need to re-initialize the SHARK LCD driver routines
             _LCDTerminate_RESIZE();
             _LCDInitialize_RESIZE();

             // now, the application must handle the new LCD instance
             memset(&e, 0, sizeof(event));
             e.eType        = _keyEvent;
             e.data.key.chr = _key_gfx_change;
             DAL_HandleEvent(&e);

             g_active = true;
           }

DAL_SIZE_ABORT:;

           processed = true;
           break;

      case WM_CHAR:

           // NOTE: we support max 0x0800 unicode base 16 for now!!
           if ((wParam >= 32) && (wParam < 0x800))
           {
             e.eType = _keyEvent;
             if (wParam < 0x80)
             {
               e.data.key.size = 1;
               e.data.key.chr  = (uint8)wParam;
             }
             else
             {
               e.data.key.size = 2;
               e.data.key.chr       = 0xc0 | (uint8)((wParam >> 6) & 0x03);
               e.data.key.chr_x[0]  = 0x80 | (uint8) (wParam       & 0x3f);
             }
             processed = DAL_HandleEvent(&e);
           }
           break;

      case WM_KEYDOWN:
           if (_KeyHandleEvent((uint16)(wParam), &e))
             processed = DAL_HandleEvent(&e);
           break;

      case WM_KEYUP:
           processed = true;
           break;

      case WM_SETFOCUS:
      case WM_ACTIVATE:
      case WM_KILLFOCUS:

           if ((message == WM_KILLFOCUS) ||
              ((message == WM_ACTIVATE) && (LOWORD(wParam) == WA_INACTIVE)))
           {
             g_active = false;
             SetFocus(NULL);

             // suspend all LCD operations
             _LCDSuspend();

             // since we have just LOST focus, lets pause!
             memset(&e, 0, sizeof(event));
             e.eType        = _keyEvent;
             e.data.key.chr = _key_focuslost;
             DAL_HandleEvent(&e); // INFORM THE GAME ENGINE RIGHT AWAY

             // stop the active audio stream
             _SndStreamStop(0);
           }
           else
           if ((message == WM_SETFOCUS) ||
              ((message == WM_ACTIVATE) && (LOWORD(wParam) == WA_ACTIVE)))
           {
             SetFocus(g_hWnd);
             SHFullScreen(g_hWnd, shFlags);

             // resume LCD operations
             _LCDResume();

             // restart the active audio stream
             _SndStreamStart(0);

             g_active = true;
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
             processed = DAL_HandleEvent(&e);
           }
           break;

      case WM_PAINT:
           hdc = BeginPaint(g_hWnd, &ps);
           _LCDRepaint();
           EndPaint(g_hWnd, &ps);
           processed = true;
           break;

      case WM_LBUTTONDOWN:
      case WM_MOUSEMOVE:
           if (_PenHandleEvent((coord)LOWORD(lParam),
                               (coord)HIWORD(lParam), &e))
             processed  = DAL_HandleEvent(&e);
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

      case WM_SETTINGCHANGE:
           SHHandleWMSettingChange(g_hWnd, wParam, lParam, &g_shai);
           processed = true;
           break;

      default:
           break;
    }
  }

  if (!processed)
    result = DefWindowProc(hWnd, message, wParam, lParam);

  return result;
}

static boolean
InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  HWND     hWnd;
  TCHAR    szTitle[MAX_LOADSTRING];    // the title bar text
  RECT     rect;
  WNDCLASS wc;
  DWORD    shFlags;
  boolean  result;

  result = false;

  // initialize global window strings
  mbstowcs(szTitle, g_config.name, strlen(g_config.name)); 

  // shell flags
  shFlags = SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON;

  // if it is already running, bring it into the foreground
  hWnd = FindWindow(TEXT("APPL"), szTitle);
  if (hWnd != NULL)
  {
    g_hWnd = hWnd;

    SHFullScreen(g_hWnd, shFlags);
    SetForegroundWindow((HWND)(((DWORD)g_hWnd) | 0x01));
    return false;
  }

  // register class
  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = (WNDPROC)WndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wc.hCursor       = 0;
  wc.lpszMenuName  = 0;
  wc.hIcon         = 0;
  wc.lpszClassName = TEXT("APPL");
  RegisterClass(&wc);

  // create the window
  rect.top    = 0;
  rect.left   = 0;
  rect.right  = g_width;
  rect.bottom = g_height;

  hWnd = CreateWindow(TEXT("APPL"), szTitle, WS_VISIBLE,
                      rect.left, rect.top,
                      (rect.right - rect.left), (rect.bottom - rect.top),
                      NULL, NULL, hInstance, NULL);

  // window created successfully?
  if (hWnd != NULL)
  {
    g_hWnd = hWnd;

    SHFullScreen(g_hWnd, shFlags);

    // display window and set focus
    UpdateWindow(g_hWnd);
    ShowWindow(g_hWnd, nCmdShow);
    SetFocus(g_hWnd);
    SetForegroundWindow((HWND)(((DWORD)g_hWnd) | 0x01));

    // initialize globals
    g_hInst     = hInstance;
    g_active    = true;

    // everything is ok, lets go!
    result = true;
  }

  return result;
}

static void
InitApplication()
{
  // load the application preferences
  _PreferencesLoad();
}

static void
EventLoop()
{
  event  _e;
  MSG    msg;

  // basic initialization (and, loading screen)
  memset(&_e, 0, sizeof(event));
  _e.eType = _appStartEvent;
  DAL_HandleEvent(&_e);

  // application is ready to startup - can do stuff like loading here
  memset(&_e, 0, sizeof(event));
  _e.eType = _appGoEvent;
  DAL_HandleEvent(&_e);

  // main message loop:
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    // platform independent 'processing' of events
    DAL_DispatchEvents();
  }
}

static void
EndApplication()
{
  // save the application preferences
  _PreferencesSave();
}

// application main-line interface
int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
        LPTSTR lpCmdLine, int nCmdShow)
{
  int32 i;
  TCHAR lpFileName[MAX_PATH];

  // obtain the application configuration parameters
  memset(&g_config, 0, sizeof(application_config));
  ApplicationConfig(&g_config);

  // we need to keep track of the base path of this application
  GetModuleFileName(NULL, lpFileName, MAX_PATH);
  wcstombs(g_path, lpFileName, MAX_PATH);
  i = strlen(g_path); while (g_path[i] != '\\') { i--; } g_path[i] = 0;
  chdir(g_path);

  // this is our "setting" for our LCD engine (hard coded)
  g_lcd_a = 0;
  g_lcd_r = 5;
  g_lcd_g = 6;
  g_lcd_b = 5;
  g_lcd_r_mask = ((0xff >> (8 - g_lcd_r)) << (g_lcd_g + g_lcd_b));
  g_lcd_g_mask = ((0xff >> (8 - g_lcd_g)) << (g_lcd_b));
  g_lcd_b_mask =  (0xff >> (8 - g_lcd_b));

#if (LOG_DISABLE == 0)
  // initialize the log file function
  _LOGInitialize();
#endif

  // standard default window size
  g_width     = GetDeviceCaps(NULL, HORZRES);
  g_width_rb  = (uint16)(((g_width  + 3) & ~3) * (LCD_DEPTH >> 3));
  g_height    = GetDeviceCaps(NULL, VERTRES);
  g_height_rb = (uint16)(((g_height + 3) & ~3) * (LCD_DEPTH >> 3));

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry("\n:: - hardware abstraction initialize -\n");
#endif

  // application startup
  g_initialized = false;
  if (InitInstance(hInstance, nCmdShow))
  {
    if (GlobalsInitialize())
    {
#if (LOG_DISABLE == 0)
      // LOGGING EVENT
      _LOGCreateEntry("\n:: - application initialize -");
#endif

      if (ApplicationInitialize())
      {
        g_initialized = true;

        InitApplication();
        EventLoop();
        EndApplication();

        g_initialized = false;
      }
      ApplicationTerminate();

#if (LOG_DISABLE == 0)
      // LOGGING EVENT
      _LOGCreateEntry(":: - application terminate -\n");
#endif
    }

    GlobalsTerminate();
  }

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry("\n:: - hardware abstraction terminate -");

  // shutdown the log file function
  _LOGTerminate();
#endif

  return FALSE;
}

/********************************* EOF ***********************************/
