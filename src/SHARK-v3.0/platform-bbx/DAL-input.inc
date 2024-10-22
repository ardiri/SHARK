/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-input.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define INPUT_GLOBALS         g_input
#define GLOBALS_UNINITIALIZED (INPUT_GLOBALS == NULL)
#define GLOBALS_DEFINITION    InputGlobals *g_input = NULL

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-input.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
DAL_InputInitialize()
{
  boolean init_ok;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(INPUT_GLOBALS, 0, sizeof(InputGlobals));

  return init_ok;
}

static boolean
DAL_InputAvailable(input_device type)
{
  boolean result;

  // default return value
  result = false;

  switch (type)
  {
    case input_stylus: 
    case input_pointer: 
#if (ACCELEROMETER_DISABLE == 0)
    case input_accelerometer: 
#endif
         result = true;
         break;

#if (ACCELEROMETER_DISABLE == 1)
    case input_accelerometer: 
#endif
    case input_dpad: 
    case input_tablet: 
    case input_joystick: 
    case input_analog: 
    case input_gyroscope: 
    default:
         break;
  }

  return result;
}

static void
DAL_InputState(input *device)
{
  // default values
  device -> x     = 0;
  device -> y     = 0;
  device -> value = 0;

  switch (device -> type)
  {
    case input_stylus: 
    case input_pointer:

         // lock the system mutex
         DAL_SysMutexLock();

         // we just read the values from our globals
         device -> x     = g_penX;
         device -> y     = g_penY;
         device -> value = (g_penDown ? 1 : 0);

         // unlock the system mutex
         DAL_SysMutexUnlock();

         break;

#if (ACCELEROMETER_DISABLE == 0)
    case input_accelerometer:

         // lock the system mutex
         DAL_SysMutexLock();

         // we just read the values from our globals
         device -> x     = g_accelX;
         device -> y     = g_accelY;
         device -> value = g_accelZ;

         // unlock the system mutex
         DAL_SysMutexUnlock();

         break;
#endif

#if (ACCELEROMETER_DISABLE == 1)
    case input_accelerometer: 
#endif
    case input_dpad:
    case input_tablet: 
    case input_joystick: 
    case input_analog: 
    case input_gyroscope: 
    default:
         break;
  }

  // give operating system a little time to do other stuff
  pthread_yield();
}

static void
DAL_KeyboardSetVisible(boolean visible)
{
  if (visible)
    virtualkeyboard_show();
  else
    virtualkeyboard_hide();
}

static void
DAL_KeyHandleEvent(uint16 keycode, event *e)
{
  // initialize the event
  memset(e, 0, sizeof(event));
  e -> eType = _keyEvent;

  // what keycode did we receive?
  switch (keycode)
  {
// NO MAPPING AVAILABLE:
//       e -> data.key.chr = _key_up;
//       e -> data.key.chr = _key_down;
//       e -> data.key.chr = _key_left;
//       e -> data.key.chr = _key_right;
//       e -> data.key.chr = _key_select;

    case KEYCODE_BACKSPACE:
         e -> data.key.chr = _key_backspace;
         break;

    case KEYCODE_TAB:
         e -> data.key.chr = _key_tab;
         break;

    case KEYCODE_KP_ENTER:
    case KEYCODE_RETURN:
         e -> data.key.chr = _key_enter;
         break;

// NO MAPPING AVAILABLE:
//       e -> data.key.chr = _key_home;
//       e -> data.key.chr = _key_end;
//       e -> data.key.chr = _key_pageup;
//       e -> data.key.chr = _key_pagedown;
//       e -> data.key.chr = _key_scrollup;
//       e -> data.key.chr = _key_scrolldown;
//       e -> data.key.chr = _key_help;
//       e -> data.key.chr = _key_menu;
//       e -> data.key.chr = _key_acknowledge;
//       e -> data.key.chr = _key_cancel;

// WINDOW MANAGER:
//       e -> data.key.chr = _key_focus_gain;
//       e -> data.key.chr = _key_focus_lost;
//       e -> data.key.chr = _key_lcd_resize;

    case KEYCODE_ESCAPE:
         e -> data.key.chr = _key_escape;
         break;

    case KEYCODE_DELETE:
         e -> data.key.chr = _key_delete;
         break;

    default:
         if ((keycode > 0x1f) && (keycode < 0xe000))
           e -> data.key.chr = keycode;
         break;
  }  
}

static void
DAL_PenHandleEvent(int16 penX, int16 penY, event *e)
{
  // initialize the event
  memset(e, 0, sizeof(event));
  e -> eType = _penEvent;
}

static uint32
DAL_KeyState(uint16 *keyX, uint16 *keyY)
{
  uint32 result;

  // default return value
  result = 0;

  return result;
}

static void
DAL_PenState(uint16 *penX, uint16 *penY, boolean *penDown)
{
  input device;

  // query the stylus/pointer location
  device.type = input_stylus;
  _InputState(&device);

  if (penX    != NULL) *penX = device.x;
  if (penY    != NULL) *penY = device.y;
  if (penDown != NULL) *penDown = (device.value != 0);
}

void 
DAL_InputTerminate()
{
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef INPUT_GLOBALS

/********************************* EOF ***********************************/
