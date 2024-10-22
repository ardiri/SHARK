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
         result = true;
         break;

    case input_accelerometer: 
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

         // we just read the values from our view object
         device -> x     = g_penX;
         device -> y     = g_penY;
         device -> value = g_penDown;

         // unlock the system mutex
         DAL_SysMutexUnlock();

         break;

    case input_accelerometer: 
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
    case NSUpArrowFunctionKey:
         e -> data.key.chr = _key_up;
         break;

    case NSDownArrowFunctionKey:
         e -> data.key.chr = _key_down;
         break;

    case NSLeftArrowFunctionKey:
    case NSPrevFunctionKey:
         e -> data.key.chr = _key_left;
         break;

    case NSRightArrowFunctionKey:
    case NSNextFunctionKey:
         e -> data.key.chr = _key_right;
         break;

// NO MAPPING AVAILABLE
//       e -> data.key.chr = _key_select;

    case NSBackspaceCharacter:
         e -> data.key.chr = _key_backspace;
         break;

    case NSTabCharacter:
         e -> data.key.chr = _key_tab;
         break;

    case NSEnterCharacter:
    case NSNewlineCharacter:
    case NSCarriageReturnCharacter:
         e -> data.key.chr = _key_enter;
         break;

    case NSHomeFunctionKey:
         e -> data.key.chr = _key_home;
         break;

    case NSEndFunctionKey:
         e -> data.key.chr = _key_end;
         break;

    case NSPageUpFunctionKey:
         e -> data.key.chr = _key_pageup; 
         break;

    case NSPageDownFunctionKey:
         e -> data.key.chr = _key_pagedown;
         break;

// NO MAPPING AVAILABLE
//       e -> data.key.chr = _key_scrollup;
//       e -> data.key.chr = _key_scrolldown;

    case NSF1FunctionKey:
         e -> data.key.chr = _key_help;
         break;

    case NSF2FunctionKey:
    case NSMenuFunctionKey:
         e -> data.key.chr = _key_menu;
         break;

    case NSF3FunctionKey:
         e -> data.key.chr = _key_acknowledge;
         break;

    case NSF4FunctionKey:
         e -> data.key.chr = _key_cancel;
         break;

    case 27:
         e -> data.key.chr = _key_escape;
         break;

// WINDOW MANAGER:
//       e -> data.key.chr = _key_focus_gain;
//       e -> data.key.chr = _key_focus_lost;
//       e -> data.key.chr = _key_lcd_resize;
    case NSF5FunctionKey:
         _SHARKRotateEvent(1);
         break;

    case NSDeleteCharFunctionKey:
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
