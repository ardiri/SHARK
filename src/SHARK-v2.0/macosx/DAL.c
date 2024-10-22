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
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

// SHARK includes
#include "../SHARK.h"
#include "../SHARK-prv.h"

#include <Carbon/Carbon.h>

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

static WindowRef          g_window;
static Rect               g_windowRect;

static boolean            g_active;
static boolean            g_initialized;

#if (LOG_DISABLE == 0)
static FILE              *g_logfile;
#endif

// globally shared information
#define LCD_COLOR_BITS    5 // 8

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

static void    DAL_AppFocusGained();
static void    DAL_AppFocusLost();

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
  chdir(g_path);
  strcpy(fileName, "application-runtime.txt");

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

static void
DAL_AppFocusGained()
{
  char title[128];

  // application focus gained
  g_active = true;

  // resume LCD operations
  _LCDResume();

  // lets update the window to include screen resolution
  sprintf(title, "%s :: (%d %d)", g_config.name, g_width, g_height);
  SetWindowTitleWithCFString(g_window, 
    CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8));

  // restart the active audio stream
  _SndStreamStart(0);

  // need to repaint the display
  _LCDRepaint();
}

static void
DAL_AppFocusLost()
{
  event e;
  char  title[128];

  // application focus lost
  g_active = false;

  // suspend LCD operations
  _LCDSuspend();

  // lets truncate the window text
  sprintf(title, "%s", g_config.name);
  SetWindowTitleWithCFString(g_window, 
    CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8));

  // since we have just LOST focus, lets pause!
  memset(&e, 0, sizeof(event));
  e.eType        = _keyEvent;
  e.data.key.chr = _key_focuslost;
  DAL_HandleEvent(&e); // INFORM THE GAME ENGINE RIGHT AWAY

  // stop the active audio stream
  _SndStreamStop(0);
}

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
  uint16 eType;
  event  _e;

  // platform independent 'processing' of events
  while (_SysEventAvailable() && (g_window != NULL))
  {
    _SysGetEvent(&_e);
    eType = _e.eType;
    DAL_HandleEvent(&_e);

    // application termination condition
    if (eType == _appStopEvent)
    {
      g_window = NULL;
      QuitApplicationEventLoop();
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

static OSStatus
MainFormHandleEvent(EventHandlerCallRef inHandlerCallRef,
                    EventRef inEvent, void *inUserData)
{
  UInt32    eventClass;
  UInt32    eventKind;
  Point     eventLocation;
  UInt32    keyCode;
  UInt32    keyChrSize;
  uint8     keyChr[4];
  OSStatus  result;
  event     e;
  char      title[128];

  eventClass = GetEventClass(inEvent);
  eventKind  = GetEventKind(inEvent);

  // not handled by default
  result = eventNotHandledErr;

  switch (eventClass)
  {
    case kEventClassApplication:
         if (eventKind == kEventAppQuit)
         {
         }
         else
         if (eventKind == kEventAppDeactivated) 
         {
           DAL_AppFocusLost();
         }
         else
         if (eventKind == kEventAppActivated)
         {
           DAL_AppFocusGained();
         }
         break;

    case kEventClassWindow:
         if (eventKind == kEventWindowFocusAcquired)
         {
           DAL_AppFocusGained();
         }
         else
         if (eventKind == kEventWindowFocusRelinquish) 
         {
           DAL_AppFocusLost();
         }
         else
         if (eventKind == kEventWindowGetMinimumSize)
         {
           Point size = { 160, 160 };  // vertical, horizontal
           SetEventParameter(inEvent, kEventParamDimensions,
             typeQDPoint, sizeof(Point), &size);
           result = noErr;  // we have handled this event now
         }
         if (eventKind == kEventWindowGetMaximumSize)
         {
           Point size = { 768, 1024 };  // vertical, horizontal
           SetEventParameter(inEvent, kEventParamDimensions,
             typeQDPoint, sizeof(Point), &size);
           result = noErr;  // we have handled this event now
         }
         else
         if (eventKind == kEventWindowBoundsChanged)
         {
           UInt32 attributes;

           GetEventParameter(inEvent, kEventParamAttributes,
             typeUInt32, NULL, sizeof(UInt32), NULL, &attributes);
           if (attributes & kWindowBoundsChangeSizeChanged)
           {
             Rect windowBounds;

             GetEventParameter(inEvent, kEventParamCurrentBounds,
               typeQDRectangle, NULL, sizeof(Rect), NULL, &windowBounds);

             // lets check the new bounds of the window
             g_width     = windowBounds.right  - windowBounds.left;
             g_width_rb  = (uint16)(((g_width  + 3) & ~3) * (LCD_DEPTH >> 3));
             g_height    = windowBounds.bottom - windowBounds.top;
             g_height_rb = (uint16)(((g_height + 3) & ~3) * (LCD_DEPTH >> 3));

             // adjust to the local window co-ordinates
             g_windowRect.left   = 0;
             g_windowRect.top    = 0;
             g_windowRect.right  = g_windowRect.left + g_width;
             g_windowRect.bottom = g_windowRect.top  + g_height;

             // re-generate the offscreen windows
             _LCDTerminate_RESIZE();
             _LCDInitialize_RESIZE();

             // lets update the window to include screen resolution
             sprintf(title, "%s :: (%d %d)", g_config.name, g_width, g_height);
             SetWindowTitleWithCFString(g_window, 
             CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8));

             // now, the application must handle the new LCD instance
             memset(&e, 0, sizeof(event));
             e.eType        = _keyEvent;
             e.data.key.chr = _key_gfx_change;
             DAL_HandleEvent(&e);

             // need to repaint the display
             _LCDRepaint();
           }
         }
         else
         if (eventKind == kEventWindowClickCloseRgn)
         {
           _SysEnqueueQuitEvent();
           QuitApplicationEventLoop();
         }
         break;

    case kEventClassMenu:
         if (eventKind == kEventMenuOpening)
         {
           DAL_AppFocusLost();
         }
         break;

    case kEventClassMouse:

         // if window isn't active; make it active (return from menu?)
         if (!g_active) { DAL_AppFocusGained(); break; }

         if ((eventKind == kEventMouseDown) ||
             (eventKind == kEventMouseDragged))
         {
           Rect  windowPosition;
           short penX, penY;
        
           GetWindowBounds(g_window, kWindowContentRgn, &windowPosition);
           GetEventParameter(inEvent, kEventParamMouseLocation, 
             typeQDPoint, NULL, sizeof(Point), NULL, &eventLocation);
           penX = (short)(eventLocation.h - windowPosition.left);
           penY = (short)(eventLocation.v - windowPosition.top);

           // is the pen within the drawable window area?
           if ((penX >= 0) && (penX <= g_width) && 
               (penY >= 0) && (penY <= g_height))
           {
             if (_PenHandleEvent(penX, penY, &e))
               DAL_HandleEvent(&e);

             result = noErr;  // we have handled this event now (within window)
             if ((penX > (g_width-16)) && (penY > (g_height-16))) 
               result = eventNotHandledErr; // allow re-sizing
           }
         }
         break;

    case kEventClassKeyboard:
         if (eventKind == kEventRawKeyDown)
         {
           GetEventParameter(inEvent, kEventParamKeyCode, typeUInt32, 
                             NULL, sizeof(keyCode), NULL, &keyCode);
           if (_KeyHandleEvent((uint16)(keyCode), &e))
           {
             DAL_HandleEvent(&e);
             goto KEY_PROCSSING_DONE;
           }

           // normal keypress, we want the UTF8 encoding for export
           GetEventParameter(inEvent, kEventParamKeyUnicodes, typeUTF8Text, 
                             NULL, 0, &keyChrSize, NULL);
           GetEventParameter(inEvent, kEventParamKeyUnicodes, typeUTF8Text, 
                             NULL, keyChrSize, 0, keyChr);

           // visible characters only please :)
           if (keyChr[0] >= 32)
           {
             _MemSet(&e, 0, sizeof(event));
             e.eType = _keyEvent;
             _MemMove(&e.data.key.chr, keyChr, keyChrSize);
             e.data.key.size = (uint8)keyChrSize;
             DAL_HandleEvent(&e);
           }
/**
           // ASCII
           GetEventParameter(inEvent, kEventParamKeyMacCharCodes, typeChar, 
                             NULL, sizeof(keyChr), NULL, &keyChr);
           if (keyChr >= 32)
           {
             e.eType        = _keyEvent;
             e.data.key.chr = (uint8)keyChr;
             DAL_HandleEvent(&e);
           }
 **/

KEY_PROCSSING_DONE:

           result = noErr;  // we have handled this event now
         }
         break;

    default:
         break;
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

  // basic initialization (and, loading screen)
  memset(&_e, 0, sizeof(event));
  _e.eType = _appStartEvent;
  DAL_HandleEvent(&_e);

  // application is ready to startup - can do stuff like loading here
  memset(&_e, 0, sizeof(event));
  _e.eType = _appGoEvent;
  DAL_HandleEvent(&_e);

  // main message loop:
  RunApplicationEventLoop();

  // platform independent 'processing' of events
  DAL_DispatchEvents();
}

static void
EndApplication()
{
  // save the application preferences
  _PreferencesSave();
}

static boolean
CreateApplicationWindow()
{
  boolean          init_ok;
  WindowClass      windowClass;
  WindowAttributes windowAttributes;
  OSStatus         err;
  char             title[128];
  EventTypeSpec    windowEvents[] =
   {
     { kEventClassWindow,      kEventWindowFocusAcquired   }, // focus gain
     { kEventClassWindow,      kEventWindowFocusRelinquish }, // focus lost
     { kEventClassWindow,      kEventWindowClickCloseRgn   }, // close window
     { kEventClassWindow,      kEventWindowBoundsChanged   }, // resizing window
     { kEventClassWindow,      kEventWindowGetMinimumSize  }, // min size
     { kEventClassWindow,      kEventWindowGetMaximumSize  }, // max size
     { kEventClassMouse,       kEventMouseDown             }, // mouse click
     { kEventClassMouse,       kEventMouseDragged          }, // mouse drag
     { kEventClassKeyboard,    kEventRawKeyDown            }, // keyboard events
     { kEventClassMenu,        kEventMenuOpening           }  // menu opened
   };
  EventTypeSpec    applicationEvents[] =
   {
     { kEventClassApplication, kEventAppQuit               }, // app quit
     { kEventClassApplication, kEventAppDeactivated        }, // app suspended
     { kEventClassApplication, kEventAppActivated          }  // app restore
   };

  // lets assume everything is ok
  init_ok = true;

  // create the window
  windowClass      = kDocumentWindowClass;
  windowAttributes =
    kWindowCloseBoxAttribute    |    // close button
    kWindowFullZoomAttribute    |    // can be maximized
    kWindowCollapseBoxAttribute |    // can be minimized
    kWindowResizableAttribute   |    // can be resized
    kWindowLiveResizeAttribute  |    // ... in real time
    kWindowStandardHandlerAttribute; // standard handlers (moving window etc)
  g_windowRect.left   = 0;
  g_windowRect.top    = 0;
  g_windowRect.right  = g_windowRect.left + g_width;
  g_windowRect.bottom = g_windowRect.top  + g_height;

  // initialize the window
  err = CreateNewWindow(windowClass, windowAttributes, 
                        &g_windowRect, &g_window);
  if (err != noErr) { init_ok = false; goto CREATE_WINDOW_DONE; }

  SetPortWindowPort(g_window);
  sprintf(title, "%s :: (%d %d)", g_config.name, g_width, g_height);
  SetWindowTitleWithCFString(g_window, 
    CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8));
  RepositionWindow(g_window, NULL, kWindowCenterOnMainScreen);

  // install event handlers
  InstallApplicationEventHandler(NewEventHandlerUPP(MainFormHandleEvent),
    GetEventTypeCount(applicationEvents), applicationEvents, NULL, NULL);
  InstallWindowEventHandler(g_window, NewEventHandlerUPP(MainFormHandleEvent),
    GetEventTypeCount(windowEvents), windowEvents, NULL, NULL);

CREATE_WINDOW_DONE:

  return init_ok;
}

// application main-line interface
int 
main(int argc, char* argv[])
{
  CFURLRef         bundleRef;
  CFStringRef      bundlePath;

  // obtain the application configuration parameters
  memset(&g_config, 0, sizeof(application_config));
  ApplicationConfig(&g_config);

  // we need to keep track of the base path of this application
  bundleRef  = CFBundleCopyBundleURL(CFBundleGetMainBundle());
  bundlePath = CFURLCopyFileSystemPath(bundleRef, kCFURLPOSIXPathStyle);
  strcpy(g_path,
         CFStringGetCStringPtr(bundlePath, CFStringGetSystemEncoding()));
  CFRelease(bundleRef);
  CFRelease(bundlePath);

// LCD SETTINGS:
// macosx - can only use the 555 (16bit) display mode

  // this is our "setting" for our LCD engine (hard coded)
  g_lcd_a = 0;
  g_lcd_r = LCD_COLOR_BITS;
  g_lcd_g = LCD_COLOR_BITS;
  g_lcd_b = LCD_COLOR_BITS;  
  g_lcd_r_mask = ((0xff >> (8 - g_lcd_r)) << (g_lcd_g + g_lcd_b));
  g_lcd_g_mask = ((0xff >> (8 - g_lcd_g)) << (g_lcd_b));
  g_lcd_b_mask =  (0xff >> (8 - g_lcd_b));

#if (LOG_DISABLE == 0)
  // initialize the log file function
  _LOGInitialize();
#endif

  // standard default window size
  g_width     = 240;
  g_width_rb  = (uint16)(((g_width  + 3) & ~3) * (LCD_DEPTH >> 3));
  g_height    = 320;
  g_height_rb = (uint16)(((g_height + 3) & ~3) * (LCD_DEPTH >> 3));

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry("\n:: - hardware abstraction initialize -\n");
#endif

  // application startup
  g_initialized = false;
  if (CreateApplicationWindow())
  {
    if (GlobalsInitialize())
    {
      // lets go!
      ShowWindow(g_window);

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
