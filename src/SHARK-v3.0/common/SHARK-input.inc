/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-input.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

typedef struct InputGlobals
{
  void *struct_prefix;

#if defined(DAL_INPUT_GLOBALS)
  DAL_INPUT_GLOBALS
#endif
} InputGlobals;

GLOBALS_DEFINITION;

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

static boolean DAL_InputInitialize();
static boolean DAL_InputAvailable(input_device type);
static void    DAL_InputState(input *device);
static void    DAL_KeyboardSetVisible(boolean visible);
static void    DAL_KeyHandleEvent(uint16 keycode, event *e);
static uint32  DAL_KeyState(uint16 *keyX, uint16 *keyY);
static void    DAL_PenHandleEvent(int16 penX, int16 penY, event *e);
static void    DAL_PenState(uint16 *penX, uint16 *penY, boolean *penDown);
static void    DAL_InputTerminate();

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
_InputInitialize()
{
  boolean init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _InputInitialize");
#endif

  // allocate memory for globals
  INPUT_GLOBALS = malloc(sizeof(InputGlobals));
  if (GLOBALS_UNINITIALIZED) return false;

  // assume everything is ok
  init_ok =  true;
  init_ok &= DAL_InputInitialize();

  return init_ok;
}

boolean
_InputAvailable(input_device type)
{
  boolean result;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // query the underlying hardware platform
  result = DAL_InputAvailable(type);

  return result;
}

void
_InputState(input *device)
{
  lcd_orientation orientation;
  uint16          tmp;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry requirement
  if (device == NULL) return;

  // query the underlying hardware platform
  DAL_InputState(device);

  // handle display orientation with device
  _LCDGetProperties(NULL, NULL, NULL, NULL, NULL, &orientation, NULL);
  switch (device -> type)
  {
    case input_stylus:
    case input_pointer:
    case input_tablet:

         // apply rotation of (x,y) values
         switch (orientation)
         {
           case lcd_rotate90:
                tmp         = device -> x;
                device -> x = device -> y;
                device -> y = g_width  - tmp;
                break;

           case lcd_rotate180:
                device -> x = g_width  - device -> x;
                device -> y = g_height - device -> y;
                break;

           case lcd_rotate270:
                tmp         = device -> x;
                device -> x = g_height - device -> y;
                device -> y = tmp;
                break;

           default:
           case lcd_rotate0:
                break;
         }
         break;

    case input_dpad:
    case input_joystick:
    case input_analog:

         // apply rotation of (x,y) values
         switch (orientation)
         {
           case lcd_rotate90:
                tmp         =  device -> x;
                device -> x =  device -> y;
                device -> y = -tmp;
                break;

           case lcd_rotate180:
                device -> x = -device -> x;
                device -> y = -device -> y;
                break;

           case lcd_rotate270:
                tmp         =  device -> x;
                device -> x = -device -> y;
                device -> y = tmp;
                break;

           default:
           case lcd_rotate0:
                break;
         }
         break;

    case input_gyroscope:
    case input_accelerometer:
    default:
         break;
  }
}

void
_InputKeyboardSetVisible(boolean visible)
{
  // query the underlying hardware platform
  DAL_KeyboardSetVisible(visible);
}

static void
_KeyHandleEvent(uint16 keycode)
{
  event           e;
  lcd_orientation orientation;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  e.eType        = _keyEvent;
  e.data.key.chr = _key_null;

  // query the underlying hardware platform
  DAL_KeyHandleEvent(keycode, &e);

  // apply screen rotation of co-ordinates
  _LCDGetProperties(NULL, NULL, NULL, NULL, NULL, &orientation, NULL);
  switch (orientation)
  {
    case lcd_rotate90:

if (e.data.key.chr == _key_up)    e.data.key.chr = _key_left;  else
if (e.data.key.chr == _key_down)  e.data.key.chr = _key_right; else
if (e.data.key.chr == _key_left)  e.data.key.chr = _key_down;  else
if (e.data.key.chr == _key_right) e.data.key.chr = _key_up;

         break;

    case lcd_rotate180:

if (e.data.key.chr == _key_up)    e.data.key.chr = _key_down;  else
if (e.data.key.chr == _key_down)  e.data.key.chr = _key_up;    else
if (e.data.key.chr == _key_left)  e.data.key.chr = _key_right; else
if (e.data.key.chr == _key_right) e.data.key.chr = _key_left;

         break;

    case lcd_rotate270:

if (e.data.key.chr == _key_up)    e.data.key.chr = _key_right; else
if (e.data.key.chr == _key_down)  e.data.key.chr = _key_left;  else
if (e.data.key.chr == _key_left)  e.data.key.chr = _key_up;    else
if (e.data.key.chr == _key_right) e.data.key.chr = _key_down;

         break;

    default:
         break;
  }

  // pass the key onto the system for processing
  if (e.data.key.chr != _key_null) _SysEnqueueEvent(&e);
}

uint32
_KeyState(uint16 *keyX, uint16 *keyY)
{
  uint32 result;

  // default return value 
  result = 0;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // query the underlying hardware platform
  result = DAL_KeyState(keyX, keyY);

  return result;
}

static void
_PenHandleEvent(int16 penX, int16 penY)
{
  int16           x, y, tmp;
  boolean         down;
  event           e;
  lcd_orientation orientation;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  x    = penX;
  y    = penY;
  down = ((x >= 0) && (x < g_width) && (y >= 0) && (y < g_height));

  // are we dealing with a valid pen event?
  if (down)
  { 
    // query the underlying hardware platform
    DAL_PenHandleEvent(x, y, &e);

    // apply screen rotation of co-ordinates
    _LCDGetProperties(NULL, NULL, NULL, NULL, NULL, &orientation, NULL);
    switch (orientation)
    {
      case lcd_rotate90:
           tmp = x;
           x   = y;
           y   = g_width - tmp;
           break;

      case lcd_rotate180:
           x   = g_width  - x;
           y   = g_height - y;
           break;

      case lcd_rotate270:
           tmp = x;
           x   = g_height - y;
           y   = tmp;
           break;

      default:
      case lcd_rotate0:
           break;
    }
  
    e.data.pen.x = (uint16)x;
    e.data.pen.y = (uint16)y;
    _SysEnqueueEvent(&e);
  }
}

void
_PenState(uint16 *penX, uint16 *penY, boolean *penDown)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // query the underlying hardware platform
  DAL_PenState(penX, penY, penDown);
}

static void
_InputTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _InputTerminate");
#endif

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  DAL_InputTerminate();

  // free memory for globals
  free(INPUT_GLOBALS); 
  INPUT_GLOBALS = NULL;
}

/********************************* EOF ***********************************/
