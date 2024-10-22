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
    case input_sylus:
    case input_pointer:
         {
           Window            rw, cw;
           int               rx, ry, x, y, _x, _y;
           uint              m;
           XWindowAttributes wA;

           XGetWindowAttributes(g_display, g_window, &wA);
           _x = (wA.width  - APPLICATION_WINDOW_WIDTH)  >> 1;
           _y = (wA.height - APPLICATION_WINDOW_HEIGHT) >> 1;

           // query the pointer location
           if (XQueryPointer(g_display, g_window, 
                             &rw, &cw, &rx, &ry, &x, &y, &m))
           {
#ifdef GFX_SCALING_150 

             x = ((x - _x) * 2) / 3;
             y = ((y - _y) * 2) / 3;

#endif

             // do we have some valid values?
             if ((x >= 0) && (x < g_width) &&
                 (y >= 0) && (y < g_height))
             {
               device -> x     = x;
               device -> y     = y;
               device -> value = m;
             }
           }
         }
         break;

    case input_dpad:
         {
           char    keys[32];
           KeyCode keycode;
           uint16  mask;

           // query the keymap
           XQueryKeymap(g_display, keys);

           // DPAD: left
           keycode = XKeysymToKeycode(g_display, XK_Left);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> x = -INPUT_VALUE_EXTREME;

           // DPAD: right
           keycode = XKeysymToKeycode(g_display, XK_Right);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> x =  INPUT_VALUE_EXTREME;

           // DPAD: up
           keycode = XKeysymToKeycode(g_display, XK_Up);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> y = -INPUT_VALUE_EXTREME;

           // DPAD: down
           keycode = XKeysymToKeycode(g_display, XK_Down);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> y =  INPUT_VALUE_EXTREME;

           // KEYS: modifiers
           device -> value = 0;

           mask = 0x0001;
           keycode = XKeysymToKeycode(g_display, XK_Shift_L);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> value |= mask; mask <<= 1;
           keycode = XKeysymToKeycode(g_display, XK_Shift_R);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> value |= mask; mask <<= 1;
           keycode = XKeysymToKeycode(g_display, XK_Control_L);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> value |= mask; mask <<= 1;
           keycode = XKeysymToKeycode(g_display, XK_Control_R);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> value |= mask; mask <<= 1;
           keycode = XKeysymToKeycode(g_display, XK_Shift_Lock);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> value |= mask; mask <<= 1;
           keycode = XKeysymToKeycode(g_display, XK_Meta_L);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> value |= mask; mask <<= 1;
           keycode = XKeysymToKeycode(g_display, XK_Meta_R);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> value |= mask; mask <<= 1;
           keycode = XKeysymToKeycode(g_display, XK_Alt_L);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> value |= mask; mask <<= 1;
           keycode = XKeysymToKeycode(g_display, XK_Alt_R);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> value |= mask; mask <<= 1;
           keycode = XKeysymToKeycode(g_display, XK_Super_L);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> value |= mask; mask <<= 1;
           keycode = XKeysymToKeycode(g_display, XK_Super_R);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> value |= mask; mask <<= 1;
           keycode = XKeysymToKeycode(g_display, XK_Hyper_L);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> value |= mask; mask <<= 1;
           keycode = XKeysymToKeycode(g_display, XK_Hyper_R);
           if (((keys[keycode >> 3] >> (keycode & 0x07)) & 0x01) != 0) 
             device -> value |= mask; mask <<= 1;
         }
         break;

    case input_stylus: 
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

  // what keycode did we receive?
  switch (keycode)
  {
    case XK_Up:
    case XK_KP_Up:
         e -> data.key.chr = _key_up;
         break;

    case XK_Down:
    case XK_KP_Down:
         e -> data.key.chr = _key_down;
         break;

    case XK_Left:
    case XK_KP_Left:
         e -> data.key.chr = _key_left;
         break;

    case XK_Right:
    case XK_KP_Right:
         e -> data.key.chr = _key_right;
         break;

// NO MAPPING AVAILABLE
//       e -> data.key.chr = _key_select;

    case XK_BackSpace:
         e -> data.key.chr = _key_backspace;
         break;

    case XK_Tab:
    case XK_KP_Tab:
         e -> data.key.chr = _key_tab;
         break;

    case XK_Return:
    case XK_KP_Enter:
         e -> data.key.chr = _key_enter;
         break;

    case XK_Home:
         e -> data.key.chr = _key_home;
         break;

    case XK_End:
         e -> data.key.chr = _key_end;
         break;

    case XK_Page_Up:
         e -> data.key.chr = _key_pageup;
         break;

    case XK_Page_Down:
         e -> data.key.chr = _key_pagedown;
         break;

// NO MAPPING AVAILABLE
//       e -> data.key.chr = _key_scrollup;
//       e -> data.key.chr = _key_scrolldown;

    case XK_F1:
    case XK_KP_F1:
         e -> data.key.chr = _key_help;
         break;

    case XK_F2:
    case XK_KP_F2:
         e -> data.key.chr = _key_menu;
         break;

    case XK_F3:
    case XK_KP_F3:
         e -> data.key.chr = _key_acknowledge;
         break;

    case XK_F4:
    case XK_KP_F4:
         e -> data.key.chr = _key_cancel;
         break;

// WINDOW MANAGER:
//       e -> data.key.chr = _key_focus_gain;
//       e -> data.key.chr = _key_focus_lost;
//       e -> data.key.chr = _key_lcd_resize;
    case XK_F5:
         _SHARKRotateEvent(1);
         break;

    case XK_Escape:
         e -> data.key.chr = _key_escape;
         break;

    case XK_Delete:
    case XK_KP_Delete:
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
  if (penDown != NULL) *penDown = g_pendown;
}

void 
DAL_InputTerminate()
{
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef INPUT_GLOBALS

/********************************* EOF ***********************************/
