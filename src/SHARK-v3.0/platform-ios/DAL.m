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
// disable the use of the accelerometer if available - an optimization
// to prevent an uneccessary thread and processing, especially when the
// application has no requirement for this functionality.
// 
// 1 - disable accelerometer use
// 0 -  enable accelerometer use

#ifndef ACCELEROMETER_DISABLE
#define ACCELEROMETER_DISABLE 1
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

// IOS_HIRES
// _LCD sub-system to use 1024x768 instead of the default 480x320 resolution
//
// 0 - 480x320 resolution
// 1 - 1024x768 resolution

#ifndef IOS_HIRES
#define IOS_HIRES 0
#endif

// IOS_PIXEL_DOUBLE
// perform pixel doubling of the framebuffer window at hardware level.
// 
// 0 - ensure native resolution
// 1 - ensure pixel doubling resolution

#ifndef IOS_PIXEL_DOUBLE
#define IOS_PIXEL_DOUBLE 0
#endif

// IOS_ALTERNATE_BUFFERS
// use two buffers for display engine - this prevents "tearing" on the
// display; however applications must draw the whole frame every iteration

#ifndef IOS_ALTERNATE_BUFFERS
#define IOS_ALTERNATE_BUFFERS 0
#endif

/*************************************************************************
 * Constants
 *************************************************************************/

#define DIRECTORY_SEPARATOR        '/'

#if (IOS_HIRES == 1)

#if (IOS_PIXEL_DOUBLE == 1)
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

#if (IOS_PIXEL_DOUBLE == 1)
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

#define SHARK_INTERNAL

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <sched.h>

#define pthread_yield sched_yield

#include "../SHARK.h"
#include "../SHARK-core.h"

#define _UINT32
#define _UINT16
#define _UINT8

#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/EAGLDrawable.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>
#undef nil

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define GLOBALS                ((Globals *)&g)
#define GLOBALS_UNINITIALIZED  (GLOBALS == NULL)
#define GLOBALS_DEFINITION     Globals g;

#define DAL_GLOBALS \
int                  argc;             \
char               **argv;             \
                                       \
id                   delegate;         \
UIApplication       *app;              \
UIWindow            *window;           \
CGRect               windowRect;       \
UIView              *view;             \
UIViewController    *view_controller;  \
UIDeviceOrientation  view_orientation; \
NSTimer             *timer;            \
UITextField         *textField;        \
                                       \
boolean              penDown;          \
int16                penX;             \
int16                penY;             \
                                       \
pthread_t            thread;           \
                                       \
FILE                *log;

#define g_argc                 (GLOBALS) -> argc
#define g_argv                 (GLOBALS) -> argv

#define g_delegate             (GLOBALS) -> delegate
#define g_app                  (GLOBALS) -> app
#define g_window               (GLOBALS) -> window
#define g_windowRect           (GLOBALS) -> windowRect
#define g_view                 (GLOBALS) -> view
#define g_view_controller      (GLOBALS) -> view_controller
#define g_view_orientation     (GLOBALS) -> view_orientation
#define g_timer                (GLOBALS) -> timer
#define g_textField            (GLOBALS) -> textField

#define g_penX                 (GLOBALS) -> penX
#define g_penY                 (GLOBALS) -> penY
#define g_penDown              (GLOBALS) -> penDown

#define g_thread               (GLOBALS) -> thread

#define g_log                  (GLOBALS) -> log

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK.inc"

#if (LOG_DISABLE == 0)
static void
DAL_LogInitialize()
{
  uint32 platform;
  char   platform_name[256];

  // all logging is to console
  printf("SOFTWARE:\n");
  printf("---------\n");
  printf("company         :: %s\n", g_config.company);
  printf("application     :: %s\n", g_config.name);
  printf("\n");
        
  _SysDeviceInfo(&platform, platform_name);
  printf("PLATFORM:\n");
  printf("---------\n");
  printf("platform id     :: %s\n", platform_name);
  printf("\n");
}

static void
DAL_LogCreateEntry(char *str)
{
  printf("%s\n", str);
}

static void
DAL_LogTerminate()
{
}
#endif

/*************************************************************************
 * Platform Dependent Implementation
 *************************************************************************/

void
DAL_ProcessArguments(int argc, char **argv)
{
  char *p;
    
  // we need to know the applications path
  p = (char *)argv[0];
  getcwd(g_path, MAX_PATH_LENGTH);
  if (strncmp(p, "./",  2) == 0) p++;
  else if (strncmp(p, "../", 3) == 0) { p += 3; p = strchr(p, '/'); }
  if ((g_path[strlen(g_path)-1] != '/') && (*p != '/')) strcat(g_path, "/");
  strcat(g_path, p);  // join the current path + argv[0] name together
  *(strrchr(g_path, '/')) = 0; // remove the executable name
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
  result = _SHARKInitialize(width, height);
    
  // lock the system mutex
  DAL_SysMutexLock();

  // initialize our pen handling
  g_penX    = width  >> 1;
  g_penY    = height >> 1;
  g_penDown = false;

  // unlock the system mutex
  DAL_SysMutexUnlock();

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
  NSAutoreleasePool *pool;
  event _e;

  pool = [ [ NSAutoreleasePool alloc ] init ];

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
    pthread_yield();
  }

  // we must shut down the main thread
  [ g_delegate performSelectorOnMainThread:@selector(terminate) 
               withObject:NULL waitUntilDone: NO ];

  [ pool release ];

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

//--------------------------------------------------------------------
// ApplicationView

@class ApplicationView;

@interface ApplicationView : UIView <UITextFieldDelegate>
{
@public
    CGSize          _size;
#if (IOS_ALTERNATE_BUFFERS == 0)
    GLubyte        *_spriteFrameBuffer;
#else
    uint16          _spriteFrameIndex;
    GLubyte        *_spriteFrameBuffer[2];
#endif

@private
    EAGLContext    *_context;
    GLuint          _framebuffer;
    GLuint          _renderbuffer;
    GLuint          _spriteTexture;
    GLfloat         _spriteVertices[8];
    GLfloat         _spriteTexcoords[8];
}
- (id) initWithFrame:(CGRect)frame;
- (void) swapBuffers;

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event;
- (BOOL)textFieldShouldReturn:(UITextField *)textField;
- (void)keyPressed:(NSNotification *)notification;

- (void) showDialogAlert:(NSArray *)args;
@end

@implementation ApplicationView

#if (IOS_HIRES == 1)

#if (IOS_PIXEL_DOUBLE == 0)
#define TEXTURE_WIDTH        1024
#define TEXTURE_HEIGHT       1024
#else
#define TEXTURE_WIDTH        512
#define TEXTURE_HEIGHT       512
#endif

#else

#if (IOS_PIXEL_DOUBLE == 0)
#define TEXTURE_WIDTH        512
#define TEXTURE_HEIGHT       512
#else
#define TEXTURE_WIDTH        256
#define TEXTURE_HEIGHT       256
#endif

#endif

+ (Class) layerClass
{
  return [CAEAGLLayer class];
}

- (BOOL) _createSurface
{
  BOOL         result;
  CAEAGLLayer* eaglLayer;
  CGSize       newSize;
  uint32       size;

  // default return value
  result = FALSE;
    
  // set the current context
  if (![EAGLContext setCurrentContext:_context]) goto CREATE_FAILED;
    
  // whats the display dimensions?
  eaglLayer = (CAEAGLLayer *)[self layer];
  newSize = [eaglLayer bounds].size;

  // create and bind the frame buffers
  glGenRenderbuffersOES(1, &_renderbuffer);
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, _renderbuffer);
  if (![_context renderbufferStorage:GL_RENDERBUFFER_OES 
                 fromDrawable:(id<EAGLDrawable>)eaglLayer]) 
  {
    glDeleteRenderbuffersOES(1, &_renderbuffer);
    goto CREATE_FAILED;
  }
  glGenFramebuffersOES(1, &_framebuffer);
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, _framebuffer);
  glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, 
                               GL_RENDERBUFFER_OES, _renderbuffer);
    
  // configure viewport if necessary
  _size.width  = newSize.width;
  _size.height = newSize.height;
  glViewport(0, 0, _size.width, _size.height);

  // black background
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // Sets up matrixes and transforms for OpenGL ES
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
#if (PLATFORM_ORIENTATION == 1)
  glRotatef(90,0,0,1);
#endif
  glOrthof(0, TEXTURE_WIDTH, 0, TEXTURE_HEIGHT, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // sets up an array of values to use as the sprite vertices.
  _spriteVertices[0] = 0;
  _spriteVertices[1] = TEXTURE_HEIGHT;
  _spriteVertices[2] = TEXTURE_WIDTH;
  _spriteVertices[3] = TEXTURE_HEIGHT;
  _spriteVertices[4] = 0;
  _spriteVertices[5] = 0;
  _spriteVertices[6] = TEXTURE_WIDTH;
  _spriteVertices[7] = 0;

  // calculate our texture co-ordinates
  _spriteTexcoords[0] = 0.0f;
  _spriteTexcoords[1] = 0.0f;
  _spriteTexcoords[2] = (GLfloat)APPLICATION_WIDTH  / TEXTURE_WIDTH;
  _spriteTexcoords[3] = 0.0f;
  _spriteTexcoords[4] = 0.0f;
  _spriteTexcoords[5] = (GLfloat)APPLICATION_HEIGHT / TEXTURE_HEIGHT;
  _spriteTexcoords[6] = (GLfloat)APPLICATION_WIDTH  / TEXTURE_WIDTH;
  _spriteTexcoords[7] = (GLfloat)APPLICATION_HEIGHT / TEXTURE_HEIGHT;

#if (PLATFORM_ORIENTATION == 0)
  // we need to save this - special handling for touch events
  g_view_orientation = UIInterfaceOrientationPortrait;
#else
  // we need to save this - special handling for touch events
  g_view_orientation = UIInterfaceOrientationLandscapeLeft;

  // be sure all dialogs are shown in landscape mode
  [ [ UIApplication sharedApplication ]
      setStatusBarOrientation: UIInterfaceOrientationLandscapeLeft ];
#endif

  // allocate memory for the framebuffer
  size = TEXTURE_WIDTH * TEXTURE_HEIGHT * 2;

#if (IOS_ALTERNATE_BUFFERS == 0)
  _spriteFrameBuffer = (GLubyte *) malloc(size);
  if (_spriteFrameBuffer == NULL) goto CREATE_FAILED;
  memset(_spriteFrameBuffer, 0x00, size);
#else
  _spriteFrameBuffer[0] = (GLubyte *) malloc(size);
  if (_spriteFrameBuffer[0] == NULL) goto CREATE_FAILED;
  memset(_spriteFrameBuffer[0], 0x00, size);
  _spriteFrameBuffer[1] = (GLubyte *) malloc(size);
  if (_spriteFrameBuffer[1] == NULL) goto CREATE_FAILED;
    memset(_spriteFrameBuffer[1], 0x00, size);
#endif

  glGenTextures(1, &_spriteTexture);
  glBindTexture(GL_TEXTURE_2D, _spriteTexture);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DITHER);
  glDisable(GL_BLEND);
  glDisable(GL_LIGHTING);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_STENCIL_TEST);
  glEnable(GL_TEXTURE_2D);
  
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  // draw the textures
  [ self swapBuffers ];

  result = TRUE;
    
CREATE_FAILED:;
    
  return result;
}

- (void) _destroySurface
{
  EAGLContext *oldContext = [EAGLContext currentContext];

#if (IOS_ALTERNATE_BUFFERS == 0)
  if (_spriteFrameBuffer != NULL)
    free(_spriteFrameBuffer);
  _spriteFrameBuffer = NULL;
#else
  // free the memory used by the framebuffer
  if (_spriteFrameBuffer[0] != NULL)
    free(_spriteFrameBuffer[0]);
  _spriteFrameBuffer[0] = NULL;
  if (_spriteFrameBuffer[1] != NULL)
    free(_spriteFrameBuffer[1]);
  _spriteFrameBuffer[1] = NULL;
#endif
    
  // use the right context
  if (oldContext != _context) [EAGLContext setCurrentContext:_context];

  // delete the render/frame buffers
  glDeleteRenderbuffersOES(1, &_renderbuffer);
  glDeleteFramebuffersOES(1, &_framebuffer);
  _renderbuffer = (GLuint)NULL;    
  _framebuffer  = (GLuint)NULL;

  // use the right context
  if (oldContext != _context) [EAGLContext setCurrentContext:oldContext];
}

- (id) initWithFrame:(CGRect)frame
{    
  CAEAGLLayer* eaglLayer;
    
  // default return value
  self = [ super initWithFrame:frame ];
    
  if (self != NULL) 
  {
    // create an instance of the EAGL layer
    eaglLayer = (CAEAGLLayer*)[self layer];

    // set the drawable properties
    eaglLayer.opaque = YES; 
    eaglLayer.drawableProperties = 
      [ NSDictionary   dictionaryWithObjectsAndKeys:
          [ NSNumber numberWithBool:NO ], kEAGLDrawablePropertyRetainedBacking, 
            kEAGLColorFormatRGB565,       kEAGLDrawablePropertyColorFormat, 
            NULL
          ];

    // create the rendering context
    _context = [[ EAGLContext alloc ] initWithAPI:kEAGLRenderingAPIOpenGLES1 ];
    if ((_context == NULL) || (![self _createSurface])) 
    {
      [self release];
      return NULL;
    }
  }
        
  // we want to create a textfield; offscreen - for our keyboard
  g_textField = [ [ UITextField alloc ] 
                    initWithFrame:CGRectMake(-1, -1, 1, 1)];
  [ g_textField setDelegate:self ];
  g_textField.returnKeyType = UIReturnKeyDone;
  g_textField.keyboardType  = UIKeyboardTypeAlphabet;
  [ self addSubview:g_textField ];

  // ensure our keyboard events are handled
  [ g_textField setText:@"x" ];
  [ [ NSNotificationCenter defaultCenter ] 
    addObserver:self selector: @selector(keyPressed:) 
    name: UITextFieldTextDidChangeNotification object: g_textField ];

  return self;
}

- (void) dealloc
{
  // unregister for our keyboard events are handled
  [ [ NSNotificationCenter defaultCenter ] 
    removeObserver:self 
    name: UITextFieldTextDidChangeNotification object: g_textField ];
  [ g_textField release ];
  g_textField = NULL;

  // destroy the surface
  [self _destroySurface];

  // release the context
  [EAGLContext setCurrentContext:NULL];
  [_context release];
  _context = NULL;

  [super dealloc];
}

- (void) swapBuffers
{
  EAGLContext *oldContext = [ EAGLContext currentContext ];
  GLubyte *spriteFrameBuffer;

  // use the right context
  if (oldContext != _context) [ EAGLContext setCurrentContext:_context ];
    
  // choose the right framebuffer
#if (IOS_ALTERNATE_BUFFERS == 0)
  spriteFrameBuffer = _spriteFrameBuffer;
#else
  spriteFrameBuffer = _spriteFrameBuffer[ _spriteFrameIndex ];
#endif

  // update our texture
  glBindTexture(GL_TEXTURE_2D, _spriteTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 
               0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, spriteFrameBuffer);
  glTexCoordPointer(2, GL_FLOAT, 0, _spriteTexcoords);
  glVertexPointer(2, GL_FLOAT, 0, _spriteVertices);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // bind the render buffer
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, _renderbuffer);
  [ _context presentRenderbuffer:GL_RENDERBUFFER_OES ];

  // use the right context
  if (oldContext != _context) [ EAGLContext setCurrentContext:oldContext ];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
  CGPoint p = [[ touches anyObject ] locationInView:self ];
  CGPoint _p;

#if (PLATFORM_ORIENTATION == 1)
  if (g_view_orientation == UIInterfaceOrientationLandscapeLeft)
  {
    _p.x    = g_windowRect.size.height - p.y;
    _p.y    = p.x;
  }
  if (g_view_orientation == UIInterfaceOrientationLandscapeRight)
  {
    _p.x    = p.y;
    _p.y    = g_windowRect.size.width - p.x;
  }

  // scale to right co-ordinates
  _p.x = (int16)(_p.x / ((float)g_windowRect.size.height / APPLICATION_WIDTH));
  _p.y = (int16)(_p.y / ((float)g_windowRect.size.width  / APPLICATION_HEIGHT));
#else
  if (g_view_orientation == UIInterfaceOrientationPortrait)
  {
    _p.x    = p.x;
    _p.y    = p.y;
  }
  else
  if (g_view_orientation == UIInterfaceOrientationPortraitUpsideDown)
  {
    _p.x    = g_windowRect.size.width  - p.x;
    _p.y    = g_windowRect.size.height - p.y;
  }

  // scale to right co-ordinates
  _p.x = (int16)(_p.x / ((float)g_windowRect.size.width  / APPLICATION_WIDTH));
  _p.y = (int16)(_p.y / ((float)g_windowRect.size.height / APPLICATION_HEIGHT));
#endif

  // do we have some valid points?
  if ((_p.x >= 0) && (_p.x < g_width) &&
      (_p.y >= 0) && (_p.y < g_height))
  {
    // lock the system mutex
    DAL_SysMutexLock();

    g_penDown = true;
    g_penX    = (int16)_p.x;
    g_penY    = (int16)_p.y;
    
    // unlock the system mutex
    DAL_SysMutexUnlock();

    _SHARKPenEvent((int16)_p.x, (int16)_p.y);
  }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
  CGPoint p = [[ touches anyObject ] locationInView:self ];
  CGPoint _p;
    
#if (PLATFORM_ORIENTATION == 1)
  if (g_view_orientation == UIInterfaceOrientationLandscapeLeft)
  {
    _p.x    = g_windowRect.size.height - p.y;
    _p.y    = p.x;
  }
  if (g_view_orientation == UIInterfaceOrientationLandscapeRight)
  {
    _p.x    = p.y;
    _p.y    = g_windowRect.size.width - p.x;
  }

  // scale to right co-ordinates
  _p.x = (int16)(_p.x / ((float)g_windowRect.size.height / APPLICATION_WIDTH));
  _p.y = (int16)(_p.y / ((float)g_windowRect.size.width  / APPLICATION_HEIGHT));
#else
  if (g_view_orientation == UIInterfaceOrientationPortrait)
  {
    _p.x    = p.x;
    _p.y    = p.y;
  }
  else
  if (g_view_orientation == UIInterfaceOrientationPortraitUpsideDown)
  {
    _p.x    = g_windowRect.size.width  - p.x;
    _p.y    = g_windowRect.size.height - p.y;
  }

  // scale to right co-ordinates
  _p.x = (int16)(_p.x / ((float)g_windowRect.size.width  / APPLICATION_WIDTH));
  _p.y = (int16)(_p.y / ((float)g_windowRect.size.height / APPLICATION_HEIGHT));
#endif

  // do we have some valid points?
  if ((_p.x >= 0) && (_p.x < g_width) &&
      (_p.y >= 0) && (_p.y < g_height))
  {
    // we only care about different points
    if ((g_penX != (int16)_p.x) || 
        (g_penY != (int16)_p.y))
    {
      // lock the system mutex
      DAL_SysMutexLock();

      g_penDown = true;
      g_penX    = (int16)_p.x;
      g_penY    = (int16)_p.y;
    
      // unlock the system mutex
      DAL_SysMutexUnlock();

      _SHARKPenEvent((int16)_p.x, (int16)_p.y);
    }
  }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
  CGPoint p = [[ touches anyObject ] locationInView:self ];
  CGPoint _p;

#if (PLATFORM_ORIENTATION == 1)
  if (g_view_orientation == UIInterfaceOrientationLandscapeLeft)
  {
    _p.x    = g_windowRect.size.height - p.y;
    _p.y    = p.x;
  }
  if (g_view_orientation == UIInterfaceOrientationLandscapeRight)
  {
    _p.x    = p.y;
    _p.y    = g_windowRect.size.width - p.x;
  }

  // scale to right co-ordinates
  _p.x = (int16)(_p.x / ((float)g_windowRect.size.height / APPLICATION_WIDTH));
  _p.y = (int16)(_p.y / ((float)g_windowRect.size.width  / APPLICATION_HEIGHT));
#else
  if (g_view_orientation == UIInterfaceOrientationPortrait)
  {
    _p.x    = p.x;
    _p.y    = p.y;
  }
  else
  if (g_view_orientation == UIInterfaceOrientationPortraitUpsideDown)
  {
    _p.x    = g_windowRect.size.width  - p.x;
    _p.y    = g_windowRect.size.height - p.y;
  }

  // scale to right co-ordinates
  _p.x = (int16)(_p.x / ((float)g_windowRect.size.width  / APPLICATION_WIDTH));
  _p.y = (int16)(_p.y / ((float)g_windowRect.size.height / APPLICATION_HEIGHT));
#endif

  // lock the system mutex
  DAL_SysMutexLock();

  // we know the pen isn't down anymore
  g_penDown = false;

  // unlock the system mutex
  DAL_SysMutexUnlock();

  // do we have some valid points?
  if ((_p.x >= 0) && (_p.x < g_width) &&
      (_p.y >= 0) && (_p.y < g_height))
  {
    // lock the system mutex
    DAL_SysMutexLock();

    g_penX = (int16)_p.x;
    g_penY = (int16)_p.y;

    // unlock the system mutex
    DAL_SysMutexUnlock();
  }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
  // lock the system mutex
  DAL_SysMutexLock();

  // we know the pen isn't down anymore
  g_penDown = false;

  // unlock the system mutex
  DAL_SysMutexUnlock();
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
  [ textField resignFirstResponder ];
  return YES;
}

- (void)keyPressed:(NSNotification *)notification
{
  NSString *s = [ g_textField text ];

  // lets grab the key value
  if ([ s length ] > 1)
    _SHARKKeyEvent((uint16)([ s characterAtIndex:1 ])); 
  if ([ s length ]  == 0)
    _SHARKKeyEvent(8); // backspace

  // a little special hack so we can detect a backspace/delete key
  [ [ NSNotificationCenter defaultCenter ] 
    removeObserver:self 
    name: UITextFieldTextDidChangeNotification object: g_textField ];
  [ g_textField setText:@"x" ];
  [ [ NSNotificationCenter defaultCenter ] 
    addObserver:self selector: @selector(keyPressed:) 
    name: UITextFieldTextDidChangeNotification object: g_textField ];
}

- (void)showDialogAlert:(NSArray *)args
{
  UIAlertView *alert;

  // create an alert
  alert = [[ UIAlertView alloc ]
             initWithTitle:[ args objectAtIndex:0 ]
             message:[ args objectAtIndex:1 ]
             delegate:NULL cancelButtonTitle:NULL
             otherButtonTitles:@"OK", NULL];
  if (alert)
  {
    // UNDOCUMENTED API: we want the alert to be modal; wait 
//  [ alert setRunsModal:YES ];
    {
      int i;
//    char api[] = "setRunsModal:";
      char api[] = "tfuSvotNpebm;";

      // lets convert our selector to what it should be
      for (i=0;i<strlen(api); i++)
        api[i] = api[i] - 1;

      [ alert performSelector:sel_registerName(api)
                              withObject: (id)YES ];
    }

    // display the alert
    [ alert show ];
  }
}

@end

//--------------------------------------------------------------------
// ApplicationViewController

@class ApplicationViewController;

@interface ApplicationViewController : UIViewController
{
}
- (void) viewWillAppear: (BOOL) animated;
- (void) receivedRotate: (NSNotification*) notification;
- (void) viewWillDisappear: (BOOL) animated;
- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation;
@end

@implementation ApplicationViewController

- (void) viewWillAppear: (BOOL) animated
{
  [ [ UIDevice currentDevice ] beginGeneratingDeviceOrientationNotifications ];
  [ [ NSNotificationCenter defaultCenter ]
     addObserver: self
     selector:    @selector(receivedRotate:)
     name: UIDeviceOrientationDidChangeNotification object: NULL ];
}

- (void) receivedRotate: (NSNotification*) notification
{
  UIDeviceOrientation orientation;

  // what orientation was the device changed to?
  orientation = [ [ UIDevice currentDevice ] orientation ];

#if (PLATFORM_ORIENTATION == 0)
  // portrait
  if ((orientation == UIInterfaceOrientationPortrait) &&
      (g_view_orientation != UIInterfaceOrientationPortrait))
  {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glRotatef(0,0,0,1);
    glOrthof(0, TEXTURE_WIDTH, 0, TEXTURE_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // we need to save this - special handling for touch events
    g_view_orientation = UIInterfaceOrientationPortrait;

    // be sure all dialogs are shown in portrait mode
    [ [ UIApplication sharedApplication ]
        setStatusBarOrientation: UIInterfaceOrientationPortrait ];

    // ensure our user interface gets updated
    [ g_view performSelectorOnMainThread:@selector(swapBuffers)
             withObject:NULL waitUntilDone: NO ];
  }
  else
  if ((orientation == UIInterfaceOrientationPortraitUpsideDown) &&
      (g_view_orientation != UIInterfaceOrientationPortraitUpsideDown))
  {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glRotatef(180,0,0,1);
    glOrthof(0, TEXTURE_WIDTH, 0, TEXTURE_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // we need to save this - special handling for touch events
    g_view_orientation = UIInterfaceOrientationPortraitUpsideDown;

    // be sure all dialogs are shown in portrait upside down mode
    [ [ UIApplication sharedApplication ]
        setStatusBarOrientation: UIInterfaceOrientationPortraitUpsideDown ];

    // ensure our user interface gets updated
    [ g_view performSelectorOnMainThread:@selector(swapBuffers)
             withObject:NULL waitUntilDone: NO ];
  }
#else
  // landscape
  if ((orientation == UIInterfaceOrientationLandscapeLeft) &&
      (g_view_orientation = UIInterfaceOrientationLandscapeLeft))
  {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glRotatef(90,0,0,1);
    glOrthof(0, TEXTURE_WIDTH, 0, TEXTURE_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // we need to save this - special handling for touch events
    g_view_orientation = UIInterfaceOrientationLandscapeLeft;

    // be sure all dialogs are shown in landscape left
    [ [ UIApplication sharedApplication ]
        setStatusBarOrientation: UIInterfaceOrientationLandscapeLeft ];

    // ensure our user interface gets updated
    [ g_view performSelectorOnMainThread:@selector(swapBuffers)
             withObject:NULL waitUntilDone: NO ];
  }
  else
  if ((orientation == UIInterfaceOrientationLandscapeRight) &&
      (g_view_orientation = UIInterfaceOrientationLandscapeRight))
  {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glRotatef(-90,0,0,1);
    glOrthof(0, TEXTURE_WIDTH, 0, TEXTURE_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // we need to save this - special handling for touch events
    g_view_orientation = UIInterfaceOrientationLandscapeRight;

    // be sure all dialogs are shown in landscape right
    [ [ UIApplication sharedApplication ]
        setStatusBarOrientation: UIInterfaceOrientationLandscapeRight ];

    // ensure our user interface gets updated
    [ g_view performSelectorOnMainThread:@selector(swapBuffers)
             withObject:NULL waitUntilDone: NO ];
  }
#endif
}

- (void) viewWillDisappear: (BOOL) animated
{
  [ [ NSNotificationCenter defaultCenter ] removeObserver: self ];
  [ [ UIDevice currentDevice ] endGeneratingDeviceOrientationNotifications ];
}

- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
  return NO;
}

@end

//--------------------------------------------------------------------
// SHARKApplication

@class SHARKApplication;

@interface SHARKApplication : NSObject <UIAccelerometerDelegate>
{
#if (ACCELEROMETER_DISABLE == 0)
@public
    int16 _accelX;
    int16 _accelY;
    int16 _accelZ;
#endif
}
- (void) applicationDidFinishLaunching:(UIApplication *)application;
- (void) applicationDidBecomeActive:(UIApplication *)application;
- (void) applicationWillResignActive:(UIApplication *)application;
- (void) applicationWillTerminate:(UIApplication *)application;
- (void) terminate;
#if (ACCELEROMETER_DISABLE == 0)
- (void) accelerometer:(UIAccelerometer *)accelerometer 
         didAccelerate:(UIAcceleration *)acceleration;
#endif
- (void) timerProc;

@end

@implementation SHARKApplication

- (void)applicationDidFinishLaunching:(UIApplication *)application
{
  float  interval;
  int16  w, h, width, height, dx, dy;
    
  // we need a copy of this for later
  g_app      = application;
  g_delegate = self;
  g_windowRect  = [[UIScreen mainScreen] bounds];

#if (PLATFORM_ORIENTATION == 1)
  w = APPLICATION_HEIGHT;
  h = APPLICATION_WIDTH;
#else
  w = APPLICATION_WIDTH;
  h = APPLICATION_HEIGHT;
#endif

  width  = (int16)g_windowRect.size.width;
  height = (int16)g_windowRect.size.height;
  if (((width % w) != 0) || ((height % h) != 0))
  {
    // lets ensure there is an integer scaling factor
    g_windowRect.size.width = w;
    while (((int16)g_windowRect.size.width + w) <= width)
      g_windowRect.size.width += w;
    g_windowRect.size.height = h;
    while (((int16)g_windowRect.size.height + h) <= height)
      g_windowRect.size.height += h;

    dx = width  - (int16)g_windowRect.size.width;
    dy = height - (int16)g_windowRect.size.height;
    g_windowRect.origin.x = dx >> 1;
    g_windowRect.origin.y = dy >> 1;
  }

  // create a full-screen window
  g_window = [[UIWindow alloc] initWithFrame:g_windowRect ];

  // create the OpenGL view (no offset, at 0,0 in our window)
  g_windowRect.origin.x = 0;
  g_windowRect.origin.y = 0;
  g_view = [[ApplicationView alloc] initWithFrame:g_windowRect ]; 

  // create the view controller and bind it
  g_view_controller = [[ApplicationViewController alloc] init ];
  g_view_controller.view = g_view;

  // add the the OpenGL view to the window
  [ g_window addSubview:g_view ];

  // show the window
  [ g_window makeKeyAndVisible ];

  // lets prevent the screen from dimming
  [[UIApplication sharedApplication] setIdleTimerDisabled:YES];

#if (ACCELEROMETER_DISABLE == 0)
  // lets register for the accelerometer (30fps)
  [ [ UIAccelerometer sharedAccelerometer ] setUpdateInterval:(1.0f / 30) ];
  [ [ UIAccelerometer sharedAccelerometer ] setDelegate:self ];
#endif

#if (LOG_DISABLE == 0)
  // initialize the logging engine
  DAL_LogInitialize();
#endif
    
  // initialize the platform
  if (DAL_Initialize(APPLICATION_WIDTH, APPLICATION_HEIGHT))
  {
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
      pthread_create(&g_thread, NULL, (void *)DAL_EventLoop, NULL);
    }
  }
  else
  {
    // we must shut down the main thread
    [ g_delegate performSelectorOnMainThread:@selector(terminate) 
                 withObject:NULL waitUntilDone: YES ];
  }
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
  // application now has gained focus
  _SHARKFocusEvent(true);
}

- (void)applicationWillResignActive:(UIApplication *)application
{
  // application now has lost focus
  _SHARKFocusEvent(false);
}

- (void)applicationWillTerminate:(UIApplication *)application
{
  if ((g_initialized) && (g_timer != NULL))
  {
    // stop the callback timer
    [ g_timer invalidate ];
    g_timer = NULL;

    // if the application is still running; send a quit event
    if (g_running)
    {
      // enqueue a appStop event
      _SysEnqueueQuitEvent();
        
      // dispatch all the remaining events in SHARK
      while (_SysEventAvailable() && g_running)
        _SHARKDispatchEvents();
    }   

    // we need to wait for the application thread to terminate
    if (g_running) 
    {
      g_running = false;
      pthread_cancel(g_thread);
      pthread_join(g_thread, NULL);
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
    
  // release the view and window 
  [ g_view_controller release ];
  [ g_view            release ];
  [ g_window          release ];
  g_view_controller = NULL;
  g_view            = NULL;
  g_window          = NULL;
}

- (void)terminate
{
  [ self applicationWillTerminate: g_app ];
  exit(0);
}

#if (ACCELEROMETER_DISABLE == 0)
- (void)accelerometer:(UIAccelerometer *)accelerometer 
        didAccelerate:(UIAcceleration *)acceleration
{
  // lock the system mutex
  DAL_SysMutexLock();

  // store these values for later use
  _accelX = (int16)(acceleration.x * 256);
  _accelY = (int16)(acceleration.y * 256);
  _accelZ = (int16)(acceleration.z * 256);

  // unlock the system mutex
  DAL_SysMutexUnlock();
}
#endif

- (void)timerProc
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

/*************************************************************************
 * Entry Point (iPod Touch/iPhone SDK)
 *************************************************************************/

int
main(int argc, char **argv)
{
  int result;
  NSAutoreleasePool *pool;
    
  // initialize our globals
  memset(GLOBALS, 0, sizeof(GLOBALS));
    
  // store a reference to the program arguments
  g_argc = argc;
  g_argv = argv;
  DAL_ProcessArguments(argc, argv);
    
  // obtain the application configuration parameters
  memset(&g_config, 0, sizeof(application_config));
  ApplicationConfig(&g_config);
  
  pool = [ [ NSAutoreleasePool alloc ] init ];
  result = UIApplicationMain(argc, argv, NULL, @"SHARKApplication");
  [ pool release ];
    
  return result;
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

/********************************* EOF ***********************************/

