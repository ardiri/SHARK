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
         {
           POINT pt;
           int   x, y;

           device -> value = (GetAsyncKeyState(VK_LBUTTON) != 0);
           if (device -> value)
           {
             GetCursorPos(&pt);
             ScreenToClient(g_hWnd, &pt);

             // convert to x,y
             x = (int16)pt.x;
             y = (int16)pt.y;

#ifdef GFX_SCALING_150

             x = (x * 2) / 3;
             y = (y * 2) / 3;

#endif

             // do we have some valid values?
             if ((x >= 0) && (x < g_width) &&
                 (y >= 0) && (y < g_height))
             {
               device -> x     = x;
               device -> y     = y;
             }
           }
         }
         break;

    case input_dpad:
         {
           uint32 mask;

           // DPAD: left
           if (GetAsyncKeyState(VK_LEFT) ||
               GetAsyncKeyState(VK_NUMPAD6))
             device -> x = -INPUT_VALUE_EXTREME;

           // DPAD: right
           if (GetAsyncKeyState(VK_RIGHT) ||
               GetAsyncKeyState(VK_NUMPAD4))
             device -> x =  INPUT_VALUE_EXTREME;

           // DPAD: up
           if (GetAsyncKeyState(VK_UP) ||
               GetAsyncKeyState(VK_NUMPAD8))
             device -> y = -INPUT_VALUE_EXTREME;

           // DPAD: down
           if (GetAsyncKeyState(VK_DOWN) ||
               GetAsyncKeyState(VK_NUMPAD2))
             device -> y =  INPUT_VALUE_EXTREME;

           // KEYS: modifiers
           device -> value = 0;
       
           mask = 0x0001;
           if (GetAsyncKeyState(VK_CONTROL))
             device -> value |= mask; mask <<= 1;
           if (GetAsyncKeyState(VK_SHIFT))
             device -> value |= mask; mask <<= 1;
           if (GetAsyncKeyState(VK_TAB))
             device -> value |= mask; mask <<= 1;
           if (GetAsyncKeyState(VK_SPACE))
             device -> value |= mask; mask <<= 1;
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
    case VK_UP:
    case VK_NUMPAD8:
         e -> data.key.chr = _key_up;
         break;

    case VK_DOWN:
    case VK_NUMPAD2:
         e -> data.key.chr = _key_down;
         break;

    case VK_LEFT:
    case VK_NUMPAD4:
         e -> data.key.chr = _key_left;
         break;

    case VK_RIGHT:
    case VK_NUMPAD6:
         e -> data.key.chr = _key_right;
         break;

// NO MAPPING AVAILABLE
//       e -> data.key.chr = _key_select;

    case VK_BACK:
         e -> data.key.chr = _key_backspace;
         break;

    case VK_TAB:
         e -> data.key.chr = _key_tab;
         break;

    case VK_RETURN:
    case VK_NUMPAD5:
         e -> data.key.chr = _key_enter;
         break;

    case VK_HOME:
         e -> data.key.chr = _key_home;
         break;

    case VK_END:
         e -> data.key.chr = _key_end;
         break;

    case VK_PRIOR:
         e -> data.key.chr = _key_pageup;
         break;

    case VK_NEXT:
         e -> data.key.chr = _key_pagedown;
         break;

// NO MAPPING AVAILABLE
//       e -> data.key.chr = _key_scrollup;
//       e -> data.key.chr = _key_scrolldown;

    case VK_F1:
         e -> data.key.chr = _key_help;
         break;

    case VK_F2:
         e -> data.key.chr = _key_menu;
         break;

    case VK_F3:
         e -> data.key.chr = _key_acknowledge;
         break;

    case VK_F4:
         e -> data.key.chr = _key_cancel;
         break;

    case VK_ESCAPE:
         e -> data.key.chr = _key_escape;
         break;

// WINDOW MANAGER:
//       e -> data.key.chr = _key_focus_gain;
//       e -> data.key.chr = _key_focus_lost;
//       e -> data.key.chr = _key_lcd_resize;
    case VK_F5:
         _SHARKRotateEvent(1);
         break;

    case VK_DELETE:
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
  if (penDown != NULL) *penDown = (boolean)device.value;
}

void 
DAL_InputTerminate()
{
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef INPUT_GLOBALS

/********************************* EOF ***********************************/
