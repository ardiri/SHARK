/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL.m
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

// MACOSX_HIRES
// _LCD sub-system to use 1024x768 instead of the default 480x320 resolution
//
// 0 - 480x320 resolution
// 1 - 1024x768 resolution

#ifndef MACOSX_HIRES
#define MACOSX_HIRES        0
#endif

// MACOSX_PIXEL_DOUBLE
// perform pixel doubling at a hardware level - this allows for using a
// resolution of 320x160 and have it shown at 480x320 on the device screen
//
// 0 - ensure native resolution
// 1 - ensure pixel doubling resolution

#ifndef MACOSX_PIXEL_DOUBLE
#define MACOSX_PIXEL_DOUBLE 0
#endif

/*************************************************************************
 * Constants
 *************************************************************************/

#define DIRECTORY_SEPARATOR        '/'

#if (PLATFORM_ORIENTATION == 1)

#if (MACOSX_HIRES == 1)
#define APPLICATION_WIDTH          1024
#define APPLICATION_HEIGHT         768
#else
#define APPLICATION_WIDTH          480
#define APPLICATION_HEIGHT         320
#endif

#else

#if (MACOSX_HIRES == 1)
#define APPLICATION_WIDTH          768
#define APPLICATION_HEIGHT         1024
#else
#define APPLICATION_WIDTH          320
#define APPLICATION_HEIGHT         480
#endif

#endif

// options: special
#define OPTION_WINDOW 1

/*************************************************************************
 * System Includes
 *************************************************************************/

#define SHARK_INTERNAL

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sched.h>

#define pthread_yield sched_yield

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
extern int inet_addr(const char *);

#include "../SHARK.h"
#include "../SHARK-core.h"

#define _UINT8
#define _UINT16
#define _UINT32

#import <Cocoa/Cocoa.h>
#include <AudioUnit/AudioUnit.h>
#undef nil

// these are pre 10.7 API's - no longer available in the headers
#if ((MAC_OS_X_VERSION_MIN_REQUIRED < 1070) && \
     (MAC_OS_X_VERSION_MAX_ALLOWED >= 1070))

CG_EXTERN void *CGDisplayBaseAddress(CGDirectDisplayID display)
  CG_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_6, __IPHONE_NA, __IPHONE_NA);
CG_EXTERN size_t CGDisplayBytesPerRow(CGDirectDisplayID display)
  CG_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_6, __IPHONE_NA, __IPHONE_NA);

enum {
  kUIModeNormal                 = 0,
  kUIModeContentSuppressed      = 1,
  kUIModeContentHidden          = 2,
  kUIModeAllHidden              = 3,
  kUIModeAllSuppressed          = 4
};

typedef UInt32     SystemUIMode;
typedef OptionBits SystemUIOptions;

CG_EXTERN OSStatus SetSystemUIMode(SystemUIMode inMode, 
                                   SystemUIOptions inOptions)
  CG_AVAILABLE_BUT_DEPRECATED(__MAC_10_2, __MAC_10_6, __IPHONE_NA, __IPHONE_NA);

#endif

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define MAX_PATH_LENGTH 256

//------------------------------------------------------------------------
// Global Variables

#define GLOBALS                (Globals *)(&g)
#define GLOBALS_UNINITIALIZED  (GLOBALS == NULL)
#define GLOBALS_DEFINITION     Globals g

#define DAL_GLOBALS \
int              argc;          \
char           **argv;          \
                                \
int              options;       \
SInt32           sys_version;   \
CFDictionaryRef  display_mode;  \
boolean          framebuffer;   \
                                \
id               delegate;      \
NSApplication   *app;           \
NSWindow        *window;        \
NSRect           windowRect;    \
uint16          *windowBitmap;  \
NSView          *view;          \
NSTimer         *timer;         \
                                \
boolean          penDown;       \
int16            penX;          \
int16            penY;          \
                                \
pthread_t        thread;        \
pthread_mutex_t  thread_mutex;  \
int              timer_ms;      \
                                \
FILE            *log;

#define g_argc                 (GLOBALS) -> argc
#define g_argv                 (GLOBALS) -> argv

#define g_options              (GLOBALS) -> options
#define g_sys_version          (GLOBALS) -> sys_version 
#define g_display_mode         (GLOBALS) -> display_mode
#define g_framebuffer          (GLOBALS) -> framebuffer

#define g_delegate             (GLOBALS) -> delegate
#define g_app                  (GLOBALS) -> app
#define g_window               (GLOBALS) -> window
#define g_windowRect           (GLOBALS) -> windowRect
#define g_windowBitmap         (GLOBALS) -> windowBitmap
#define g_view                 (GLOBALS) -> view
#define g_timer                (GLOBALS) -> timer

#define g_penDown              (GLOBALS) -> penDown
#define g_penX                 (GLOBALS) -> penX
#define g_penY                 (GLOBALS) -> penY

#define g_thread               (GLOBALS) -> thread
#define g_thread_mutex         (GLOBALS) -> thread_mutex
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

  // open the log file for writing
  g_log = stdout;
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
}
#endif

/*************************************************************************
 * Platform Dependent Implementation
 *************************************************************************/

CFDictionaryRef DAL_CGDisplayBestModeForParameters(CGDirectDisplayID, size_t, size_t, size_t, boolean_t); 

void
DAL_ProcessArguments(int argc, char **argv)
{
  int                i;
  NSAutoreleasePool *pool;

  pool = [ [ NSAutoreleasePool alloc ] init ];

  // lets define our global path
  strcpy(g_path, [ [ [ NSBundle mainBundle ] bundlePath ] UTF8String ]);
  strcat(g_path, "/Contents/Resources");

  g_options = 0;
  for (i=1; i<argc; i++)
  {
    if (strcmp(argv[i], "--noFullScreen") == 0)
      g_options |= OPTION_WINDOW;
  }

  [ pool release ];
}

boolean
DAL_Initialize(uint16 width, uint16 height)
{
  boolean result;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // startup SHARK
#if (MACOSX_PIXEL_DOUBLE == 1)
  width  = width >> 1;
  height = height >> 1;
#endif
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

void
DAL_InitApplication()
{
  _PreferencesLoad();
}

void *
DAL_EventLoop(void *data)
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

  g_running = true;
  while (g_running)
  {
    // make sure the application is in focus
    if (g_active)
    {
      // dispatch the events in SHARK
      while (_SysEventAvailable() && g_running)
        _SHARKDispatchEvents();
    }

    // give operating system chance to do something else
    pthread_mutex_lock(&g_thread_mutex);
    usleep(g_timer_ms * 500);
    pthread_mutex_unlock(&g_thread_mutex);
  }

  // we must shut down the main thread
  [ g_delegate performSelectorOnMainThread:@selector(shutdown)
               withObject:NULL waitUntilDone: NO ];

  return NULL;
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
}

CFDictionaryRef 
DAL_CGDisplayBestModeForParameters(CGDirectDisplayID display, 
                                   size_t bpp, size_t width, size_t height, 
                                   boolean_t stretch)
{  
  CFArrayRef      modeArray;
  CFDictionaryRef mode;  
  CFNumberRef     num;  
  int             i, len;
  int             examWidth, examHeight, examBpp, examStretch;  
     
  // iterate through the known display modes (does not work from 10.7+)
  modeArray = CGDisplayAvailableModes(display);  
  len = CFArrayGetCount(modeArray);  
  for (i=0; i < len; i++)  
  {  
    // get this mode.  
    mode = (CFDictionaryRef) CFArrayGetValueAtIndex(modeArray,i);  
        
    // lets filter some "undesired" (obvious) modes
    if (!CFDictionaryContainsKey(mode, kCGDisplayModeIsSafeForHardware)) 
      continue;  
    if (CFDictionaryContainsKey(mode, kCGDisplayModeIsInterlaced))  
      continue;  
    if (CFDictionaryContainsKey(mode, kCGDisplayModeIsTelevisionOutput))  
      continue;  
        
    // obtain the various attribtues of the mode
    num = CFDictionaryGetValue(mode, kCGDisplayWidth);  
    CFNumberGetValue(num, kCFNumberLongType, &examWidth);  
    num = CFDictionaryGetValue(mode, kCGDisplayHeight);
    CFNumberGetValue(num, kCFNumberLongType, &examHeight);  
    num = CFDictionaryGetValue(mode, kCGDisplayBitsPerPixel);
    CFNumberGetValue(num, kCFNumberLongType, &examBpp);  
    examStretch = CFDictionaryContainsKey(mode, kCGDisplayModeIsStretched);  

    // do we have an exact match?
    if ((examWidth  == width)  && (examHeight == height) &&  
        (examBpp    == bpp)    && (stretch == examStretch))  
      goto MODE_SEARCH_DONE;
  }  
   
  // if we got this far; we need to "default" and beg :)
  if (stretch) 
    mode = CGDisplayBestModeForParametersAndRefreshRateWithProperty(display, 
      bpp, width, height, 60, kCGDisplayModeIsStretched, NULL);  
  else
    mode = CGDisplayBestModeForParameters(display, bpp, width, height, NULL);

MODE_SEARCH_DONE:;

  return mode;
}  

@class ApplicationWindow;

@interface ApplicationWindow : NSWindow
{
}
- (BOOL) canBecomeKeyWindow;

@end

@implementation ApplicationWindow

- (BOOL) canBecomeKeyWindow
{
  return YES;
}

@end

@class ApplicationView;

@interface ApplicationView : NSView
{
  int trackingTag;
}
- (id) initWithFrame:(NSRect)frame;
- (void) updateView;
- (void) drawRect:(NSRect)pRect;
- (void) mouseDown:(NSEvent *)theEvent;
- (void) mouseDragged:(NSEvent *)theEvent;
- (void) mouseUp:(NSEvent *)theEvent;
- (void) clearTrackingRect;
- (void) resetTrackingRect;
- (void) resetCursorRects;
- (void) viewDidMoveToWindow;
- (void) mouseEntered:(NSEvent *)theEvent;
- (void) mouseMoved:(NSEvent *)theEvent;
- (void) mouseExited:(NSEvent *)theEvent;
- (BOOL) acceptsFirstResponder;
- (void) keyDown:(NSEvent *)theEvent;

- (void) showDialogAlert:(NSArray *)array;
@end

@implementation ApplicationView

- (id) initWithFrame:(NSRect)frame
{
  NSRect sub_frame;
  uint32 size;

  // default return value
  self = [ super initWithFrame:frame ];

  // we want to embed a webview
  sub_frame.origin.x    = frame.origin.x;
  sub_frame.origin.y    = frame.origin.y;
  sub_frame.size.width  = frame.size.width;
  sub_frame.size.height = frame.size.height;

  // this only applies when running as a window
  if (((g_options & OPTION_WINDOW) != 0) || (!g_framebuffer))
  {
    // allocate the bitmap framebuffer
    size = (APPLICATION_WIDTH << 1) * APPLICATION_HEIGHT;
#if (MACOSX_PIXEL_DOUBLE == 1)
    size = size >> 2;
#endif
    g_windowBitmap = malloc(size);
  }

  return self;
}

- (void) updateView
{
  // this only applies when running as a window
  if (((g_options & OPTION_WINDOW) != 0) || (!g_framebuffer))
  {
    [ self setNeedsDisplay:YES ];

    // this is critical to ensure we get mouse move events
    [ [ self window ] setAcceptsMouseMovedEvents:YES ];
    [ [ self window ] makeFirstResponder:self ];
  }
}

- (void)drawRect:(NSRect)pRect
{
  if (((g_options & OPTION_WINDOW) != 0) || (!g_framebuffer))
  {
    NSGraphicsContext *gc;
    CGContextRef       gcContext, context;
    CGColorSpaceRef    cs;
    CGImageRef         img;
    CGRect             rect;
    CGBitmapInfo       bitmapInfo;

    [ NSGraphicsContext saveGraphicsState ];

    // what is the rectangle we are drawing to?
    rect = CGRectMake(pRect.origin.x,   pRect.origin.y,
                      pRect.size.width, pRect.size.height);

    gc = [NSGraphicsContext currentContext];
    gcContext = (CGContextRef)[ gc graphicsPort ];

    // create generic RGB color space.
    cs = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
  
    // create the bitmap context
    bitmapInfo = kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder16Host;
    context = CGBitmapContextCreate(g_windowBitmap,
                                    g_width, g_height,
                                    5, (g_width << 1),
                                    cs, bitmapInfo);

    if (context)
    {
      // get an image from the context
      img = CGBitmapContextCreateImage(context);
  
      // blit the bitmap
      CGContextDrawImage(gcContext, rect, img);
  
      // release image
      CGImageRelease(img);
    }
  
    // release the context
    CGContextRelease(context);

    // release colorspace before returning
    CGColorSpaceRelease(cs);

    [ NSGraphicsContext restoreGraphicsState ];
  }
}

- (void) mouseDown:(NSEvent *)theEvent
{
  int16   x, y;
  NSPoint p = [self convertPoint:[theEvent locationInWindow] fromView:NULL];

  x = (int16)p.x;
  y = (int16)p.y;

  // scale to right co-ordinates
  x = (int16)(x / ((float)g_windowRect.size.width  / APPLICATION_WIDTH));
  y = (int16)(y / ((float)g_windowRect.size.height / APPLICATION_HEIGHT));

  // flip the y-axis
  y = (int16)(APPLICATION_HEIGHT - 1) - y;

#if (MACOSX_PIXEL_DOUBLE == 1)
  x = x / 2;
  y = y / 2;
#endif

  // do we have some valid points?
  if ((x >= 0) && (x < g_width) &&
      (y >= 0) && (y < g_height))
  {
    // lock the system mutex
    DAL_SysMutexLock();

    g_penDown = true;
    g_penX    = (int16)x;
    g_penY    = (int16)y;

    // unlock the system mutex
    DAL_SysMutexUnlock();

    _SHARKPenEvent((int16)x, (int16)y);
  }
}

- (void) mouseDragged:(NSEvent *)theEvent
{
  int16   x, y;
  NSPoint p = [self convertPoint:[theEvent locationInWindow] fromView:NULL];

  x = (int16)p.x;
  y = (int16)p.y;

  // scale to right co-ordinates
  x = (int16)(x / ((float)g_windowRect.size.width  / APPLICATION_WIDTH));
  y = (int16)(y / ((float)g_windowRect.size.height / APPLICATION_HEIGHT));

  // flip the y-axis
  y = (int16)(APPLICATION_HEIGHT - 1) - y;

#if (MACOSX_PIXEL_DOUBLE == 1)
  x = x / 2;
  y = y / 2;
#endif

  // do we have some valid points?
  if ((x >= 0) && (x < g_width) &&
      (y >= 0) && (y < g_height))
  {
    // we only care about different points
    if ((g_penX != x) || (g_penY != y))
    {
      // lock the system mutex
      DAL_SysMutexLock();

      g_penDown = true;
      g_penX    = (int16)x;
      g_penY    = (int16)y;

      // unlock the system mutex
      DAL_SysMutexUnlock();

      _SHARKPenEvent((int16)x, (int16)y);
    }
  }
}

- (void) mouseUp:(NSEvent *)theEvent
{
  int16  x, y;
  NSPoint p = [self convertPoint:[theEvent locationInWindow] fromView:NULL];

  x = (int16)p.x;
  y = (int16)p.y;

  // scale to right co-ordinates
  x = (int16)(x / ((float)g_windowRect.size.width  / APPLICATION_WIDTH));
  y = (int16)(y / ((float)g_windowRect.size.height / APPLICATION_HEIGHT))
;

  // flip the y-axis
  y = (int16)(APPLICATION_HEIGHT - 1) - y;

#if (MACOSX_PIXEL_DOUBLE == 1)
  x = x / 2;
  y = y / 2;
#endif

  // lock the system mutex
  DAL_SysMutexLock();

  // we know the pen isn't down anymore
  g_penDown = false;

  // unlock the system mutex
  DAL_SysMutexUnlock();

  // do we have some valid points?
  if ((x >= 0) && (x < g_width) &&
      (y >= 0) && (y < g_height))
  {
    // we only care about different points
    if ((g_penX != x) || (g_penY != y))
    {
      // lock the system mutex
      DAL_SysMutexLock();

      g_penX = (int16)x;
      g_penY = (int16)y;

      // unlock the system mutex
      DAL_SysMutexUnlock();
    }
  }
}

- (void)clearTrackingRect
{
  if (trackingTag > 0)
  {
    [ self removeTrackingRect:trackingTag ];
    trackingTag = 0;
  }
}

- (void)resetTrackingRect
{
  [ self clearTrackingRect ];
  trackingTag = [ self addTrackingRect:[ self visibleRect ]
                       owner:self userData:NULL assumeInside:NO ];
}

- (void)resetCursorRects
{
  [ super resetCursorRects ];
  [ self resetTrackingRect ];
}

- (void)viewDidMoveToWindow
{
  if ([self window]) 
    [ self resetTrackingRect ];
}

- (void) mouseEntered:(NSEvent *)theEvent
{
  [ [ self window ] setAcceptsMouseMovedEvents:YES ];
  [ [ self window ] makeFirstResponder:self ];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
  int16  x, y;
  NSPoint p = [self convertPoint:[theEvent locationInWindow] fromView:NULL];

  x = (int16)p.x;
  y = (int16)p.y;

  // scale to right co-ordinates
  x = (int16)(x / ((float)g_windowRect.size.width  / APPLICATION_WIDTH));
  y = (int16)(y / ((float)g_windowRect.size.height / APPLICATION_HEIGHT))
;

  // flip the y-axis
  y = (int16)(APPLICATION_HEIGHT - 1) - y;

#if (MACOSX_PIXEL_DOUBLE == 1)
  x = x / 2;
  y = y / 2;
#endif

  // we know the pen isn't down anymore
  g_penDown = false;

  // do we have some valid points?
  if ((x >= 0) && (x < g_width) &&
      (y >= 0) && (y < g_height))
  {
    // we only care about different points
    if ((g_penX != x) || (g_penY != y))
    {
      g_penX = (uint16)x;
      g_penY = (uint16)y;
    }
  }
}

- (void)mouseExited:(NSEvent *)theEvent
{
  [ [ self window ] setAcceptsMouseMovedEvents:NO];
}

- (BOOL) acceptsFirstResponder
{
  return YES;
}

- (BOOL) performKeyEquivalent:(NSEvent *)theEvent
{
  BOOL    result;
  event   e;
  unichar key;

  result = false;

  key = [ [ theEvent characters ] characterAtIndex: 0 ];
  switch (key)
  {
    case 'Q': case 'q':

         // enqueue a appStop event
         _SysGetEvent(&e);
         e.eType = _appStopEvent;
         _SysEnqueueEvent(&e);

         result = true;

         break;

    default:
         break;
  }

  return result;
}

- (void) keyDown:(NSEvent *)theEvent
{
  unichar key;

  key = [ [ theEvent characters ] characterAtIndex: 0 ];
  _SHARKKeyEvent((uint16)key);
}

- (void)showDialogAlert:(NSArray *)args
{
  // this only applies when running as a window
  if ((g_options & OPTION_WINDOW) != 0)
  {
    NSAlert *alert;

    // create an alert
    alert = [ NSAlert alertWithMessageText:[ args objectAtIndex:0 ]
                      defaultButton:@"OK"
                      alternateButton:NULL
                      otherButton:NULL
                      informativeTextWithFormat:[ args objectAtIndex:1 ] ];
    if (alert)
    {
      // display the alert - modal fashion
      [ alert runModal ];
    }
  }
}

- (void) dispose
{
  // this only applies when running as a window
  if ((g_options & OPTION_WINDOW) != 0)
  {
    // release the bitmap framebuffer
    free(g_windowBitmap);
    g_windowBitmap = NULL;
  }
}

@end

//------------------------------------------------------------------------
// SHARKApplication

@interface SHARKApplication : NSApplication 
#if ((MAC_OS_X_VERSION_MIN_REQUIRED < 1070) && \
     (MAC_OS_X_VERSION_MAX_ALLOWED >= 1070))
  <NSApplicationDelegate>
#endif
{
}

- (id) init;
- (NSString *) applicationName;
- (void) applicationDidFinishLaunching:(NSNotification *)aNotification;
- (void) applicationDidBecomeActive:(NSNotification *)aNotification;
- (void) applicationWillResignActive:(NSNotification *)aNotification;
- (void) applicationWillTerminate:(NSNotification *)aNotification;
- (void) shutdown;
- (void) timerProc;

@end

@implementation SHARKApplication

- (id) init 
{
  if ((self = [ super init ])) 
  {
    [ self setDelegate:self ];
  }
  return self;
}

- (NSString *) applicationName
{
  return [ NSString stringWithCString:g_config.name
                    encoding:NSASCIIStringEncoding ];
}

- (void) applicationDidFinishLaunching:(NSNotification *)aNotification 
{
  NSString                     *applicationName;
  NSMenu                       *mainMenu, *subMenu;
  NSMenuItem                   *item;
  NSNotificationCenter         *center;
  size_t                        depth  = 16;
  size_t                        width  = APPLICATION_WIDTH;  
  size_t                        height = APPLICATION_HEIGHT;
  CFDictionaryRef               mode;
  CGDisplayFadeReservationToken fade;
  float                         interval;

  // obtain the application configuration parameters
  memset(&g_config, 0, sizeof(application_config));
  ApplicationConfig(&g_config);

  // store these globally
  g_app      = self;
  g_delegate = self;

  // application mode: window
  if ((g_options & OPTION_WINDOW) != 0)
  {
    // the name of the application
    applicationName = [ self applicationName ];

    // setup the application menu (basic menu)
    mainMenu = [ [ [ NSMenu alloc ] initWithTitle:@"MainMenu" ] autorelease ];
    subMenu = [ [ [ NSMenu alloc ] initWithTitle:@"Apple" ] autorelease ];
    {
      item = [ subMenu addItemWithTitle:
               [ NSString stringWithFormat:@"Quit %@", applicationName ]
               action:@selector(shutdown) keyEquivalent:@"q" ];
      [ item setTarget:NSApp ];
    }
    item = [ mainMenu addItemWithTitle:@"Apple" action:NULL keyEquivalent:@"" ];
    [ NSApp performSelector:@selector(setAppleMenu:) withObject:subMenu ];
    [ mainMenu setSubmenu:subMenu forItem:item ];
    [ NSApp setMainMenu:mainMenu ];

    // what are the boundaries of the window?
    g_windowRect = NSMakeRect(0, 0, APPLICATION_WIDTH, APPLICATION_HEIGHT);

    // create the main window
    g_window = [[ NSWindow alloc ]
                    initWithContentRect: g_windowRect
                    styleMask:(NSClosableWindowMask |
                               NSMiniaturizableWindowMask |
                               NSTitledWindowMask )
                    backing:NSBackingStoreBuffered defer:false screen:NULL ];

    // setup event notifications
    center = [NSNotificationCenter defaultCenter];
    [ center addObserver:self selector:@selector(shutdown)
             name:NSWindowWillCloseNotification object:NULL ];

    // create a view and add it to our window
    g_view   = [[ ApplicationView alloc ] initWithFrame:g_windowRect ];
    [ g_window setContentView:g_view ];

    // show the window
    [ g_window setTitle:applicationName ];
    [ g_window center ];
    [ g_window makeKeyAndOrderFront:NULL ];
  }
  else 

  // application mode: full screen 
  {
    // lets fade the screen before switching
    fade = kCGDisplayFadeReservationInvalidToken;
    if (CGAcquireDisplayFadeReservation(5, &fade) == kCGErrorSuccess)
    {
      CGDisplayFade(fade, 0.3, kCGDisplayBlendNormal, 
                    kCGDisplayBlendSolidColor, 0.0, 0.0, 0.0, TRUE);
    }

    // disable the menu/dock
    SetSystemUIMode(kUIModeContentHidden, 0);

    // we should really ensure we dont go below 1024x768 when in full screen
    width  = MAX(width, 1024);
    height = MAX(height, 768);

    // configure the full screen display mode
#ifdef __BIG_ENDIAN__
    mode = CGDisplayBestModeForParametersAndRefreshRateWithProperty(kCGDirectMainDisplay, 
      depth, width, height, 60, kCGDisplayModeIsStretched, NULL);  
#else
    mode = DAL_CGDisplayBestModeForParameters(kCGDirectMainDisplay,
                                              depth, width, height, true);
#endif
    g_display_mode = CGDisplayCurrentMode(kCGDirectMainDisplay);
    CGDisplaySwitchToMode(kCGDirectMainDisplay, mode);
    CGDisplayCapture(kCGDirectMainDisplay);

    // fade back in
    if (fade != kCGDisplayFadeReservationInvalidToken) 
    {
      CGDisplayFade(fade, 0.5, kCGDisplayBlendSolidColor, 
                    kCGDisplayBlendNormal, 0.0, 0.0, 0.0, FALSE);
      CGReleaseDisplayFadeReservation(fade);
    }

    // what are the boundaries of the window?
    g_windowRect = [ [ NSScreen mainScreen ] frame ];

    // can we write direct to the framebuffer?
    g_framebuffer = true; 
    if (g_sys_version >= 0x1070) g_framebuffer &= false; // no support in lion
    if ((g_windowRect.size.width  != APPLICATION_WIDTH) ||
        (g_windowRect.size.height != APPLICATION_HEIGHT))
      g_framebuffer &= false; // framebuffer doesn't match widnow 

    // create the main window
    g_window = [[ ApplicationWindow alloc ]
                  initWithContentRect: g_windowRect
                  styleMask:NSBorderlessWindowMask
                  backing:NSBackingStoreBuffered defer:false screen:NULL ];
    if (g_framebuffer) [ g_window setViewsNeedDisplay:NO ];

    // create a view and add it to our window
    g_view   = [[ ApplicationView alloc ] initWithFrame:g_windowRect ];
    [ g_window setContentView:g_view ];
    [ g_window setAcceptsMouseMovedEvents:YES ];
    [ g_window makeFirstResponder:g_view ];

    // force window to draw above everything
    [ g_window setLevel:CGShieldingWindowLevel() ];
    [ g_window makeKeyAndOrderFront:NULL ];
  }

#if (LOG_DISABLE == 0)
  // initialize the logging engine
  DAL_LogInitialize();
#endif

  // initialize our abstraction layer
  if (DAL_Initialize(APPLICATION_WIDTH, APPLICATION_HEIGHT))
  {
    // whats the desired fps the application wants us to use?
    g_timer_ms = 10000L / MAX(100, MIN(g_config.event.fps, 600)); // 10..60fps

    // setup a callback timer
    interval = 10.0f / MAX(100, MIN(g_config.event.fps, 600)); // 10..60fps
    g_timer = [NSTimer scheduledTimerWithTimeInterval:interval
                       target:self selector:@selector(timerProc)
                       userInfo:NULL repeats:YES];

#if (LOG_DISABLE == 0)
    // LOGGING EVENT
    _SHARKLogCreateEntry(":: - application initialize -\n");
#endif

    if ((g_timer != NULL) && (ApplicationInitialize()))
    {
      g_initialized = true;
      DAL_InitApplication();

      // application now has focus
      _SHARKFocusEvent(true);

      // we need to start an application thread
      pthread_mutex_init(&g_thread_mutex, NULL);
      pthread_create(&g_thread, NULL, (void *)DAL_EventLoop, NULL);
    }
    else
    {
      // we must terminate
      [ g_delegate performSelectorOnMainThread:@selector(shutdown)
                   withObject:NULL waitUntilDone: NO ];
    }
  }
}

- (void) applicationDidBecomeActive:(NSNotification *)aNotification
{
  // resume the thread
  if (g_running)
  {
    // application now has gained focus
    _SHARKFocusEvent(true);

    pthread_mutex_unlock(&g_thread_mutex);
  }
}

- (void) applicationWillResignActive:(NSNotification *)aNotification
{
  // pause the thread
  if (g_running)
  {
    pthread_mutex_lock(&g_thread_mutex);

    // application now has lost focus
    _SHARKFocusEvent(false);
  }
}

- (void)shutdown
{
  // only if the timer is running - prevent double call
  if (g_timer)
    [ [ NSApplication sharedApplication] terminate: NULL ];
}

- (void) applicationWillTerminate:(NSNotification *)aNotification 
{
  event                         e;
  CGDisplayFadeReservationToken fade;

  if (g_initialized)
  {
    // stop the callback timer
    if (g_timer != NULL)
      [ g_timer invalidate ];
    g_timer = NULL;

    // if the application is still running; send a quit event
    if (g_running)
    {
      // lets bring the thread to a close
      pthread_mutex_lock(&g_thread_mutex);
      g_running = false;
      pthread_mutex_unlock(&g_thread_mutex);

      // enqueue a appStop event
      _SysGetEvent(&e);
      e.eType = _appStopEvent;
      _SHARKHandleEvent(&e);
    }

    DAL_QuitApplication();
    g_initialized = false;

    ApplicationTerminate();

#if (LOG_DISABLE == 0)
    // LOGGING EVENT
    _SHARKLogCreateEntry("\n:: - application terminate -\n");
#endif
  }

  DAL_Terminate();

#if (LOG_DISABLE == 0)
  // initialize the logging engine
  DAL_LogTerminate();
#endif

  // application mode: window
  if ((g_options & OPTION_WINDOW) != 0)
  {
    // dispose of the view
    [ g_view release ];
    g_view = NULL;

    // dispose of the window
    [ g_window release ];
    g_window = NULL;
  }
  else
 
  // application mode: full screen
  {
    // dispose of the view
    [ g_view release ];
    g_view = NULL;

    // dispose of the window
    [ g_window release ];
    g_window = NULL;

    // lets fade the screen before switching
    fade = kCGDisplayFadeReservationInvalidToken;
    if (CGAcquireDisplayFadeReservation(5, &fade) == kCGErrorSuccess)
    {
      CGDisplayFade(fade, 0.3, kCGDisplayBlendNormal, 
                    kCGDisplayBlendSolidColor, 0.0, 0.0, 0.0, TRUE);
    }
  
    // re-enable the menu/dock
    SetSystemUIMode(kUIModeNormal, 0);

    // release the display mode
    CGDisplaySwitchToMode(kCGDirectMainDisplay, g_display_mode);
    CGDisplayRelease(kCGDirectMainDisplay);
  
    // fade back in
    if (fade != kCGDisplayFadeReservationInvalidToken ) 
    {
      CGDisplayFade(fade, 0.5, kCGDisplayBlendSolidColor, 
                    kCGDisplayBlendNormal, 0.0, 0.0, 0.0, FALSE);
      CGReleaseDisplayFadeReservation(fade);
    }
  }
}

- (void) timerProc
{
  event e;

  // we only post nilevents if we are active
  if (g_active)
  {
    memset(&e, 0, sizeof(event));
    e.eType = _nilEvent;
    _SysEnqueueEvent(&e);
  }
}

@end

//------------------------------------------------------------------------
// ApplicationBundle

@interface ApplicationBundle : NSBundle 
{
}

@end

@implementation ApplicationBundle

+ (BOOL) loadNibNamed:(NSString *)aNibNamed owner:(id)owner 
{
  if (!aNibNamed && (owner == NSApp)) 
  {
    // we're lying here. don't load anything.
    return YES;
  } 
  else 
    return [ super loadNibNamed:aNibNamed owner:owner ];
}

@end

/*************************************************************************
 * Entry Point
 *************************************************************************/

int 
main(int argc, char * argv[]) 
{
  int result;
  NSAutoreleasePool *pool;

  // initialize our globals
  memset(GLOBALS, 0, sizeof(Globals));

  // store a reference to the program arguments
  g_argc = argc;
  g_argv = argv;
  DAL_ProcessArguments(argc, argv);

  // we need a reference to the system version (10.7+ changed some API's)
  Gestalt(gestaltSystemVersion, &g_sys_version);

  // load the main bundle
  [ [ ApplicationBundle class ] poseAsClass:[ NSBundle class ] ];

  // start the application
  pool = [ [ NSAutoreleasePool alloc ] init ];
  result = NSApplicationMain(argc, (const char **)argv);
  [ pool release ];
  
  return result;
}

/*************************************************************************
 * Local Dependencies
 *************************************************************************/

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED

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

/********************************* EOF ***********************************/
