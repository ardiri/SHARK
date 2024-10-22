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
    case input_dpad: 
         result = true;
         break;

    case input_tablet: 
    case input_joystick: 
    case input_analog: 
    case input_gyroscope: 
    case input_accelerometer: 
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

         // we are going to ignore x, y - use the "main" event values
         device -> x = g_penX;
         device -> y = g_penY;
         device -> value = (g_penDown ? 1 : 0);

         // unlock the system mutex
         DAL_SysMutexUnlock();

         break;

    case input_dpad:
         {
           Uint8 *keystate;

           keystate = SDL_GetKeyState(NULL);

           if (keystate[SDLK_LEFT])  device -> x = -INPUT_VALUE_EXTREME;
           if (keystate[SDLK_RIGHT]) device -> x =  INPUT_VALUE_EXTREME;
           if (keystate[SDLK_UP])    device -> y = -INPUT_VALUE_EXTREME;
           if (keystate[SDLK_DOWN])  device -> y =  INPUT_VALUE_EXTREME;
           device -> value = 0;
         }
         break;

    case input_tablet: 
    case input_joystick: 
    case input_analog: 
    case input_gyroscope: 
    case input_accelerometer: 
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

  switch (keycode)
  {
    case SDLK_UP:
         e -> data.key.chr = _key_up;
         break;

    case SDLK_DOWN:
         e -> data.key.chr = _key_down;
         break;

    case SDLK_LEFT:
         e -> data.key.chr = _key_left;
         break;

    case SDLK_RIGHT:
         e -> data.key.chr = _key_right;
         break;

// NO MAPPING AVAILABLE
//       e -> data.key.chr = _key_select;

    case SDLK_BACKSPACE:
         e -> data.key.chr = _key_backspace;
         break;

    case SDLK_TAB:
         e -> data.key.chr = _key_tab;
         break;

    case SDLK_RETURN:
         e -> data.key.chr = _key_enter;
         break;

    case SDLK_HOME:
         e -> data.key.chr = _key_home;
         break;

    case SDLK_END:
         e -> data.key.chr = _key_end;
         break;

    case SDLK_PAGEUP:
         e -> data.key.chr = _key_pageup;
         break;

    case SDLK_PAGEDOWN:
         e -> data.key.chr = _key_pagedown;
         break;

// NO MAPPING AVAILABLE
//       e -> data.key.chr = _key_scrollup;
//       e -> data.key.chr = _key_scrolldown;

    case SDLK_F1:
         e -> data.key.chr = _key_help;
         break;

    case SDLK_F2:
         e -> data.key.chr = _key_menu;
         break;

    case SDLK_F3:
         e -> data.key.chr = _key_acknowledge;
         break;

    case SDLK_F4:
         e -> data.key.chr = _key_cancel;
         break;

// WINDOW MANAGER:
//       e -> data.key.chr = _key_focus_gain;
//       e -> data.key.chr = _key_focus_lost;
//       e -> data.key.chr = _key_lcd_resize;
    case SDLK_F5:
         _SHARKRotateEvent(1);
         break;

    case SDLK_ESCAPE:
         e -> data.key.chr = _key_escape;
         break;

    case SDLK_DELETE:
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
  input  device;

  // default return value
  result = 0;

  // query the directional dpad location
  device.type = input_dpad;
  _InputState(&device);

  // process the values from the input device
  if (keyX != NULL) *keyX = device.x;
  if (keyY != NULL) *keyY = device.y;
  result = device.value;

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
