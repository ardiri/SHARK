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

boolean
DAL_Initialize()
{
  boolean result;
  GLOBALS_ACCESS;

  result = false;

  // allocate the information block
  g->platform        = (void *)SYSTEM_MemPtrNew(sizeof(MSmob_PlatformInfo));
  g->device.platform = (void *)NULL;
  if (g->platform != NULL)
  {
    MSmob_PlatformInfo *platform = (MSmob_PlatformInfo *)g->platform;
    _MemSet(platform, sizeof(MSmob_PlatformInfo), 0);

    // load configuration options
    DAL_Configuration();
    result = true;

#ifdef GAPI_SUPPORT
/**
 ** /Program Files/Windows CE Tools/wce300/Pocket PC 2002/target/arm
 ** $ dumpbin /exports gx.dll
 **
 ** ?GXBeginDraw@@YAPAXXZ
 ** ?GXCloseDisplay@@YAHXZ
 ** ?GXCloseInput@@YAHXZ
 ** ?GXEndDraw@@YAHXZ
 ** ?GXGetDefaultKeys@@YA?AUGXKeyList@@H@Z
 ** ?GXGetDisplayProperties@@YA?AUGXDisplayProperties@@XZ
 ** ?GXIsDisplayDRAMBuffer@@YAHXZ
 ** ?GXOpenDisplay@@YAHPAUHWND__@@K@Z
 ** ?GXOpenInput@@YAHXZ
 ** ?GXResume@@YAHXZ
 ** ?GXSetViewport@@YAHKKKK@Z
 ** ?GXSuspend@@YAHXZ
 **
 **/
    // initialize the GAPI library callbacks
    platform->gapi.libRef = LoadLibrary(_T("gx.dll"));
    if (platform->gapi.libRef != NULL)
    {
      platform->gapi.GXOpenDisplay  = (void *)GetProcAddress(platform->gapi.libRef,_T("?GXOpenDisplay@@YAHPAUHWND__@@K@Z"));
      platform->gapi.GXCloseDisplay = (void *)GetProcAddress(platform->gapi.libRef,_T("?GXCloseDisplay@@YAHXZ"));
      platform->gapi.GXBeginDraw    = (void *)GetProcAddress(platform->gapi.libRef,_T("?GXBeginDraw@@YAPAXXZ"));
      platform->gapi.GXSuspend      = (void *)GetProcAddress(platform->gapi.libRef,_T("?GXSuspend@@YAHXZ"));
      platform->gapi.GXResume       = (void *)GetProcAddress(platform->gapi.libRef,_T("?GXResume@@YAHXZ"));
      platform->gapi.GXEndDraw      = (void *)GetProcAddress(platform->gapi.libRef,_T("?GXEndDraw@@YAHXZ"));
    }
    else
    {
      platform->gapi.GXOpenDisplay  = NULL;
      platform->gapi.GXCloseDisplay = NULL;
      platform->gapi.GXBeginDraw    = NULL;
      platform->gapi.GXSuspend      = NULL;
      platform->gapi.GXResume       = NULL;
      platform->gapi.GXEndDraw      = NULL;
    }
#endif
  }

  return result;
}

void
DAL_Configuration()
{ 
  int  width, height;
  GLOBALS_ACCESS;

  // lets get the system parameter information
  width  = GetSystemMetrics(SM_CXSCREEN); 
  height = GetSystemMetrics(SM_CYSCREEN);

  g->device.display.width  = width;
  g->device.display.height = height;
  g->device.display.depth  = 16;
  g->device.input.stylus   = (width > 176);   // special hack: > 176 = stylus
  g->device.input.analog   = false;
}

void
DAL_Terminate()
{
  GLOBALS_ACCESS;

#ifdef GAPI_SUPPORT
  if (g->platform != NULL)
  {
    MSmob_PlatformInfo *platform = (MSmob_PlatformInfo *)g->platform;

    // release the GAPI library
    FreeLibrary(platform->gapi.libRef);
    platform->gapi.GXOpenDisplay  = NULL;
    platform->gapi.GXCloseDisplay = NULL;
    platform->gapi.GXBeginDraw    = NULL;
    platform->gapi.GXSuspend      = NULL;
    platform->gapi.GXResume       = NULL;
    platform->gapi.GXEndDraw      = NULL;
  }
#endif

  // release information blocks
  if (g->platform != NULL) SYSTEM_MemPtrFree(g->platform); 
  g->platform = NULL;
}

boolean
GlobalsInitialize()
{
  boolean result;

  // initialize all sub-components
  result = true;
  result &= _MemInitialize();  // REQ: must be done first
  result &= _LCDInitialize();  // REQ: must be done second
  result &= _FileInitialize();
  result &= _InpInitialize();
  result &= _SndInitialize();
  result &= _SysInitialize();
  result &= PlatformInitialize();

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

  PlatformTerminate();
  _SysTerminate();
  _SndTerminate();
  _InpTerminate();
  _FileTerminate();
  _LCDTerminate();             // REQ: must be done second last

  _MemTerminate();             // REQ: must be done last
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
          if (ApplicationInitialize()) 
          {
            InitApplication();
            EventLoop();
            EndApplication();
          }
          ApplicationTerminate();
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
  MSmob_PlatformInfo *platform;
  HWND                hWnd;
  TCHAR                     szTitle[MAX_LOADSTRING];    // the title bar text
  TCHAR               szWindowClass[MAX_LOADSTRING];    // the window class name
  RECT                rect;
  WNDCLASS            wc;
  boolean             result;
  GLOBALS_ACCESS;

  result = false;

  // access device/platform information block
  platform = (MSmob_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // initialize global strings
    LoadString(hInstance, IDC_DEFAULT_CLASS_ID,    szWindowClass, MAX_LOADSTRING);
    LoadString(hInstance, IDS_DEFAULT_APP_NAME_ID, szTitle,       MAX_LOADSTRING);
  
    // if it is already running, then focus on the window
    hWnd = FindWindow(szWindowClass, szTitle);
    if (hWnd != NULL)
    {
      SHFullScreen(hWnd, SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON);
      SetForegroundWindow((HWND)(((DWORD)hWnd) | 0x01));
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
    wc.lpszClassName = szWindowClass;
    RegisterClass(&wc);
  
    // create the window
    rect.left   = 0;
    rect.top    = rect.left = 0;
    rect.right  = GetDeviceCaps(NULL, HORZRES);
    rect.bottom = GetDeviceCaps(NULL, VERTRES);
    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
                        rect.left, rect.top, 
                        (rect.right - rect.left), (rect.bottom - rect.top), 
                        NULL, NULL, hInstance, NULL);
  
    // window created successfully?
    if (hWnd != NULL)
    {  
      DAL_InpDisableHardwareKeys(hWnd);
      SHFullScreen(hWnd, SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON);

      // display window and set focus
      UpdateWindow(hWnd);
      ShowWindow(hWnd, nCmdShow);
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
  }

  return result;
}

LRESULT CALLBACK
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  MSmob_PlatformInfo *platform;
  HDC                 hdc;
  PAINTSTRUCT         ps;
  LRESULT             result;
  event               e;
  RECT                rect;
  uint16              width, height;
  boolean             processed;
  WORD                key;
  GLOBALS_ACCESS;

  // default return value
  result    = 0;
  processed = false;

  // access device/platform information block
  platform = (MSmob_PlatformInfo *)g->platform;
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

                 // now, the application must handle the new LCD/Gfx instances
                 _MemSet(&e, sizeof(event), 0);
                 e.eType        = _keyEvent;
                 e.data.key.chr = _key_gfx_change;
                 DAL_ApplicationHandleEvent(&e); 
               }
             }
             processed = true;
             break;

        case WM_HOTKEY:
             key = HIWORD(lParam);
             SendMessage(hWnd, WM_KEYDOWN, key, 0);
             processed = true;
             break;

#ifdef INP_HANDLE_KEY_UP_DN_MASK
        case WM_KEYDOWN:
             key = (uint16)(wParam);

             if (key == VK_APP1)  platform->input.appKeyState |= KEY_STATE_APP1;
             if (key == VK_APP2)  platform->input.appKeyState |= KEY_STATE_APP2;
             if (key == VK_APP3)  platform->input.appKeyState |= KEY_STATE_APP3;
             if (key == VK_APP4)  platform->input.appKeyState |= KEY_STATE_APP4;
             if (key == VK_APP5)  platform->input.appKeyState |= KEY_STATE_APP5;
             if (key == VK_APP6)  platform->input.appKeyState |= KEY_STATE_APP6;
             if (key == VK_TTALK) platform->input.appKeyState |= KEY_STATE_TTALK;
             if (key == VK_TEND)  platform->input.appKeyState |= KEY_STATE_TEND;

             processed = true;
             break;
#endif
    
        case WM_KEYUP:
#ifdef INP_HANDLE_KEY_UP_DN_MASK
            key = (uint16)(wParam);

             if (key == VK_APP1)  platform->input.appKeyState &= ~KEY_STATE_APP1;
             if (key == VK_APP2)  platform->input.appKeyState &= ~KEY_STATE_APP2;
             if (key == VK_APP3)  platform->input.appKeyState &= ~KEY_STATE_APP3;
             if (key == VK_APP4)  platform->input.appKeyState &= ~KEY_STATE_APP4;
             if (key == VK_APP5)  platform->input.appKeyState &= ~KEY_STATE_APP5;
             if (key == VK_APP6)  platform->input.appKeyState &= ~KEY_STATE_APP6;
             if (key == VK_TTALK) platform->input.appKeyState &= ~KEY_STATE_TTALK;
             if (key == VK_TEND)  platform->input.appKeyState &= ~KEY_STATE_TEND;
#endif

             if (_KeyHandleEvent((uint16)(wParam), &e))
               processed = DAL_ApplicationHandleEvent(&e);
             break;
    
        case WM_SETFOCUS:
        case WM_ACTIVATE:
        case WM_KILLFOCUS:
    
             if ((message == WM_KILLFOCUS) || ((message == WM_ACTIVATE) && (LOWORD(wParam) == WA_INACTIVE)))
             {
               DAL_InpEnableHardwareKeys(hWnd);
               SetFocus(NULL);
  
#ifdef GAPI_SUPPORT
               // suspend GAPI
               if (platform->gapi.GXSuspend)
                 platform->gapi.GXSuspend();
#endif

               // since we have just LOST focus, lets pause!
               _MemSet(&e, sizeof(event), 0);
               e.eType        = _keyEvent;
               e.data.key.chr = _key_focuslost;
               DAL_ApplicationHandleEvent(&e); // INFORM THE GAME ENGINE RIGHT AWAY
               platform->system.active = false;

               // stop the active audio stream
               _SndStreamStop(0);
             }
             else
             if ((message == WM_SETFOCUS) || ((message == WM_ACTIVATE) && (LOWORD(wParam) == WA_ACTIVE)))
             {
               DAL_InpDisableHardwareKeys(hWnd);
               SHFullScreen(hWnd, SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON);
               SetFocus(hWnd);
               platform->system.active = true;

#ifdef GAPI_SUPPORT
               // resume GAPI
               if (platform->gapi.GXResume)
                 platform->gapi.GXResume();
#endif

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
  
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
             if (_PenHandleEvent(&e))
               processed = DAL_ApplicationHandleEvent(&e);
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
  
        case WM_SETTINGCHANGE:
             SHHandleWMSettingChange(hWnd, wParam, lParam, &(platform->windows.s_sai));
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
  MSmob_PlatformInfo *platform;
  event               _e;
  uint16              eType;
  MSG                 msg;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (MSmob_PlatformInfo *)g->platform;
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

  // CARD PROTECTION: 
  if (PlatformResourceExists(9999))
  {
    HANDLE           hFlashCard;
    WIN32_FIND_DATA  hFlashCardInfo;
    TCHAR           *cFlashCardPath;
    boolean          search;
    uint8            path[MAX_PATH], i;
    uint8            fileName[256];
    FILE            *f;
    uint8           *info;

    // lets lock down the DRM resource
    info = (uint8 *)PlatformGetResource(9999);

    // find first card, no handle? break
    hFlashCard = FindFirstFlashCard(&hFlashCardInfo);
    search = (hFlashCard != INVALID_HANDLE_VALUE);

    // we need to assume they have not met the requirements
    valid = false;

    // look
    while (search)
    {
      cFlashCardPath = hFlashCardInfo.cFileName;

      // convert from TCHAR --> uint8 buffer
      i = 0;
      while (cFlashCardPath[i] != 0) { path[i] = (uint8)cFlashCardPath[i]; i++; }
      path[i] = 0;

      // find /LICENSE.txt file
      sprintf((char *)fileName, "/%s/LICENSE.txt", path);
      f = fopen((char *)fileName, "rb");
      if (f != NULL) 
      {
        ULARGE_INTEGER cardSize;
        TCHAR pathW[64];
        char  pathC[64];

        // generate the path name
        sprintf(pathC, "/%s/", path);
        MultiByteToWideChar(CP_ACP,0,pathC,-1,pathW,64);
  
        // check card 
        GetDiskFreeSpaceEx(pathW, NULL, &cardSize, NULL);
        
        // verification check here!
        valid |= (cardSize.LowPart < (uint32)((*info+1) * 1048576L));

        break;
      }
      search = FindNextFlashCard(hFlashCard, &hFlashCardInfo);
    }

    // release the DRM resource
    PlatformReleaseResource(info);
  }

  // something went horribly wrong?
  if (!valid)
    _SysDebugMessage("DRM Validation Failed.", true);

  return valid;
}

/********************************* EOF ***********************************/
