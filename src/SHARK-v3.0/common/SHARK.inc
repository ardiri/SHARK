/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

typedef struct Globals
{
  boolean  running;

  boolean  initialized;
  boolean  active;

  char     path[MAX_PATH_LENGTH];

  uint16   width;
  uint16   height;

  application_config config;

#if defined(DAL_GLOBALS)
  DAL_GLOBALS
#endif

} Globals;

GLOBALS_DEFINITION;

#define g_running              (GLOBALS) -> running    

#define g_initialized          (GLOBALS) -> initialized
#define g_active               (GLOBALS) -> active     

#define g_path                 (GLOBALS) -> path     

#define g_width                (GLOBALS) -> width
#define g_height               (GLOBALS) -> height

#define g_config               (GLOBALS) -> config

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

#if (LOG_DISABLE == 0)
static void DAL_LogInitialize();
static void DAL_LogCreateEntry(char *str);
static void DAL_LogTerminate();
#endif

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
_SHARKInitialize(uint16 width, uint16 height)
{
  boolean init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(":: hardware abstraction initialize\n");
#endif

  // assume everything is ok
  init_ok =  true;

  // initialize all sub-components
  init_ok &= _LCDInitialize(width, height);
  init_ok &= _FileInitialize();
  init_ok &= _SysInitialize();
  init_ok &= _LibInitialize();
  init_ok &= _InputInitialize();
  init_ok &= _PreferencesInitialize();
  init_ok &= _SocketInitialize();
  init_ok &= _SndInitialize();
  init_ok &= _ResourceInitialize();
  init_ok &= _MemInitialize();

  // initialize the default orientation state based on window resolution
  _LCDSetOrientation(lcd_rotate0);

  // we need to record these values to assist in detection of changes
  g_width  = width;
  g_height = height;

  return init_ok;
}

static void
_SHARKDispatchEvents()
{
  event _e;
  int16 eType;

  // platform independent 'processing' of events
  if (_SysEventAvailable())
  {
    if (_SysGetEvent(&_e))
    {
      eType = _e.eType;
      _SHARKHandleEvent(&_e);
   
      // application termination condition
      if (eType == _appStopEvent)
      {
        // we must break our event loop, now!
        g_running = false;
      }
    }
  }
}

static boolean
_SHARKHandleEvent(event *e)
{
  boolean result;

  // default return value
  result = false;

  // pass it through all active libraries
  result = _LibHandleEvent(e);
  if (!result)
    result = ApplicationHandleEvent(e);

  return result;
}

static void
_SHARKFocusEvent(boolean state)
{
  event e;

  // if we are already in the state - why change?
  if (g_active == state) return;

  // based on the state, we need to perform a few housekeeping things
  if (state)
  {
    // resume LCD operations
    _LCDResume();

    // restart the active audio stream
    _SndStreamStart(0);

    memset(&e, 0, sizeof(event));
    e.eType = _keyEvent;
    e.data.key.chr = _key_focus_gain;
    _SHARKHandleEvent(&e);
  }
  else
  {
    memset(&e, 0, sizeof(event));
    e.eType = _keyEvent;
    e.data.key.chr = _key_focus_lost;
    _SHARKHandleEvent(&e);

    // suspend the active audio stream
    _SndStreamStop(0);

    // suspend LCD operations
    _LCDSuspend();
  }

  // we need to set this variable to prevent events from happening
  g_active = state;
}

static void
_SHARKKeyEvent(uint16 keycode)
{
  _KeyHandleEvent(keycode);
}

static void
_SHARKPenEvent(int16 x, int16 y)
{
  _PenHandleEvent(x, y);
}

static void
_SHARKRotateEvent(int16 dir)
{
  lcd_orientation orientation;
  _LCDGetProperties(NULL, NULL, NULL, NULL, NULL, &orientation, NULL);
  _LCDSetOrientation(orientation + dir);
}

static void
_SHARKResizeEvent(uint16 width, uint16 height)
{
  event e;

  // suspend any event processing 
  g_active = false;

  _LCDTerminate_RESIZE();
  _LCDInitialize_RESIZE(width, height);

  // we need to record these values to assist in detection of changes
  g_width  = width;
  g_height = height;

  // now, the application must handle the new LCD instance
  memset(&e, 0, sizeof(event));
  e.eType        = _keyEvent;
  e.data.key.chr = _key_lcd_resize;
  _SHARKHandleEvent(&e);

  // force a repaint on the display
  _LCDRepaint();

  // resume any event processing 
  g_active = true;
}

#if (LOG_DISABLE == 0)
static void
_SHARKLogCreateEntry(char *str)
{
  DAL_LogCreateEntry(str);
}
#endif

static void
_SHARKTerminate()
{
  // shutdown all sub-components
  _MemTerminate();
  _ResourceTerminate();
  _SndTerminate();
  _SocketTerminate();
  _PreferencesTerminate();
  _InputTerminate();
  _LibTerminate();
  _SysTerminate();
  _FileTerminate();
  _LCDTerminate();

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry("\n:: hardware abstraction terminate\n");
#endif
}

/********************************* EOF ***********************************/
