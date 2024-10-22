/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-fb.c
 */

#include "../SHARK.h"
#include "../SHARK-fb.h"

/*************************************************************************
 * Configuration
 *************************************************************************/

#define FB_LIBRARY_ID               0x5F66625F // '_fb_'

/*************************************************************************
 * Component Variables
 *************************************************************************/

// none

/*************************************************************************
 * Global Variables
 *************************************************************************/

typedef struct _FBGlobals
{
  fb_type       type;
  uint16        depth;

  void         *bits;              // the display window (used for blitting)
  uint16        width;
  uint16        height;
  uint16        rowBytes;

  void         *LCD_bits;
  uint16        LCD_width;
  uint16        LCD_height;
  uint16        LCD_rowBytes;

  boolean       LCD_direct_draw;   // if lcd == fb (format), direct draw

  void         *extension;

  boolean     (*fnFBInitialize)(void);
  void        (*fnFBTerminate)(void);
  color       (*fnFBGetPaletteIndex)(uint8, uint8, uint8);
  void        (*fnFBGetRGBFromIndex)(color, uint8 *, uint8 *, uint8 *);
  void        (*fnFBCopyToDisplay)(fb_region *, int16, int16);

  void        (*fnFBGetPalette)(uint8 *);  // used only by fb_palette mode
  void        (*fnFBSetPalette)(uint8 *);  // used only by fb_palette mode
  void        (*fnFBDisplayLookup)(void);  // used only by fb_palette mode

} _FBGlobals;

#define FB_GLOBALS_ACCESS \
  _FBGlobals *g_fb = (_FBGlobals *)_LibGetGlobals(FB_LIBRARY_ID);
#define GLOBALS_UNINITIALIZED (g_fb == NULL)

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibGetGlobals() API.
//------------------------------------------------------------------------

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

static void    _FBInitialize_RESIZE();
static boolean _FBDriverConfiguration(fb_format *);
static boolean _FBHandleEvent(event *);
static void    _FBTerminate_RESIZE();

/*************************************************************************
 * Implementation
 *************************************************************************/

boolean
_FBInitialize(fb_format *format)
{
  boolean init_ok;
  FB_GLOBALS_ACCESS;

  // assume everything is ok
  init_ok = true;

  // entry condition
  if ((format == NULL) || (!GLOBALS_UNINITIALIZED)) return false;

  // register the library
  init_ok = _LibRegister(FB_LIBRARY_ID, "_FBLibrary", (void *)_FBHandleEvent);
  if (!init_ok) goto FB_INIT_ABORT;

  // allocate globals memory for the library
  g_fb = (_FBGlobals *)_MemPtrNew(sizeof(_FBGlobals), false);
  init_ok &= (g_fb != NULL); if (!init_ok) goto FB_INIT_ABORT;
  _LibSetGlobals(FB_LIBRARY_ID, (void *)g_fb);

  // initialize the _FB driver
  init_ok = _FBDriverConfiguration(format);
  if (!init_ok) goto FB_INIT_ABORT;

  // initialize _FB internally
  _FBInitialize_RESIZE();

FB_INIT_ABORT:

  return init_ok;
}

static void
_FBInitialize_RESIZE()
{
  lcd_format lcd;
  uint32     size;
  FB_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION
  if (GLOBALS_UNINITIALIZED) return;

  // we need to know the LCD properties
  _LCDGetProperties(&g_fb -> LCD_bits, 
                    &g_fb -> LCD_width, &g_fb -> LCD_height, 
                    &g_fb -> LCD_rowBytes, &lcd, NULL, NULL);

  // the _FB LCD window is the same size as the true LCD window
  g_fb -> width  = g_fb -> LCD_width;
  g_fb -> height = g_fb -> LCD_height;

  // we need to create the LCD window
  if (g_fb -> LCD_direct_draw)
  {
    g_fb -> rowBytes = g_fb -> LCD_rowBytes;

    // SPECIAL: direct LCD hacking
    size = g_fb -> rowBytes * g_fb -> height;
    g_fb -> bits = g_fb -> LCD_bits;
  }
  else
  {
    // row-bytes must be a multiple four to alignment on uint32
    g_fb -> rowBytes = g_fb -> width * (g_fb -> depth >> 3);
    g_fb -> rowBytes = (uint16)((g_fb -> rowBytes + 3) & ~0x3);

    // BUFFER: we need to implement a transitional layer 
    size = g_fb -> rowBytes * g_fb -> height;
    g_fb -> bits = _MemPtrNew(size, false);
  }

  // clear the memory buffer - dont want noise from previous 
  _FBClear();
}

static void
_FBTerminate_RESIZE()
{
  FB_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION
  if (GLOBALS_UNINITIALIZED) return;

  // dispose of the *old* _FB window
  if (!g_fb -> LCD_direct_draw)
    _MemPtrFree(g_fb -> bits);

  g_fb -> bits = NULL;
}

/*************************************************************************
 * _FB GetPaletteIndex implementations
 *************************************************************************/

// 4:4:4 - 16 bit display

static color
_FB_ARGB4444_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  return 0xf000 | FB444GetColor(r,g,b,0);
}

static color
_FB_ABGR4444_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  return 0xf000 | FB444GetColor(b,g,r,0);
}

static color
_FB_RGBA4444_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  return FB444GetColor(r,g,b,4) | 0x000f;
}

static color
_FB_BGRA4444_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  return FB444GetColor(b,g,r,4) | 0x000f;
}

// 5:5:5 - 16 bit display

static color
_FB_ARGB1555_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  return 0x8000 | FB555GetColor(r,g,b,0);
}

static color
_FB_ABGR1555_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  return 0x8000 | FB555GetColor(b,g,r,0);
}

static color
_FB_RGBA5551_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  return FB555GetColor(r,g,b,1) | 0x0001;
}

static color
_FB_BGRA5551_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  return FB555GetColor(b,g,r,1) | 0x0001;
}

// 5:6:5 - 16 bit display

static color
_FB_RGB565_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  return FB565GetColor(r,g,b,0);
}

static color
_FB_BGR565_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  return FB565GetColor(b,g,r,0);
}

// 8:8:8 - 32 bit display

static color
_FB_ARGB8888_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  return 0xff000000 | FB888GetColor(r,g,b,0);
}

static color
_FB_ABGR8888_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  return 0xff000000 | FB888GetColor(b,g,r,0);
}

static color
_FB_RGBA8888_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  return FB888GetColor(r,g,b,8) | 0x000000ff;
}

static color
_FB_BGRA8888_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  return FB888GetColor(b,g,r,8) | 0x000000ff;
}

/*************************************************************************
 * _FB GetRGBFromIndex implementations
 *************************************************************************/

// 4:4:4 - 16 bit display

static void
_FB_4444_GetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  FBColorGet444(c,*r,*g,*b,0);
}

// 5:5:5 - 16 bit display

static void
_FB_1555_GetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  FBColorGet555(c,*r,*g,*b,0);
}

// 5:6:5 - 16 bit display

static void
_FB_565_GetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  FBColorGet565(c,*r,*g,*b,0);
}

#include "SHARK-fb-i8.inc"
#include "SHARK-fb-d16.inc"

static boolean
_FBDriverConfiguration(fb_format *format)
{
  boolean     result;
  lcd_format  lcd;
  uint32      flag;
  FB_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION
  if (GLOBALS_UNINITIALIZED) return false;

  // default return value
  result = false;

  // we need to know the LCD properties (just want lcd_format)
  _LCDGetProperties(NULL, NULL, NULL, NULL, &lcd, NULL, NULL);

  // ARGB = 0x00, ABGR = 0x01, RGBA = 0x02, BGRA = 0x03
  flag = 0;
  if  (lcd.data.direct.b_mask > lcd.data.direct.r_mask) flag |= 0x01;
  if ((lcd.data.direct.a_mask & 0x01) == 0x01)          flag |= 0x02;

  g_fb -> LCD_direct_draw = false;

  switch (format -> type)
  {
    case fb_palette:

         // whats the requested depth/palette size
         switch (format -> depth)
         {
           case 8:

  // initialize the _FB routines
  g_fb -> fnFBInitialize       = (void *)_FB_I8_Initialize;
  g_fb -> fnFBTerminate        = (void *)_FB_I8_Terminate;
  g_fb -> fnFBGetPalette       = (void *)_FB_I8_GetPalette;
  g_fb -> fnFBGetPaletteIndex  = (void *)_FB_I8_GetPaletteIndex;
  g_fb -> fnFBGetRGBFromIndex  = (void *)_FB_I8_GetRGBFromIndex;
  g_fb -> fnFBSetPalette       = (void *)_FB_I8_SetPalette;

                switch (lcd.type)
                {
                  case lcd_grayscale:
                       break;

                  case lcd_direct:
                       if ((lcd.depth_bus         == 16) &&
                           (lcd.data.direct.red   == 4) &&
                           (lcd.data.direct.green == 4) &&
                           (lcd.data.direct.blue  == 4))
                       {
  if (flag == 0x00)
    g_fb -> fnFBDisplayLookup = (void *)_FB_I8_ARGB4444_DisplayLookup;
  else if (flag == 0x01)
    g_fb -> fnFBDisplayLookup = (void *)_FB_I8_ABGR4444_DisplayLookup;
  else if (flag == 0x02)
    g_fb -> fnFBDisplayLookup = (void *)_FB_I8_RGBA4444_DisplayLookup;
  else if (flag == 0x03)
    g_fb -> fnFBDisplayLookup = (void *)_FB_I8_BGRA4444_DisplayLookup;

  g_fb -> fnFBCopyToDisplay = (void *)_FB_I8_D16_CopyToDisplay;
                       }
                       else
                       if ((lcd.depth_bus         == 16) &&
                           (lcd.data.direct.red   == 5) &&
                           (lcd.data.direct.green == 5) &&
                           (lcd.data.direct.blue  == 5))
                       {
  if (flag == 0x00)
    g_fb -> fnFBDisplayLookup = (void *)_FB_I8_ARGB1555_DisplayLookup;
  else if (flag == 0x01)
    g_fb -> fnFBDisplayLookup = (void *)_FB_I8_ABGR1555_DisplayLookup;
  else if (flag == 0x02)
    g_fb -> fnFBDisplayLookup = (void *)_FB_I8_RGBA5551_DisplayLookup;
  else if (flag == 0x03)
    g_fb -> fnFBDisplayLookup = (void *)_FB_I8_BGRA5551_DisplayLookup;

  g_fb -> fnFBCopyToDisplay = (void *)_FB_I8_D16_CopyToDisplay;
                       }
                       else
                       if ((lcd.depth_bus         == 16) &&
                           (lcd.data.direct.red   == 5) &&
                           (lcd.data.direct.green == 6) &&
                           (lcd.data.direct.blue  == 5))
                       {
  if (flag == 0x00)
    g_fb -> fnFBDisplayLookup = (void *)_FB_I8_RGB565_DisplayLookup;
  else if (flag == 0x01)
    g_fb -> fnFBDisplayLookup = (void *)_FB_I8_BGR565_DisplayLookup;

  g_fb -> fnFBCopyToDisplay = (void *)_FB_I8_D16_CopyToDisplay;
                       }
                       else
                       if ((lcd.depth_bus         == 32) &&
                           (lcd.data.direct.red   == 8) &&
                           (lcd.data.direct.green == 8) &&
                           (lcd.data.direct.blue  == 8))
                       {
  if (flag == 0x00)
    g_fb -> fnFBDisplayLookup = (void *)_FB_I8_ARGB8888_DisplayLookup;
  else if (flag == 0x01)
    g_fb -> fnFBDisplayLookup = (void *)_FB_I8_ABGR8888_DisplayLookup;
  else if (flag == 0x02)
    g_fb -> fnFBDisplayLookup = (void *)_FB_I8_RGBA8888_DisplayLookup;
  else if (flag == 0x03)
    g_fb -> fnFBDisplayLookup = (void *)_FB_I8_BGRA8888_DisplayLookup;

  g_fb -> fnFBCopyToDisplay = (void *)_FB_I8_D32_CopyToDisplay;
                       }
                       break;
                }
                break;

           default:
                break;
         }
         break;

    case fb_direct:

         if (format -> depth == 16)
         {
  // initialize the _FB routines
  g_fb -> fnFBInitialize      = (void *)_FB_D16_Initialize;
  g_fb -> fnFBTerminate       = (void *)_FB_D16_Terminate;

           if ((format -> data.direct.red   == 4) &&
               (format -> data.direct.green == 4) &&
               (format -> data.direct.blue  == 4))
           {
  g_fb -> fnFBGetPaletteIndex = (void *)_FB_ARGB4444_GetPaletteIndex;
  g_fb -> fnFBGetRGBFromIndex = (void *)_FB_4444_GetRGBFromIndex;

             switch (lcd.type)
             {
               case lcd_grayscale:
                    break;

               case lcd_direct:
                    if ((lcd.depth_bus         == 16) &&
                        (lcd.data.direct.red   == 4) &&
                        (lcd.data.direct.green == 4) &&
                        (lcd.data.direct.blue  == 4))
                    {
  if (flag == 0x00)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_4444_ARGB4444_CopyToDisplay;
  else if (flag == 0x01)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_4444_ABGR4444_CopyToDisplay;
  else if (flag == 0x02)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_4444_RGBA4444_CopyToDisplay;
  else if (flag == 0x03)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_4444_BGRA4444_CopyToDisplay;
                    }
                    else
                    if ((lcd.depth_bus         == 16) &&
                        (lcd.data.direct.red   == 5) &&
                        (lcd.data.direct.green == 5) &&
                        (lcd.data.direct.blue  == 5))
                    {
  if (flag == 0x00)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_4444_ARGB1555_CopyToDisplay;
  else if (flag == 0x01)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_4444_ABGR1555_CopyToDisplay;
  else if (flag == 0x02)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_4444_RGBA5551_CopyToDisplay;
  else if (flag == 0x03)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_4444_BGRA5551_CopyToDisplay;
                    }
                    else
                    if ((lcd.depth_bus         == 16) &&
                        (lcd.data.direct.red   == 5) &&
                        (lcd.data.direct.green == 6) &&
                        (lcd.data.direct.blue  == 5))
                    {
  if (flag == 0x00)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_4444_RGB565_CopyToDisplay;
  else if (flag == 0x01)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_4444_BGR565_CopyToDisplay;
                    }
                    else
                    if ((lcd.depth_bus         == 32) &&
                        (lcd.data.direct.red   == 8) &&
                        (lcd.data.direct.green == 8) &&
                        (lcd.data.direct.blue  == 8))
                    {
  if (flag == 0x00)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_4444_ARGB8888_CopyToDisplay;
  else if (flag == 0x01)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_4444_ABGR8888_CopyToDisplay;
  else if (flag == 0x02)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_4444_RGBA8888_CopyToDisplay;
  else if (flag == 0x03)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_4444_BGRA8888_CopyToDisplay;
                    }
                    break;
             }
           }
           else
           if ((format -> data.direct.red   == 5) &&
               (format -> data.direct.green == 5) &&
               (format -> data.direct.blue  == 5))
           {
  g_fb -> fnFBGetPaletteIndex = (void *)_FB_ARGB1555_GetPaletteIndex;
  g_fb -> fnFBGetRGBFromIndex = (void *)_FB_1555_GetRGBFromIndex;

             switch (lcd.type)
             {
               case lcd_grayscale:
                    break;

               case lcd_direct:
                    if ((lcd.depth_bus         == 16) &&
                        (lcd.data.direct.red   == 4) &&
                        (lcd.data.direct.green == 4) &&
                        (lcd.data.direct.blue  == 4))
                    {
  if (flag == 0x00)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_1555_ARGB4444_CopyToDisplay;
  else if (flag == 0x01)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_1555_ABGR4444_CopyToDisplay;
  else if (flag == 0x02)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_1555_RGBA4444_CopyToDisplay;
  else if (flag == 0x03)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_1555_BGRA4444_CopyToDisplay;
                    }
                    else
                    if ((lcd.depth_bus         == 16) &&
                        (lcd.data.direct.red   == 5) &&
                        (lcd.data.direct.green == 5) &&
                        (lcd.data.direct.blue  == 5))
                    {
  if (flag == 0x00)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_1555_ARGB1555_CopyToDisplay;
  else if (flag == 0x01)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_1555_ABGR1555_CopyToDisplay;
  else if (flag == 0x02)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_1555_RGBA5551_CopyToDisplay;
  else if (flag == 0x03)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_1555_BGRA5551_CopyToDisplay;
                    }
                    else
                    if ((lcd.depth_bus         == 16) &&
                        (lcd.data.direct.red   == 5) &&
                        (lcd.data.direct.green == 6) &&
                        (lcd.data.direct.blue  == 5))
                    {
  if (flag == 0x00)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_1555_RGB565_CopyToDisplay;
  else if (flag == 0x01)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_1555_BGR565_CopyToDisplay;
                    }
                    else
                    if ((lcd.depth_bus         == 32) &&
                        (lcd.data.direct.red   == 8) &&
                        (lcd.data.direct.green == 8) &&
                        (lcd.data.direct.blue  == 8))
                    {
  if (flag == 0x00)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_1555_ARGB8888_CopyToDisplay;
  else if (flag == 0x01)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_1555_ABGR8888_CopyToDisplay;
  else if (flag == 0x02)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_1555_RGBA8888_CopyToDisplay;
  else if (flag == 0x03)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_1555_BGRA8888_CopyToDisplay;
                    }
                    break;
             }
           }
           else
           if ((format -> data.direct.red   == 5) &&
               (format -> data.direct.green == 6) &&
               (format -> data.direct.blue  == 5))
           {
  g_fb -> fnFBGetPaletteIndex = (void *)_FB_RGB565_GetPaletteIndex;
  g_fb -> fnFBGetRGBFromIndex = (void *)_FB_565_GetRGBFromIndex;

             switch (lcd.type)
             {
               case lcd_grayscale:
                    break;

               case lcd_direct:
                    if ((lcd.depth_bus         == 16) &&
                        (lcd.data.direct.red   == 4) &&
                        (lcd.data.direct.green == 4) &&
                        (lcd.data.direct.blue  == 4))
                    {
  if (flag == 0x00)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_565_ARGB4444_CopyToDisplay;
  else if (flag == 0x01)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_565_ABGR4444_CopyToDisplay;
  else if (flag == 0x02) 
    g_fb -> fnFBCopyToDisplay = (void *)_FB_565_RGBA4444_CopyToDisplay;
  else if (flag == 0x03)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_565_BGRA4444_CopyToDisplay;
                    }
                    else
                    if ((lcd.depth_bus         == 16) &&
                        (lcd.data.direct.red   == 5) &&
                        (lcd.data.direct.green == 5) &&
                        (lcd.data.direct.blue  == 5))
                    {
  if (flag == 0x00)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_565_ARGB1555_CopyToDisplay;
  else if (flag == 0x01)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_565_ABGR1555_CopyToDisplay;
  else if (flag == 0x02)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_565_RGBA5551_CopyToDisplay;
  else if (flag == 0x03)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_565_BGRA5551_CopyToDisplay;
                    }
                    else
                    if ((lcd.depth_bus         == 16) &&
                        (lcd.data.direct.red   == 5) &&
                        (lcd.data.direct.green == 6) &&
                        (lcd.data.direct.blue  == 5))
                    {
  if (flag == 0x00)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_D16_D16_CopyToDisplay;
  else if (flag == 0x01)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_565_BGR565_CopyToDisplay;
                    }
                    else
                    if ((lcd.depth_bus         == 32) &&
                        (lcd.data.direct.red   == 8) &&
                        (lcd.data.direct.green == 8) &&
                        (lcd.data.direct.blue  == 8))
                    {
  if (flag == 0x00)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_565_ARGB8888_CopyToDisplay;
  else if (flag == 0x01)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_565_ABGR8888_CopyToDisplay;
  else if (flag == 0x02)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_565_RGBA8888_CopyToDisplay;
  else if (flag == 0x03)
    g_fb -> fnFBCopyToDisplay = (void *)_FB_565_BGRA8888_CopyToDisplay;
                    }
                    break;
             }
           }

           g_fb -> fnFBGetPalette    = NULL;
           g_fb -> fnFBSetPalette    = NULL;
           g_fb -> fnFBDisplayLookup = NULL;
         }
         break;

    default:
         break;
  }

  // we need these values for later
  g_fb -> type  = format -> type;
  g_fb -> depth = format -> depth;

  // this is confirmation that we support this mode
  result = (g_fb -> fnFBCopyToDisplay != NULL);

  // if fb == LCD - we can draw direct to LCD offscreen buffer
  if ((lcd.depth_bus         == format -> depth            ) &&
      (lcd.data.direct.red   == format -> data.direct.red  ) &&
      (lcd.data.direct.green == format -> data.direct.green) &&
      (lcd.data.direct.blue  == format -> data.direct.blue ))
  {
    // _FBCopyToDisplay:: = color transformation, can optimize out
    g_fb -> LCD_direct_draw = (flag == 0x00); // xRGB supported only
  }

  // initialize the sub-graphics layer
  if (result && (g_fb -> fnFBInitialize != NULL))
    result = g_fb -> fnFBInitialize();

  return result;
}

static boolean
_FBHandleEvent(event *e)
{
  boolean result;
  FB_GLOBALS_ACCESS;

  // default return value
  result = false;

  // GLOBALS_PROTECTION
  if (GLOBALS_UNINITIALIZED) return result;

  switch (e -> eType)
  {
    case _keyEvent:
         switch (e -> data.key.chr)
         {
           case _key_lcd_resize:
                _FBTerminate_RESIZE();
                _FBInitialize_RESIZE();

                result = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return result;
}

void
_FBTerminate()
{
  FB_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION
  if (GLOBALS_UNINITIALIZED) return;

  // terminate _FB internally
  _FBTerminate_RESIZE();

  // shutdown the sub-graphics layer
  if (g_fb -> fnFBTerminate != NULL) 
    g_fb -> fnFBTerminate();

  // free the globals used
  _MemPtrFree(g_fb); g_fb = NULL;
  _LibSetGlobals(FB_LIBRARY_ID, (void *)g_fb);

  // shutdown the library
  _LibShutdown(FB_LIBRARY_ID);
}

void
_FBClear()
{
  uint32 size;
  FB_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION
  if (GLOBALS_UNINITIALIZED) return;

  size = g_fb -> rowBytes * g_fb -> height;
  switch (g_fb -> type)
  {
    case fb_palette: _MemSet(g_fb -> bits, 0x00, size); break;
    case fb_direct:  _MemSet(g_fb -> bits, 0xff, size); break;
  }
}

void
_FBGetPalette(uint8 *rgb)
{
  FB_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION
  if (GLOBALS_UNINITIALIZED) return;


  // execute the sub-graphics layer command
  if (g_fb -> fnFBGetPalette != NULL)
    g_fb -> fnFBGetPalette(rgb);
}

color
_FBGetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  color result;
  FB_GLOBALS_ACCESS;

  // default return value
  result = 0;

  // GLOBALS_PROTECTION
  if (GLOBALS_UNINITIALIZED) return result;

  // execute the sub-graphics layer command
  if (g_fb -> fnFBGetPaletteIndex != NULL)
    result = g_fb -> fnFBGetPaletteIndex(r, g, b);

  return result;
}

void
_FBGetProperties(void **bits, uint16 *width, uint16 *height, uint16 *rowBytes)
{
  FB_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION
  if (GLOBALS_UNINITIALIZED) return;

  if (bits     != NULL) *bits     = g_fb -> bits;
  if (width    != NULL) *width    = g_fb -> width;
  if (height   != NULL) *height   = g_fb -> height;
  if (rowBytes != NULL) *rowBytes = g_fb -> rowBytes;
}

void
_FBGetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  FB_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION
  if (GLOBALS_UNINITIALIZED) return;

  // entry requirement 
  if ((r == NULL) || (g == NULL) || (b == NULL)) return;

  // execute the sub-graphics layer command
  if (g_fb -> fnFBGetRGBFromIndex != NULL)
    g_fb -> fnFBGetRGBFromIndex(c, r, g, b);
}

void
_FBRepaint()
{
  fb_region region;
  FB_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION
  if (GLOBALS_UNINITIALIZED) return;

  // define the repaint boundaries
  region.x      = 0;
  region.y      = 0;
  region.width  = g_fb -> width;
  region.height = g_fb -> height;

  // do the repaint
  _FBRepaintRegion(&region);
}

void
_FBRepaintRegion(fb_region *region)
{
  FB_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION
  if (GLOBALS_UNINITIALIZED) return;

  // entry requirement 
  if (region == NULL) return;

  // copy fb -> LCD with color conversion logic
  if ((!g_fb -> LCD_direct_draw) && (g_fb -> fnFBCopyToDisplay != NULL))
    g_fb -> fnFBCopyToDisplay(region, region -> x, region -> y);

  // repaint the LCD region with applying rotation
  _LCDLock();
  _LCDRepaintRegion((lcd_region *)region);
  _LCDUnlock();

  // ensure the latest LCD bits are used :0
  _LCDGetProperties(&g_fb -> LCD_bits, NULL, NULL, NULL, NULL, NULL, NULL);
  if (g_fb -> LCD_direct_draw) g_fb -> bits = g_fb -> LCD_bits;
}

void
_FBSetPalette(uint8 *rgb)
{
  FB_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION
  if (GLOBALS_UNINITIALIZED) return;

  // execute the sub-graphics layer command
  if (g_fb -> fnFBSetPalette != NULL)
    g_fb -> fnFBSetPalette(rgb);
}

#undef GLOBALS_UNINITIALIZED
#undef FB_GLOBALS_ACCESS

#undef FB_LIBRARY_ID

/********************************* EOF ***********************************/
