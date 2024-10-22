/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)windows/DAL.c
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 *     Global Variables
 *
 *************************************************************************/

static DAL_GlobalsType gDALGlobals;

#include "DAL-platform.inc"      // platform specific functions

#include "DAL-system.inc"        // system functions
#include "DAL-memory.inc"        // memory functions
#include "DAL-io.inc"            // input/output functions
#include "DAL-string.inc"        // string functions
#include "DAL-gfx.inc"           // graphics functions
#include "DAL-audio.inc"         // audio functions
#include "DAL-file.inc"          // file i/o functions

/*************************************************************************
 *
 *     Local Functions
 *
 *************************************************************************/

void
GlobalsInitialize()
{
  // initialize the modules
  _SysInitialize();
  _MemInitialize();
  _IOInitialize();
  _StrInitialize();
  _GfxInitialize();
  _AudioInitialize();
  _FileInitialize();

  // platform specific initialization (load fonts etc)
  PlatformInitialize();
}

void
GlobalsTerminate()
{
  // platform specific shutdown (release fonts etc)
  PlatformTerminate();

  // shutdown the modules
  _FileTerminate();
  _AudioTerminate();
  _GfxTerminate();
  _StrTerminate();
  _IOTerminate();
  _MemTerminate();
  _SysTerminate();
}

#ifdef _WIN32_WCE
void 
WndDisableHardwareKeys(HWND hWnd)
{
  typedef BOOL (__stdcall *UnregisterFunc1Proc)( UINT, UINT );

  INT i;
  HINSTANCE hCoreDll;
  UnregisterFunc1Proc procUndergisterFunc;

  hCoreDll = LoadLibrary(_T("coredll.dll"));
  procUndergisterFunc = (UnregisterFunc1Proc)GetProcAddress(hCoreDll, _T("UnregisterFunc1"));

  // dpad select key
  procUndergisterFunc(MOD_WIN, 0x86);
  RegisterHotKey(hWnd, 0x86, MOD_WIN, 0x86);

  // application buttons
  for (i = 0xC1; i <= 0xCF; i++)
  {
    procUndergisterFunc(MOD_WIN, i);
    RegisterHotKey(hWnd, i, MOD_WIN, i);
  }

  // phone buttons (yes/no) for O2 based devices
  for (i = 0x72; i <= 0x73; i++)
  {
    procUndergisterFunc(MOD_WIN, i);
    RegisterHotKey(hWnd, i, MOD_WIN, i);
  }

  FreeLibrary(hCoreDll);
}

void 
WndEnableHardwareKeys(HWND hWnd)
{
  INT i;

  // dpad select key
  UnregisterHotKey(hWnd, 0x86);

  // application buttons
  for (i = 0xC1; i <= 0xCF; i++)
    UnregisterHotKey(hWnd, i);

  // phone buttons (yes/no) for O2 based devices
  for (i = 0x72; i <= 0x73; i++)
    UnregisterHotKey(hWnd, i);
}
#endif

/*************************************************************************
 *
 *     Public Functions
 *
 *************************************************************************/

/*************************************************************************
 *
 * FUNCTION:    WndProc
 *
 * DESCRIPTION: Processes messages for the main window.
 *
 * PARAMETERS:  hWnd           - handle window.
 *              message        - message number.
 *              wParam         - word parameter.
 *              lParam         - long parameter.
 *
 * RETURNED:    true if message is processed, false otherwise
 *
 * REVISION HISTORY:
 *
 *************************************************************************/
LRESULT CALLBACK
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  HDC         hdc;
  PAINTSTRUCT ps;
  LRESULT     result = 0;
  boolean     processed = false;

  switch (message)
  {
    case WM_CREATE:

#ifdef _WIN32_SPV                                    // define this in the PROJECT!!!
         // set up soft keys menu
         {
           SHMENUBARINFO mbi;

           memset(&mbi, 0, sizeof(SHMENUBARINFO));
           mbi.cbSize     = sizeof(SHMENUBARINFO);
           mbi.hwndParent = hWnd;
           mbi.nToolBarId = IDR_GENERIC_MENUBAR;
           mbi.hInstRes   = gDALGlobals.hInst;
            
           // quit if we cannot create the menu
           if (!SHCreateMenuBar(&mbi)) _SysEnqueueQuitEvent();
         }
#endif

         processed = true;
         break;

		case WM_COMMAND:
         switch (LOWORD(wParam))
         {
           case IDM_GENERIC_QUIT:
                _SysEnqueueQuitEvent();
                processed = true;
                break;

           default:
                break;
         }
         break;

    case WM_HOTKEY:
#ifdef _WIN32_WCE
        {
          WORD key = HIWORD(lParam);
          if (((key >= 0xC1) && (key <= 0xC4)) || // calender, contacts, mail, swoosh arrow
              ((key >= 0x72) && (key <= 0x73)))   // phone accept/reject on O2
            SendMessage(gDALGlobals.LCDWnd, WM_KEYDOWN, key, 0);
          processed = true;
        }
#endif
        break;

    case WM_KEYDOWN:
        {
          event e;

          if (_KeyHandleEvent((UINT)(wParam), &e))
            processed = ApplicationHandleEvent(&e);
          processed = true;
        }
        break;

    case WM_KEYUP:
        {
          processed = true;
        }
        break;

    case WM_SETFOCUS:
    case WM_ACTIVATE:
    case WM_KILLFOCUS:

         if (hWnd == gDALGlobals.LCDWnd)
         {
           if ((message == WM_KILLFOCUS) || ((message == WM_ACTIVATE) && (LOWORD(wParam) == WA_INACTIVE)))
           {
#ifdef _WIN32_WCE
             WndEnableHardwareKeys(hWnd);
#endif
             SetFocus(NULL);

             // since we have just LOST focus, lets pause!
             {
               event e;
               memset(&e, 0, sizeof(event));
               e.eType        = _keyEvent;
               e.data.key.chr = _key_pauseSpecial;
               ApplicationHandleEvent(&e); // INFORM THE GAME ENGINE RIGHT AWAY
             }

             gDALGlobals.sys.active = false;
           }
           else
           if ((message == WM_SETFOCUS) || ((message == WM_ACTIVATE) && (LOWORD(wParam) == WA_ACTIVE)))
           {
#ifdef _WIN32_WCE
             WndDisableHardwareKeys(hWnd);
             SHFullScreen(hWnd, SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON);
#endif
             SetFocus(hWnd);

             gDALGlobals.sys.active = true;
           }
         }
         processed = true;
      	 break;

    case WM_TIMER:
         if ((gDALGlobals.sys.active) && (hWnd == gDALGlobals.LCDWnd))
         {
           event e;

           e.eType = _nilEvent;
           processed = ApplicationHandleEvent(&e);
         }
         break;

    case WM_PAINT:
         hdc = BeginPaint(hWnd, &ps);
         _GfxRepaint();
         EndPaint(hWnd, &ps);
         processed = true;
         break;

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
         {
          event e;

          if (_PenHandleEvent(&e))
            processed  = ApplicationHandleEvent(&e);
         }
         break;

    case WM_CLOSE:
         _SysEnqueueQuitEvent();
         processed = true;
         break;

    case WM_DESTROY:
         PostQuitMessage(0);
         processed = true;
         break;

    case WM_SETTINGCHANGE:
#ifdef _WIN32_WCE
         SHHandleWMSettingChange(hWnd, wParam, lParam, &(gDALGlobals.s_sai));
#endif
         processed = true;
         break;

    default:
         break;
  }

  if (!processed)
    result = DefWindowProc(hWnd, message, wParam, lParam);

   return result;
}

/*************************************************************************
 *
 * FUNCTION:    MyRegisterClass
 *
 * DESCRIPTION: Registers the window clas.
 *
 * PARAMETERS:  hInstance      - the current instance of the application.
 *              szWindowClass  - window class name.
 *
 * RETURNED:    false if instance of application already exists,
 *              true otherwise
 *
 * REVISION HISTORY:
 *
 *************************************************************************/
ATOM
MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
  WNDCLASS  wc;

  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = (WNDPROC) WndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
#ifdef _WIN32_WCE
  wc.hCursor       = 0;
  wc.lpszMenuName  = 0;
  wc.hIcon         = 0;
#else
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.lpszMenuName  = (LPCSTR)(NULL);
  wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IconWindows));
#endif
  wc.lpszClassName = szWindowClass;

  return RegisterClass(&wc);
}

/*************************************************************************
 *
 * FUNCTION:    InitInstance
 *
 * DESCRIPTION: Saves instance handle and creates main window.
 *
 * PARAMETERS:  hInstance      - the current instance of the application.
 *              nCmdShow       - flags for how the window is to be shown.
 *
 * RETURNED:    false if instance of application already exists,
 *              true otherwise
 *
 * REVISION HISTORY:
 *
 *************************************************************************/
BOOL
InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  HWND      hWnd = NULL;
  TCHAR     szTitle[MAX_LOADSTRING];          // The title bar text
  TCHAR     szWindowClass[MAX_LOADSTRING];    // The window class name
  RECT      rect;

  gDALGlobals.hInst     = hInstance;             // store instance handle in our global variable

  // Initialize global strings
  LoadString(hInstance, ClassID,       szWindowClass, MAX_LOADSTRING);
  LoadString(hInstance, ApplicationID, szTitle,       MAX_LOADSTRING);

#ifdef _WIN32_WCE
  // If it is already running, then focus on the window
  hWnd = FindWindow(szWindowClass, szTitle);
  if (hWnd)
  {
    SHFullScreen(hWnd, SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON);
    SetForegroundWindow((HWND)(((DWORD)hWnd) | 0x01));
    return false;
  }
#endif

  MyRegisterClass(hInstance, szWindowClass);

#ifdef _WIN32_WCE
  SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
  hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
    rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, NULL, NULL, hInstance, NULL);
  GetClientRect(hWnd, &rect);

  // lets determine the size of the screen based on the 'rectangle' we just received
  gDALGlobals.gfx.width  = rect.right  - rect.left;
  gDALGlobals.gfx.height = rect.bottom - rect.top;
  gDALGlobals.gfx.depth  = GFX_WINDOW_BPP;

  // fix these stupid key button mappings
  WndDisableHardwareKeys(hWnd);
#else
  GetClientRect(hWnd, &rect);
  rect.left   = rect.top = 0;
  rect.right  = (int16)(gDALGlobals.gfx.width  * gDALGlobals.gfx.stretch_x) - (GetSystemMetrics(SM_CXBORDER) * 2);
  rect.bottom = (int16)(gDALGlobals.gfx.height * gDALGlobals.gfx.stretch_y) - GetSystemMetrics(SM_CYCAPTION) - (GetSystemMetrics(SM_CYBORDER) * 2);
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, TRUE);
  hWnd = CreateWindow(szWindowClass, szTitle, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
    NULL, NULL, hInstance, NULL);
#endif

  if (!hWnd)
    return FALSE;

  // initialize globals
  gDALGlobals.LCDWnd    = hWnd;
  gDALGlobals.LCDScreen = GetDC(hWnd);

#ifdef _WIN32_WCE
  SHFullScreen(hWnd, SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON);
#endif

  UpdateWindow(hWnd);
  ShowWindow(hWnd, nCmdShow);

  SetFocus(hWnd);
  gDALGlobals.sys.active = true;

  return true;
}

/*************************************************************************
 *
 * FUNCTION:    InitApplication
 *
 * DESCRIPTION: Perform initialization routines, load preferences etc.
 *
 * PARAMETERS:  hInstance      - the current instance of the application.
 *              nCmdShow       - flags for how the window is to be shown.
 *
 * RETURNED:    Initialization state
 *
 * REVISION HISTORY:
 *
 *************************************************************************/
BOOL
InitApplication(HINSTANCE hInstance, int nCmdShow)
{
  // Perform application initialization:
  if (!InitInstance(hInstance, nCmdShow))
    return FALSE;

#ifndef _WIN32_WCE
  srand((unsigned)time(NULL));
#endif

  gDALGlobals.hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)ClassID);

  return TRUE;
}

/*************************************************************************
 *
 * FUNCTION:    WinMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  hInstance      - the current instance of the application.
 *              hPrevInstance  - the previous instance of the application.
 *              lpCmdLine      - the command line for the application.
 *              nCmdShow       - flags for how the window is to be shown.
 *
 * RETURNED:    zero if launch successful, error code otherwise
 *
 * REVISION HISTORY:
 *
 *************************************************************************/
int
WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
  int      ret = FALSE;
#ifndef _WIN32_WCE
  FILE    *f;
  char     str[32];
  boolean  normal;
#endif

// #define CARD_LOCKING
#ifdef CARD_LOCKING
  //
  // MDM CARD PROTECTION: find /LICENSE.txt file, check card >> SIZE + TYPE <<
  //

#ifdef _WIN32_WCE
  {
    boolean  valid = false;
    HANDLE           hFlashCard;
    WIN32_FIND_DATA  hFlashCardInfo;
    TCHAR           *cFlashCardPath;
    boolean          search;
    uint8            path[MAX_PATH], i;
    uint8            fileName[256];
    FILE            *f;

    // find first card, no handle? break
    hFlashCard = FindFirstFlashCard(&hFlashCardInfo);
    search = (hFlashCard != INVALID_HANDLE_VALUE);

    while (search)
    {
      cFlashCardPath = hFlashCardInfo.cFileName;

      // convert from TCHAR --> uint8 buffer
      i = 0;
      while (cFlashCardPath[i] != 0) { path[i] = (uint8)cFlashCardPath[i]; i++; }
      path[i] = 0;

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

        GetDiskFreeSpaceEx(pathW, NULL, &cardSize, NULL);
        
        // verification check here!
        valid |= (cardSize.LowPart < 9437184); // 9Mb

        break;
      }
      search = FindNextFlashCard(hFlashCard, &hFlashCardInfo);
    }

    // BARF!!
    if (!valid) return false;
  }
#endif
#endif

  memset(&gDALGlobals, 0, sizeof(DAL_GlobalsType));

  // SPECIAL: win32 building (debugging)
#ifndef _WIN32_WCE
  // determine resolution we should be using
  normal = true;
  f = fopen("resolution.cfg", "r");
  if (f != NULL)
  {
    // try to read "width height" from file, if fail, default to 350x350
    fgets(str, 32, f);
    normal = (sscanf(str, "%d%d", &gDALGlobals.gfx.width, &gDALGlobals.gfx.height) != 2);
    fclose(f);

    // round to 4 pixel boundary
    gDALGlobals.gfx.width = (gDALGlobals.gfx.width + 3) & ~0x03;
  }

  // normal resolution (nice square 350x350)
  if (normal)
  {
    gDALGlobals.gfx.width  = 350;
    gDALGlobals.gfx.height = 350;
  }
  gDALGlobals.gfx.depth  = GFX_WINDOW_BPP;

  // determine if we should stretch on any axis
  normal = true;
  f = fopen("stretch.cfg", "r");
  if (f != NULL)
  {
    // try to read "stretch_x stretch_y" from file, if fail, default to 1:1
    fgets(str, 32, f);
    normal = (sscanf(str, "%f%f", &gDALGlobals.gfx.stretch_x, &gDALGlobals.gfx.stretch_y) != 2);
    fclose(f);
  }

  // normal resolution (based on PPC 240x320)
  if (normal)
  {
    gDALGlobals.gfx.stretch_x = 1;
    gDALGlobals.gfx.stretch_y = 1;
  }
#endif

  if (InitApplication(hInstance, nCmdShow))
  {
    GlobalsInitialize();
           
    if (ApplicationInitialize()) 
    {
      UINT   prefSize;
      void  *prefs;
      FILE  *fpPrefs;
      uint8  fileName[64];

      // read the saved preferences/saved state information
      prefSize = ApplicationGetPreferencesSize();
      if (prefSize != 0)
      {
        prefs    = (void *)ApplicationGetPreferences();
        sprintf(fileName, "%s-%s", ApplicationGetName(), DEFAULT_PREF_FILENAME);
        fpPrefs = fopen(fileName, "rb");
        if (fpPrefs == NULL)
          memset(prefs, 0, prefSize);
        else
        {
          fread(prefs, prefSize, 1, fpPrefs);
          fclose(fpPrefs);
        }
      }

      // everything seemed to initialize correctly, go!
      EventLoop();
      EndApplication();

      // write the saved preferences/saved state information
      prefSize = ApplicationGetPreferencesSize();
      if (prefSize != 0)
      {
        prefs   = ApplicationGetPreferences();
        sprintf(fileName, "%s-%s", ApplicationGetName(), DEFAULT_PREF_FILENAME);
        fpPrefs = fopen(fileName, "wb");
        if (fpPrefs != NULL)
        {
          fwrite(prefs, prefSize, 1, fpPrefs);
          fclose(fpPrefs);
        }
      }
    }
    ApplicationTerminate();
             
    GlobalsTerminate();
  }

  return ret;
}

/*************************************************************************
 *
 * FUNCTION:    EventLoop
 *
 * DESCRIPTION: The event loop for the application.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    none
 *
 * REVISION HISTORY:
 *
 *************************************************************************/
int
EventLoop()
{
  event _e;
  MSG   msg;

  memset(&_e, 0, sizeof(event));
  _e.eType = _appStartEvent;
  ApplicationHandleEvent(&_e);

  // main message loop:
  while (GetMessage(&msg, NULL, 0, 0))
  {
    if (!TranslateAccelerator(msg.hwnd, gDALGlobals.hAccelTable, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    // platform independent 'processing' of events
    while (_SysEventAvailable())
    {
      _SysGetEvent(&_e);
      ApplicationHandleEvent(&_e);

      // application termination condition
      if (_e.eType == _appStopEvent)
      {
#ifdef _WIN32_WCE
        SendMessage(gDALGlobals.LCDWnd, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)gDALGlobals.LCDWnd);
        SendMessage(gDALGlobals.LCDWnd, WM_DESTROY, 0, 0);
#else
        DestroyWindow(gDALGlobals.LCDWnd);
#endif
      }
    }
  }

  return msg.wParam;
}

/*************************************************************************
 *
 * FUNCTION:    EndApplication
 *
 * DESCRIPTION: Perform termination routines, save preferences etc.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    none
 *
 * REVISION HISTORY:
 *
 *************************************************************************/
void
EndApplication()
{
#ifdef _WIN32_WCE
  WndEnableHardwareKeys(gDALGlobals.LCDWnd);
#endif

  KillTimer(gDALGlobals.LCDWnd, gDALGlobals.timerID);
  SetFocus(NULL);
}

/********************************* EOF ***********************************/
