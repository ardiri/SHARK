/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-display.c
 */

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct
{
  boolean           lcd_direct_access;

  WinHandle         lcd_win;

  void             *lcd_bits;          // bitmap buffer pointer
  void             *lcd_off_bits;      // offscreen window (drawing)

  lcd_orientation   lcd_orientation;
  lcd_dpi           lcd_dpi;
} LCDGlobals;

#define g_lcd_direct_access   ((LCDGlobals *)g -> lcd) -> lcd_direct_access

#define g_lcd_win             ((LCDGlobals *)g -> lcd) -> lcd_win

#define g_lcd_bits            ((LCDGlobals *)g -> lcd) -> lcd_bits
#define g_lcd_off_bits        ((LCDGlobals *)g -> lcd) -> lcd_off_bits

#define g_lcd_orientation     ((LCDGlobals *)g -> lcd) -> lcd_orientation
#define g_lcd_dpi             ((LCDGlobals *)g -> lcd) -> lcd_dpi

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static void _LCDInitialize_RESIZE();
static void _LCDTerminate_RESIZE();

// initialize the LCD display driver
static boolean
_LCDInitialize()
{
  boolean     init_ok;
  LCDGlobals *g_lib;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _LCDInitialize");
#endif

  // allocate memory for lcd globals
  g_lib = (LCDGlobals *)malloc(sizeof(LCDGlobals));
  if (g_lib != NULL) g -> lcd = (void *)g_lib;

  // assume everything is ok
  init_ok  = true;
  init_ok &= (g_lib != NULL);

  if (init_ok)
  {
    // lets determine the DPI of the display
    switch (g -> ftr.scr_density)
    {
      case kDensityDouble:      g_lcd_dpi = lcd_dpi144; break;
      case kDensityOneAndAHalf: g_lcd_dpi = lcd_dpi96;  break;

      default:
      case kDensityLow:         g_lcd_dpi = lcd_dpi72;  break;
    }

    // lets first assume, we should use proper API
    g_lcd_direct_access = false; 
    switch (g -> ftr.comID)
    {
      case 'palm':
      case 'Palm': // Palm
           g_lcd_direct_access |= (g -> ftr.devID == 'Frg1'); // T|T
           g_lcd_direct_access |= (g -> ftr.devID == 'Frg2'); // T|T2
           g_lcd_direct_access |= (g -> ftr.devID == 'MT64'); // T|C
           g_lcd_direct_access |= (g -> ftr.devID == 'Cct1'); // T|E
           g_lcd_direct_access |= (g -> ftr.devID == 'Zir4'); // T|E2
           g_lcd_direct_access |= (g -> ftr.devID == 'D051'); // Zire 22
           g_lcd_direct_access |= (g -> ftr.devID == 'Zi22'); // Zire 31
           g_lcd_direct_access |= (g -> ftr.devID == 'Zpth'); // Zire 71
           g_lcd_direct_access |= (g -> ftr.devID == 'Zi72'); // Zire 72
           break;

      case 'hspr': // HandSpring
           g_lcd_direct_access |= (g -> ftr.devID == 'H101'); // treo 600
           g_lcd_direct_access |= (g -> ftr.devID == 'H102'); // treo 650
           break;

      case 'grmn': // Garmin
           g_lcd_direct_access |= (g -> ftr.devID == '3000'); // iQue 3000
           g_lcd_direct_access |= (g -> ftr.devID == '3200'); // iQue 3200
           break;

      case 'sony': // Sony
           g_lcd_direct_access |= (g -> ftr.devID == 'goha'); // Clie TJ25
           g_lcd_direct_access |= (g -> ftr.devID == 'leia'); // Clie TJ27
           g_lcd_direct_access |= (g -> ftr.devID == 'goku'); // Clie TJ35
           g_lcd_direct_access |= (g -> ftr.devID == 'luke'); // Clie TJ37
           g_lcd_direct_access |= (g -> ftr.devID == 'vrna'); // Clie TG50
           break;

      default:
           break;
    }

    // initialize the window to the current size
    _LCDInitialize_RESIZE();

#if (LOG_DISABLE == 0)
    // LOGGING EVENT
    {
      char str_log[256];
      int dpi[4] = { 72, 96, 144, 192 };

      sprintf(str_log, "\n\tresolution\t:: %3dx%3d", g_width, g_height);
      _LOGCreateEntry(str_log);
      sprintf(str_log, "\tdpi (dots/in)\t:: %d\n", dpi[g_lcd_dpi]);
      _LOGCreateEntry(str_log);
    }
#endif
  }

  return init_ok;
}

static void
_LCDSuspend()
{
  // palmos - no suspend option, handled by window manager
}

static void
_LCDResume()
{
  // palmos - no resume option, handled by window manager
}

// set the display orientation, forced _key_gfx_change event
void
_LCDSetOrientation(lcd_orientation orientation)
{
  event e;
  
  // save this for when we need to redraw to the LCD
  g_lcd_orientation = orientation;
  
  // now, the application must handle the new LCD instance
  if (g_initialized)
  {
    memset(&e, 0, sizeof(event));
    e.eType        = _keyEvent;
    e.data.key.chr = _key_gfx_change;
    DAL_HandleEvent(&e);
  }
    
  // redraw the display
  _LCDRepaint();
}

// get the properties of the LCD display controller
void
_LCDGetProperties(void **lcd, coord *width, coord *height, uint16 *row_bytes, 
                  lcd_format *format, lcd_orientation *orientation, lcd_dpi *dpi)
{
  if (lcd != NULL)
  {
    *lcd = (void *)g_lcd_off_bits;
  }
  if (format != NULL)
  {
    // clear the format structure
    memset(format, 0, sizeof(format));

    format -> type                = lcd_direct;
    format -> depth               = LCD_DEPTH;
    format -> data.direct.alpha   = 0;
    format -> data.direct.red     = g_lcd_r;
    format -> data.direct.green   = g_lcd_g;
    format -> data.direct.blue    = g_lcd_b;

    format -> data.direct.a_mask  = 0x00000000;
    format -> data.direct.r_mask  = g_lcd_r_mask;
    format -> data.direct.g_mask  = g_lcd_g_mask;
    format -> data.direct.b_mask  = g_lcd_b_mask;
  }
  if (orientation != NULL) *orientation = g_lcd_orientation;
  if (dpi         != NULL) *dpi         = g_lcd_dpi;

  // default:: landscape
  if ((g_lcd_orientation == lcd_rotate0) || 
      (g_lcd_orientation == lcd_rotate180))
  {
    if (width     != NULL) *width     = g_width;
    if (height    != NULL) *height    = g_height;
    if (row_bytes != NULL) *row_bytes = g_width_rb;
  }
  // default:: portrait
  else
  {
    if (width     != NULL) *width     = g_height;
    if (height    != NULL) *height    = g_width;
    if (row_bytes != NULL) *row_bytes = g_height_rb;
  }
}

// set the palette of the LCD display controller
void
_LCDPalette(boolean mode, uint8 *palette)
{
  // palmos doesn't run in paletted mode
}

// repaint a specific region from offscreen to the LCD display
void
_LCDRepaintRegion(rectangle *rect)
{
  uint16       *pS, *pD;
  uint16        rw,  rh;
  uint32        rw_h;
  int           i, j;
  RectangleType _rect;

  // lock the display, get access to screen bitmap
  if (g_lcd_direct_access)
    g_lcd_bits = (uint16 *)BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));
  else
    g_lcd_bits = (uint16 *)BmpGetBits(WinGetBitmap(g_lcd_win));

  // SPECIAL: we have nothing to draw to!
  if (g_lcd_bits == NULL) return;

  // default:: standard mode
  switch (g_lcd_orientation)
  {
    case lcd_rotate0:

         // ensure boundary clipping
         if  (rect -> x > g_width)  rect -> x = g_width;
         if  (rect -> y > g_height) rect -> y = g_height;
         if ((rect -> x + rect -> width)  > g_width)  rect -> width  = g_width  - rect -> x;
         if ((rect -> y + rect -> height) > g_height) rect -> height = g_height - rect -> y;

         rw = (uint16)(g_width_rb / (LCD_DEPTH >> 3));
     
         // initialize pointers         
         pS = (uint16 *)g_lcd_off_bits + (rect -> y * rw) + rect -> x;
         pD = (uint16 *)g_lcd_bits     + (rect -> y * rw) + rect -> x;

         i = rect -> height;
         while (i--)
         {
           j = rect -> width;
           while (j--)
           {
             *pD++ = *pS++;
           }
           pD += (rw - rect -> width);
           pS += (rw - rect -> width); 
         }

         if (!g_lcd_direct_access)
         {
           _rect.topLeft.x = rect -> x;
           _rect.topLeft.y = rect -> y;
           _rect.extent.x  = rect -> width;
           _rect.extent.y  = rect -> height;

           WinSetCoordinateSystem(kCoordinatesNative);
           WinCopyRectangle(g_lcd_win, WinGetDisplayWindow(),
                            &_rect, rect -> x, rect -> y, winPaint);
           WinSetCoordinateSystem(kCoordinatesStandard);
         }
         break;

    case lcd_rotate90:

         // ensure boundary clipping
         if  (rect -> y > g_width)  rect -> y = g_width;
         if  (rect -> x > g_height) rect -> x = g_height;
         if ((rect -> y + rect -> height) > g_width)  rect -> height = g_width  - rect -> y;
         if ((rect -> x + rect -> width)  > g_height) rect -> width  = g_height - rect -> x;

         // ensure we are on a 4-byte boundary
         rw   = (uint16)(g_height_rb / (LCD_DEPTH >> 3));
         rh   = (uint16)(g_width_rb  / (LCD_DEPTH >> 3));
         rw_h = rect -> height * rw;  // optimization
         
         // initialize pointers         
         pD  = (uint16 *)g_lcd_bits     + ((rh - rect -> y - 1) + (rect -> x * rh));
         pS  = (uint16 *)g_lcd_off_bits + (rect -> x + (rect -> y * rw));
         if (g_width & 3) pD -= (4 - (g_width & 3));

         i = rect -> width;
         while (i--)
         {
           j = rect -> height;
           while (j--)
           {
             *pD-- = *pS;
              pS += rw;
           }
           pS -= (rw_h - 1);
           pD += (rect -> height + rh);
         }

         if (!g_lcd_direct_access)
         {
           _rect.topLeft.x = rect -> y;
           _rect.topLeft.y = rect -> x;
           _rect.extent.x  = rect -> height;
           _rect.extent.y  = rect -> width;

           WinSetCoordinateSystem(kCoordinatesNative);
           WinCopyRectangle(g_lcd_win, WinGetDisplayWindow(),
                            &_rect, rect -> y, rect -> x, winPaint);
           WinSetCoordinateSystem(kCoordinatesStandard);
         }
         break;

    case lcd_rotate180:

         // ensure boundary clipping
         if  (rect -> x > g_width)                  rect -> x      = g_width;
         if  (rect -> y > g_height)                 rect -> y      = g_height;
         if ((rect -> x + rect -> width)  > g_width)  rect -> width  = g_width  - rect -> x;
         if ((rect -> y + rect -> height) > g_height) rect -> height = g_height - rect -> y;

         // ensure we are on a 4-byte boundary
         rw = (uint16)(g_width_rb / (LCD_DEPTH >> 3));

         // initialize pointers         
         pS = (uint16 *)g_lcd_off_bits + ((rect -> y + rect -> height) * rw) + rect -> x - 1;
         pD = (uint16 *)g_lcd_bits     +  (rect -> y                 * rw) + rect -> x;

         // offset for wider screens (due to odd sized screen)
         pS -= (rw - rect -> width);
         
         i = rect -> height;
         while (i--)
         {
           j = rect -> width;
           while (j--)
           {
             *pD++ = *pS--;
           }
           pD += (rw - rect -> width);
           pS -= (rw - rect -> width); 
         }

         if (!g_lcd_direct_access)
         {
           _rect.topLeft.x = rect -> x;
           _rect.topLeft.y = rect -> y;
           _rect.extent.x  = rect -> width;
           _rect.extent.y  = rect -> height;

           WinSetCoordinateSystem(kCoordinatesNative);
           WinCopyRectangle(g_lcd_win, WinGetDisplayWindow(),
                            &_rect, rect -> x, rect -> y, winPaint);
           WinSetCoordinateSystem(kCoordinatesStandard);
         }
         break;

    case lcd_rotate270:

         // ensure boundary clipping
         if  (rect -> y > g_width)                  rect -> y      = g_width;
         if  (rect -> x > g_height)                 rect -> x      = g_height;
         if ((rect -> y + rect -> height) > g_width)  rect -> height = g_width  - rect -> y;
         if ((rect -> x + rect -> width)  > g_height) rect -> width  = g_height - rect -> x;

         // ensure we are on a 4-byte boundary
         rw   = (uint16)(g_height_rb / (LCD_DEPTH >> 3));
         rh   = (uint16)(g_width_rb  / (LCD_DEPTH >> 3));
         rw_h = rect -> height * rw;  // optimization

         // initialize pointers         
         pD  = (uint16 *)g_lcd_bits     + (rect -> y + ((rw - rect -> x - 1) * rh));
         pS  = (uint16 *)g_lcd_off_bits + (rect -> x + (rect -> y * rw));
         if (g_height & 3) pD -= ((4 - (g_height & 3)) * rh);

         i = rect -> width;
         while (i--)
         { 
           j = rect -> height;
           while (j--)
           {
             *pD++ = *pS;
              pS += rw;
           }
           pS -= (rw_h - 1);
           pD -= (rect -> height + rh);
         }

         if (!g_lcd_direct_access)
         {
           _rect.topLeft.x = rect -> y;
           _rect.topLeft.y = rect -> x;
           _rect.extent.x  = rect -> height;
           _rect.extent.y  = rect -> width;

           WinSetCoordinateSystem(kCoordinatesNative);
           WinCopyRectangle(g_lcd_win, WinGetDisplayWindow(),
                            &_rect, rect -> y, rect -> x, winPaint);
           WinSetCoordinateSystem(kCoordinatesStandard);
         }
         break;

    default:
         break;
  }
}

// terminate the LCD display driver
static void
_LCDTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _LCDTerminate");
#endif

  // release memory used by the display driver
  _LCDTerminate_RESIZE();

  // free memory used by library globals
  if (g -> lcd != NULL) free(g -> lcd);
  g -> lcd = NULL;
}

// repaint the entire area used by the LCD display controller
void
_LCDRepaint()
{
  rectangle rect;

  rect.x = 0;
  rect.y = 0;
  _LCDGetProperties(NULL, &rect.width, &rect.height, NULL, NULL, NULL, NULL);
  _LCDRepaintRegion(&rect);
}

static void
_LCDInitialize_RESIZE()
{
  uint32 size;
  Err    err;

  // allocate the offscreen window (for use with rotation of display)
  size = ((uint32)g_width_rb * g_height_rb / (LCD_DEPTH >> 3));
  g_lcd_off_bits = malloc(size);
  if (g_lcd_off_bits != NULL)
    memset(g_lcd_off_bits, 0xff, size);  // white window (should copy to lcd)

  // create the base bitmap
  if (!g_lcd_direct_access)
  {
    WinSetCoordinateSystem(kCoordinatesNative);
    g_lcd_win = WinCreateOffscreenWindow(g_width, g_height, nativeFormat, &err);
    WinSetCoordinateSystem(kCoordinatesStandard);
  }
}

static void
_LCDTerminate_RESIZE()
{
  // dispose of all the bitmap windows
  if (!g_lcd_direct_access)
  {
    WinDeleteWindow(g_lcd_win, false);
    g_lcd_win = NULL;
  }

  // delete offscreen window
  if (g_lcd_off_bits) free(g_lcd_off_bits);
  g_lcd_off_bits = NULL;
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-display.inc"

/********************************* EOF ***********************************/
