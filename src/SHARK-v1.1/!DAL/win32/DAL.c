/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL.c
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

static DAL_Globals globals;
DAL_Globals *g;

DAL_Globals *
DAL_GetGlobals()
{
  return (DAL_Globals *)&globals;
}

/*************************************************************************
 *
 * Platform Depending Application Framework
 *
 *************************************************************************/

#define RES_COUNT 20   // there are 20 resolutions available right now

uint16  res_x[]  = { 160, 160, 176, 176, 208, 208, 220, 240, 240, 240, 320, 320, 320, 320, 320, 320, 352, 448, 480, 480, };
uint16  res_y[]  = { 160, 240, 208, 220, 176, 320, 176, 160, 240, 320, 208, 240, 320, 352, 448, 480, 320, 320, 320, 480, };
int     stylus[] = {   1,   1,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1, };

boolean
DAL_Initialize()
{
  boolean             result;
  Win32_PlatformInfo *platform;
  FILE               *f;
  GLOBALS_ACCESS;

  result = false;

  // allocate the information block
  g->platform        = (void *)SYSTEM_MemPtrNew(sizeof(Win32_PlatformInfo));
  g->device.platform = (void *)SYSTEM_MemPtrNew(sizeof(Win32_DeviceInfo));
  if ((g->platform != NULL) && (g->device.platform != NULL))
  {
    _MemSet(g->platform, sizeof(Win32_PlatformInfo), 0);
    _MemSet(g->device.platform, sizeof(Win32_DeviceInfo), 0);

    platform = g->platform;
    f = fopen("device.cfg", "r");
    platform->log.active = (f != NULL);
    if (f != NULL) fclose(f);
  
    // initialize logging
    LOGInitalize();
  
    // load configuration options
    DAL_Configuration();
    result = true;
  }

  return result;
}

void
DAL_Configuration()
{
  Win32_DeviceInfo *device;
  FILE             *f;
  uint8             line[128];
  uint8             key[64], value[64], *v;
  uint32            options;
  GLOBALS_ACCESS;

  // access device information block
  device = (Win32_DeviceInfo *)g->device.platform;
  if (device == NULL) return;

  // initialize device information (nullify)
  options = 0;

  // process the configuration file
  f = fopen("device.cfg", "r");
  if (f != NULL)
  {
    // process the file fully
    while (!feof(f))
    {
      fgets(line, 128, f);

      // option directive?
      if (sscanf(line, "@%s%s\n", &key, &value) == 2)
      {
        // skip the '=' and ' ' or '\t' et al
        v = value; while ((*v == '=') || (*v == ' ') || (*v == '\t')) v++;

        // process the value 
        if (strcmp(key, "width")  == 0)   g->device.display.width  = atoi(v); 
        if (strcmp(key, "height") == 0)   g->device.display.height = atoi(v); 
        if (strcmp(key, "depth")  == 0)   g->device.display.depth  = atoi(v); 
        if (strcmp(key, "stylus") == 0) { g->device.input.stylus   = (*v == 'y'); options |= OPTIONS_INPUT_STYLUS;      }

        if (device != NULL)
        {
          if (strcmp(key, "heap")    == 0) device->memory.heap_size    = atoi(v) * 1024; 
          if (strcmp(key, "storage") == 0) device->memory.storage_size = atoi(v) * 1024; 
          if (strcmp(key, "memwarn") == 0) device->memory.warnings     = atoi(v); 
        }
      }

      if (g->device.display.width    != 0) options |= OPTIONS_DISPLAY_WIDTH;
      if (g->device.display.height   != 0) options |= OPTIONS_DISPLAY_HEIGHT;
      if ((g->device.display.depth   == 1)  ||
          (g->device.display.depth   == 2)  ||
          (g->device.display.depth   == 4)  ||
          (g->device.display.depth   == 8)  ||
          (g->device.display.depth   == 12) ||
          (g->device.display.depth   == 16)) options |= OPTIONS_DISPLAY_DEPTH;  // valid depths

      if (device != NULL) 
      {
        if (device->memory.heap_size    >= 32768) options |= OPTIONS_MEMORY_HEAP;     // minimum of 32K
        if (device->memory.storage_size >= 32768) options |= OPTIONS_MEMORY_STORAGE;  // minimum of 32K
        if (device->memory.warnings     == 0)     options |= OPTIONS_MEMORY_WARNINGS; // show warnings?
      }
    }

    fclose(f);

    // round to 4 pixel boundary
    g->device.display.width  = (g->device.display.width  + 3) & ~0x03;
    g->device.display.height = (g->device.display.height + 3) & ~0x03;
  }
  
  // standard default configuration (if wasn't defined right)
  if ((options & OPTIONS_DISPLAY_WIDTH)     == 0) g->device.display.width  = 480;
  if ((options & OPTIONS_DISPLAY_HEIGHT)    == 0) g->device.display.height = 320;
  if ((options & OPTIONS_DISPLAY_DEPTH)     == 0) g->device.display.depth  = 16;
  if ((options & OPTIONS_INPUT_STYLUS)      == 0) g->device.input.stylus   = true;
  g->device.input.analog = false;

  if (device != NULL)
  {
    if ((options & OPTIONS_MEMORY_HEAP)     == 0) device->memory.heap_size    = 4194304; // 4MB dynamic heap
    if ((options & OPTIONS_MEMORY_STORAGE)  == 0) device->memory.storage_size = 4194304; // 4MB storage heap
    if ((options & OPTIONS_MEMORY_WARNINGS) == 0) device->memory.warnings     = true;    // show warnings?
  }

  // LOGGING EVENT
  {
    uint8 str_log[256];

    LOG_ENTRY("DAL: device configuration:");
    sprintf(str_log, "  width   = %d", g->device.display.width);
    LOG_ENTRY(str_log);
    sprintf(str_log, "  height  = %d", g->device.display.height);
    LOG_ENTRY(str_log);
    sprintf(str_log, "  depth   = %d", g->device.display.depth);
    LOG_ENTRY(str_log);
    sprintf(str_log, "  stylus  = %s", (g->device.input.stylus) ? "yes" : "no" );
    LOG_ENTRY(str_log);

    if (device != NULL)
    {
      sprintf(str_log, "  heap    = %ld", device->memory.heap_size);
      LOG_ENTRY(str_log);
      sprintf(str_log, "  storage = %ld", device->memory.storage_size);
      LOG_ENTRY(str_log);
    }
    LOG_ENTRY("");
  }
}

void
DAL_Terminate()
{
  GLOBALS_ACCESS;

  // terminate logging
  LOGTerminate();

  // release information blocks
  if (g->device.platform != NULL) SYSTEM_MemPtrFree(g->device.platform);
  if (g->platform        != NULL) SYSTEM_MemPtrFree(g->platform); 
  g->device.platform = NULL;
  g->platform        = NULL;
}

boolean
GlobalsInitialize()
{
  boolean result;

  // LOGGING EVENT
  LOG_ENTRY(":: - hardware abstraction initialize -");

  // initialize all sub-components
  result = true;
  result &= _MemInitialize();  // REQ: must be done first
  result &= _LCDInitialize();  // REQ: must be done second
  result &= _FileInitialize();
  result &= _InpInitialize();
  result &= _SndInitialize();
  result &= _SysInitialize();
  result &= PlatformInitialize();

  // LOGGING EVENT
  LOG_ENTRY(":: - SHARK library initialize -");

  result &= _GfxInitialize();
  result &= _StrInitialize();
  result &= _Gfx16Initialize();
  result &= _UIInitialize();

  return result;
}

void
GlobalsTerminate()
{
  // shutdown all sub-components
  _UITerminate();
  _Gfx16Terminate();
  _StrTerminate();
  _GfxTerminate();

  // LOGGING EVENT
  LOG_ENTRY(":: - SHARK library terminate -");

  PlatformTerminate();
  _SysTerminate();
  _SndTerminate();
  _InpTerminate();
  _FileTerminate();
  _LCDTerminate();             // REQ: must be done second last
  _MemTerminate();             // REQ: must be done last

  // LOGGING EVENT
  LOG_ENTRY(":: - hardware abstraction terminate -");
}

int
WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
  GLOBALS_ACCESS;

  // we need to initialize the globals data pointer
  g = (DAL_Globals *)DAL_GetGlobals();

  // reset global memory space
  _MemSet(g, sizeof(DAL_Globals), 0);

  // application startup
  if (DAL_Initialize())
  {
    if (InitInstance(hInstance, nCmdShow))
    {
      if (GlobalsInitialize())
      {
        if (DAL_ApplicationValidateDRM())
        {
          // LOGGING EVENT
          LOG_ENTRY("\n:: - application initialize -");
  
          if (ApplicationInitialize()) 
          {
            InitApplication();
            EventLoop();
            EndApplication();
          }
          ApplicationTerminate();
  
          // LOGGING EVENT
          LOG_ENTRY(":: - application terminate -\n");
        }
      }
      GlobalsTerminate();
    }
             
    DAL_Terminate();
  }

  return FALSE;
}

boolean
InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  Win32_PlatformInfo *platform;
  HWND                hWnd;
  TCHAR                     szTitle[MAX_LOADSTRING];    // the title bar text
  TCHAR               szWindowClass[MAX_LOADSTRING];    // the window class name
  RECT                rect, clientRect;
  WNDCLASS            wc;
  LONG                width, height;
  DWORD               winFlags;
  boolean             result;
  GLOBALS_ACCESS;

  result = false;

  // access device/platform information block
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // initialize global strings
    LoadString(hInstance, IDC_DEFAULT_CLASS_ID,    szWindowClass, MAX_LOADSTRING);
    LoadString(hInstance, IDS_DEFAULT_APP_NAME_ID, szTitle,       MAX_LOADSTRING);

    // if it is already running, bring it up
    hWnd = FindWindow(szWindowClass, szTitle);
    if (hWnd != NULL)
    {
      SetForegroundWindow(hWnd);
      ShowWindow(hWnd, SW_RESTORE);
      return false;
    }
  
    // register class
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = (WNDPROC)WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName  = (LPCSTR)(NULL);
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DEFAULT_ICON));
    wc.lpszClassName = szWindowClass;
    RegisterClass(&wc);

    winFlags = 0;
    if (ApplicationGetDisplayProperties() != DISPLAY_RESIZE_UNSUPPORTED) 
      sprintf((szTitle + strlen(szTitle)), " :: (%d %d)", g->device.display.width, g->device.display.height);
  
    // create the window
    rect.top    = 0;
    rect.left   = 0;
    rect.right  = g->device.display.width  + GetSystemMetrics(SM_CXBORDER) + GetSystemMetrics(SM_CXFRAME) + 1;
    rect.bottom = g->device.display.height + GetSystemMetrics(SM_CYBORDER) + GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CYCAPTION) + 1;
    if (ApplicationGetDisplayProperties() == DISPLAY_RESIZE_ARBITARY) 
    {
      winFlags |= WS_SIZEBOX;
      rect.right  += GetSystemMetrics(SM_CXSIZEFRAME);
      rect.bottom += GetSystemMetrics(SM_CYSIZEFRAME);
    }

    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, TRUE);
    hWnd = CreateWindow(szWindowClass, szTitle, 
                        winFlags | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                        CW_USEDEFAULT, CW_USEDEFAULT, 
                        (rect.right - rect.left), (rect.bottom - rect.top),
                        NULL, NULL, hInstance, NULL);
  
    // window created successfully?
    if (hWnd != NULL)
    {
      GetClientRect(GetDesktopWindow(), &clientRect);
      width  = clientRect.right  - clientRect.left;
      height = clientRect.bottom - clientRect.top;
  
      // center the window on the desktop
      MoveWindow(hWnd, 0, 0, (rect.right - rect.left), (rect.bottom - rect.top), TRUE);
      GetClientRect(hWnd, &clientRect);
      MoveWindow(hWnd, 
                 (width  - clientRect.right)  >> 1, 
                 (height - clientRect.bottom) >> 1, 
                 (rect.right  - rect.left - (clientRect.right  - g->device.display.width)),
                 (rect.bottom - rect.top  - (clientRect.bottom - g->device.display.height)), TRUE);
    
      // display window and set focus
      UpdateWindow(hWnd);
      ShowWindow(hWnd, SW_SHOW);
      SetFocus(hWnd);
   
      // initialize globals
      platform->windows.hInst       = hInstance;
      platform->windows.LCDWnd      = hWnd;
      platform->windows.LCDScreen   = GetDC(hWnd);
      platform->windows.hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_DEFAULT_CLASS_ID);
      platform->system.active       = true;

      // everything is ok, lets go!  
      result = true;
    }

    // normal window event processing
    platform->windows.resizing = false;
  }

  return result;
}

LRESULT CALLBACK
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  Win32_PlatformInfo *platform;
  HDC                 hdc;
  PAINTSTRUCT         ps;
  LRESULT             result;
  event               e;
  boolean             processed;
  RECT                rect;
  uint16              width, height;
  TCHAR               *p, szTitle[MAX_LOADSTRING];    // the title bar text
  GLOBALS_ACCESS;

  // default return value
  result    = 0;
  processed = false;

  // access device/platform information block
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // lets ensure we are dealing with the right window (weird events)?
    if (platform->windows.LCDWnd == hWnd)
    {
      switch (message)
      {
        case WM_CREATE:
             processed = true;
             break;

        case WM_GETMINMAXINFO:
             if (ApplicationGetDisplayProperties() != DISPLAY_RESIZE_UNSUPPORTED)
             {
               ((MINMAXINFO *)lParam)->ptMaxSize.x      = (LONG)res_x[RES_COUNT-1];
               ((MINMAXINFO *)lParam)->ptMaxSize.y      = (LONG)res_y[RES_COUNT-1];
               ((MINMAXINFO *)lParam)->ptMinTrackSize.x = (LONG)res_x[0];
               ((MINMAXINFO *)lParam)->ptMinTrackSize.y = (LONG)res_y[0];
               ((MINMAXINFO *)lParam)->ptMaxTrackSize.x = ((MINMAXINFO *)lParam)->ptMaxSize.x;
               ((MINMAXINFO *)lParam)->ptMaxTrackSize.y = ((MINMAXINFO *)lParam)->ptMaxSize.y;

               // take into consideration the window frame/borders (outside of client area)
               ((MINMAXINFO *)lParam)->ptMaxSize.x      += GetSystemMetrics(SM_CXBORDER) + GetSystemMetrics(SM_CXFRAME) + 1;
               ((MINMAXINFO *)lParam)->ptMinTrackSize.x += GetSystemMetrics(SM_CXBORDER) + GetSystemMetrics(SM_CXFRAME) + 1;
               ((MINMAXINFO *)lParam)->ptMaxTrackSize.x += GetSystemMetrics(SM_CXBORDER) + GetSystemMetrics(SM_CXFRAME) + 1;
               ((MINMAXINFO *)lParam)->ptMaxSize.y      += GetSystemMetrics(SM_CYBORDER) + GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CYCAPTION) + 1;
               ((MINMAXINFO *)lParam)->ptMinTrackSize.y += GetSystemMetrics(SM_CYBORDER) + GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CYCAPTION) + 1;
               ((MINMAXINFO *)lParam)->ptMaxTrackSize.y += GetSystemMetrics(SM_CYBORDER) + GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CYCAPTION) + 1;

               if (ApplicationGetDisplayProperties() == DISPLAY_RESIZE_ARBITARY) 
               {
                 ((MINMAXINFO *)lParam)->ptMaxSize.x      += GetSystemMetrics(SM_CXSIZEFRAME);
                 ((MINMAXINFO *)lParam)->ptMinTrackSize.x += GetSystemMetrics(SM_CXSIZEFRAME);
                 ((MINMAXINFO *)lParam)->ptMaxTrackSize.x += GetSystemMetrics(SM_CXSIZEFRAME);
                 ((MINMAXINFO *)lParam)->ptMaxSize.y      += GetSystemMetrics(SM_CYSIZEFRAME);
                 ((MINMAXINFO *)lParam)->ptMinTrackSize.y += GetSystemMetrics(SM_CYSIZEFRAME);
                 ((MINMAXINFO *)lParam)->ptMaxTrackSize.y += GetSystemMetrics(SM_CYSIZEFRAME);
               }
             }
             break;

        case WM_SIZE:
             if (ApplicationGetDisplayProperties() != DISPLAY_RESIZE_UNSUPPORTED)
             {
               // round to 4 pixel boundary
               GetClientRect(hWnd, &rect);
               width  = ((rect.right  - rect.left) + 3) & ~0x03;
               height = ((rect.bottom - rect.top)  + 3) & ~0x03;

               if ((width  != g->device.display.width) ||
                   (height != g->device.display.height))
               {
                 // set the new properties
                 g->device.display.width  = width;
                 g->device.display.height = height;

                 // we need to re-initialize the SHARK LCD driver routines
                 _LCDTerminate();
                 _LCDInitialize();

                 // lets re-initialize the _Gfx window
                 if (g->gfx.window != NULL)
                 {
                   _GfxDeleteWindow(g->gfx.window);
                   g->gfx.window = _GfxCreateWindow(g->device.display.width, g->device.display.height);
                   _GfxSetDrawWindow(g->gfx.window);
                 }

                 // lets update the window to include screen resolution
                 GetWindowText(hWnd, szTitle, MAX_LOADSTRING-1);
                 p = strstr(szTitle, " ::");
                 sprintf((p != NULL) ? p : (szTitle + strlen(szTitle)), " :: (%d %d)", g->device.display.width, g->device.display.height);
                   SetWindowText(hWnd, szTitle);

                 // now, the application must handle the new LCD/Gfx instances
                 _MemSet(&e, sizeof(event), 0);
                 e.eType        = _keyEvent;
                 e.data.key.chr = _key_gfx_change;
                 DAL_ApplicationHandleEvent(&e); 
               }
             }
             processed = true;
             break;

        case WM_KEYDOWN:
             if (_KeyHandleEvent((uint16)(wParam), &e))
               processed = DAL_ApplicationHandleEvent(&e);
             break;
    
        case WM_KEYUP:
             processed = true;
             break;
    
        case WM_SETFOCUS:
        case WM_ACTIVATE:
        case WM_KILLFOCUS:
    
             if ((message == WM_KILLFOCUS) || ((message == WM_ACTIVATE) && (LOWORD(wParam) == WA_INACTIVE)))
             {
               SetFocus(NULL);
               platform->system.active = false;
  
               if (!platform->windows.resizing)
               {
                 // since we have just LOST focus, lets pause!
                 _MemSet(&e, sizeof(event), 0);
                 e.eType        = _keyEvent;
                 e.data.key.chr = _key_focuslost;
                 DAL_ApplicationHandleEvent(&e); // INFORM THE GAME ENGINE RIGHT AWAY
               }

               // lets truncate the window text (so, we can find it again later)
               if (ApplicationGetDisplayProperties() != DISPLAY_RESIZE_UNSUPPORTED)
               {
                 GetWindowText(hWnd, szTitle, MAX_LOADSTRING-1);
                 p = strstr(szTitle, " ::"); 
                 if (p != NULL) *p = 0;
                 SetWindowText(hWnd, szTitle);
               }

               // stop the active audio stream
               _SndStreamStop(0);
             }
             else
             if ((message == WM_SETFOCUS) || ((message == WM_ACTIVATE) && (LOWORD(wParam) == WA_ACTIVE)))
             {
               SetFocus(hWnd);
               platform->system.active = true;

               // lets update the title bar (to reflect the width/height)
               if (ApplicationGetDisplayProperties() != DISPLAY_RESIZE_UNSUPPORTED)
               {
                 GetWindowText(hWnd, szTitle, MAX_LOADSTRING-1);
                 p = strstr(szTitle, " ::");
                 sprintf((p != NULL) ? p : (szTitle + strlen(szTitle)), " :: (%d %d)", g->device.display.width, g->device.display.height);
                   SetWindowText(hWnd, szTitle);

                 // now we have focus back
                 platform->windows.resizing = false;
               }

               // restart the active audio stream
               _SndStreamStart(0);
             }
             processed = true;
             break;
    
        case WM_TIMER:
             if (platform->system.active)
             {
               MSG Msg;
  
               // make sure there no other timer events waiting
               while (PeekMessage(&Msg, platform->windows.LCDWnd, WM_TIMER, WM_TIMER, PM_REMOVE) == TRUE);
  
               _MemSet(&e, sizeof(event), 0);
               e.eType = _nilEvent;
               processed = DAL_ApplicationHandleEvent(&e);
             }
             break;
    
        case WM_PAINT:
             hdc = BeginPaint(hWnd, &ps);
             _LCDRepaint();
             EndPaint(hWnd, &ps);
             processed = true;
             break;
  
        case WM_LBUTTONDOWN:
        case WM_MOUSEMOVE:

             if (_PenHandleEvent(&e))
               processed  = DAL_ApplicationHandleEvent(&e);
             break;
    
        case WM_CLOSE:
             _SysEnqueueQuitEvent();
             processed = true;
             break;
    
        case WM_DESTROY:
             PostQuitMessage(0);
             platform->windows.LCDWnd = NULL;
             processed = true;
             break;
    
        default:
             break;
      }
    }
  }

  if (!processed)
    result = DefWindowProc(hWnd, message, wParam, lParam);

  return result;
}

void
InitApplication()
{
  // load the application preferences
  PlatformLoadPreferences();
}

void
EventLoop()
{
  Win32_PlatformInfo *platform;
  event               _e;
  uint16              eType;
  MSG                 msg;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    _MemSet(&_e, sizeof(event), 0);
    _e.eType = _appStartEvent;
    DAL_ApplicationHandleEvent(&_e);

    // main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
      if (!TranslateAccelerator(msg.hwnd, platform->windows.hAccelTable, &msg))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
  
      // platform independent 'processing' of events
      while (_SysEventAvailable())
      {
        _SysGetEvent(&_e); 
        eType = _e.eType;
        DAL_ApplicationHandleEvent(&_e);

        // application termination condition
        if (eType == _appStopEvent)
          DestroyWindow(platform->windows.LCDWnd);
      }
    }
  }
}

void
EndApplication()
{
  // save the application preferences
  PlatformSavePreferences();
}

/*************************************************************************
 *
 * Internal Handling
 *
 *************************************************************************/

void
LOGInitalize()
{
  Win32_PlatformInfo *platform;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // should we even have this active (ie: no device.cfg = no log)
    if (platform->log.active)
    {
      platform->log.f = fopen(LOG_FILE, "w");
  
      fprintf(platform->log.f, "APPLICATION_LOG_FILE:\n");
      fprintf(platform->log.f, "---------------------\n");
      fprintf(platform->log.f, "\n");
      fprintf(platform->log.f, ":: %s\n", ApplicationGetName());
      fprintf(platform->log.f, "\n");
    }
  }
}

void
LOG_ENTRY(uint8 *str)
{
  Win32_PlatformInfo *platform;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // should we even have this active (ie: no device.cfg = no log)
    if (platform->log.active)
      fprintf(platform->log.f, "%s\n", str);
  }
}

void
LOGTerminate()
{
  Win32_PlatformInfo *platform;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // should we even have this active (ie: no device.cfg = no log)
    if (platform->log.active)
    {
      fprintf(platform->log.f, "\n");
      fprintf(platform->log.f, "- END\n");
      fclose(platform->log.f);
    }
    platform->log.f = NULL;
  }
}

void
DAL_ApplicationResizeWindow(boolean mode)
{
  RECT                rect, clientRect;
  LONG                width, height;
  int16               index, i;
  Win32_PlatformInfo *platform;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    if (ApplicationGetDisplayProperties() != DISPLAY_RESIZE_UNSUPPORTED)
    {
      index = (mode) ? -2 : -1;
      for (i=0; i<RES_COUNT; i++)
      {
        if (g->device.display.width == res_x[i])
        {
          while ((g->device.display.width == res_x[i]) && (i<RES_COUNT))
          {
            if (g->device.display.height == res_y[i]) { index = i; goto SEARCH_ABORT; }
            i++;
          }
        }
      }

SEARCH_ABORT:

      // did we find the resolution in our list?
      if (mode) index++; else index--;

      if ((index >= 0) && (index < RES_COUNT))
      {
        rect.top    = 0;
        rect.left   = 0;
        rect.right  = res_x[index] + GetSystemMetrics(SM_CXFRAME);
        rect.bottom = res_y[index] + GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYMENU);
        if (ApplicationGetDisplayProperties() == DISPLAY_RESIZE_ARBITARY) 
        {
          rect.right  += GetSystemMetrics(SM_CXFRAME);
          rect.bottom += GetSystemMetrics(SM_CYFRAME);
        }

        // now, need to mark if they want the stylus
        g->device.input.stylus = stylus[index];

        // mark as resizing (dont pause game etc)
        platform->windows.resizing = true;

        GetClientRect(GetDesktopWindow(), &clientRect);
        width  = clientRect.right  - clientRect.left;
        height = clientRect.bottom - clientRect.top;
    
        // hide the window
        ShowWindow(platform->windows.LCDWnd, SW_HIDE);

        // center the window on the desktop
        MoveWindow(platform->windows.LCDWnd, 0, 0, (rect.right - rect.left), (rect.bottom - rect.top), TRUE);
        GetClientRect(platform->windows.LCDWnd, &clientRect);
        MoveWindow(platform->windows.LCDWnd, 
                   (width  - clientRect.right)  >> 1, 
                   (height - clientRect.bottom) >> 1, 
                   (rect.right  - rect.left - (clientRect.right  - res_x[index])),
                   (rect.bottom - rect.top  - (clientRect.bottom - res_y[index])), TRUE);

        // make the window visible (and, react to a resize event)
        ShowWindow(platform->windows.LCDWnd, SW_SHOW);
        SendMessage(platform->windows.LCDWnd, WM_SIZE, 0, 0);
      }
    }
  }
}

boolean
DAL_ApplicationHandleEvent(event *e)
{
  boolean processed = false;

  processed = _UIHandleEvent(e);
  if (!processed)
    processed = ApplicationHandleEvent(e);

  return processed;
}

boolean
DAL_ApplicationValidateDRM()
{
  boolean valid;

  // innocent until proven guilty?
  valid = true;

  // something went horribly wrong?
  if (!valid)
    _SysDebugMessage("DRM Validation Failed.", true);

  return valid;
}

/********************************* EOF ***********************************/
