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

static PixMapHandle      g_lcd_pm;

static void             *g_lcd_bits;          // bitmap buffer pointer
static void             *g_lcd_off_bits;      // offscreen window (drawing)

static lcd_orientation   g_lcd_orientation;
static lcd_dpi           g_lcd_dpi;

static void            (*g_LCDRepaintRegion)(rectangle *rect);

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static void _LCDInitialize_RESIZE();
static void _LCDTerminate_RESIZE();

static void _LCDRepaintRegion16(rectangle *rect);
static void _LCDRepaintRegion32(rectangle *rect);

// initialize the LCD display driver
static boolean
_LCDInitialize()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _LCDInitialize");
#endif

  // lets determine the DPI of the display
  g_lcd_dpi = lcd_dpi72;

  if (LCD_DEPTH == 16)
    g_LCDRepaintRegion = _LCDRepaintRegion16;
  else
    g_LCDRepaintRegion = _LCDRepaintRegion32;

  // initialize the window to the current size
  _LCDInitialize_RESIZE();

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  {
    char str_log[256];
    int  dpi[4] = { 72, 96, 144, 192 };

    sprintf(str_log, "\n\tresolution\t:: %3dx%3d", g_width, g_height);
    _LOGCreateEntry(str_log);
    sprintf(str_log, "\tdpi (dots/in)\t:: %d\n", dpi[g_lcd_dpi]);
    _LOGCreateEntry(str_log);
  }
#endif

  return true;
}

static void
_LCDSuspend()
{
  // macosx - no suspend option, handled by window manager
}

static void
_LCDResume()
{
  // macosx - no resume option, handled by window manager
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
    if (g_lcd_orientation == lcd_rotate0) *lcd = (void *)g_lcd_bits;
    else                                  *lcd = (void *)g_lcd_off_bits;
  }
  if (format != NULL)
  {
    // clear the format structure
    memset(format, 0, sizeof(format));

    format->type                = lcd_direct;
    format->depth               = LCD_DEPTH;
    format->data.direct.alpha   = 0;
    format->data.direct.red     = g_lcd_r;
    format->data.direct.green   = g_lcd_g;
    format->data.direct.blue    = g_lcd_b;

    format->data.direct.a_mask  = 0x00000000;
    format->data.direct.r_mask  = g_lcd_r_mask;
    format->data.direct.g_mask  = g_lcd_g_mask;
    format->data.direct.b_mask  = g_lcd_b_mask;
  }
  if (orientation != NULL) *orientation = g_lcd_orientation;
  if (dpi         != NULL) *dpi         = g_lcd_dpi;

  // default:: landscape
  if ((g_lcd_orientation == lcd_rotate0) || 
      (g_lcd_orientation == lcd_rotate180))
  {
    if (width != NULL)     *width     = g_width;
    if (height != NULL)    *height    = g_height;
    if (row_bytes != NULL) *row_bytes = g_width_rb;
  }
  // default:: portrait
  else
  {
    if (width != NULL)     *width     = g_height;
    if (height != NULL)    *height    = g_width;
    if (row_bytes != NULL) *row_bytes = g_height_rb;
  }
}

// set the palette of the LCD display controller
void
_LCDPalette(boolean mode, uint8 *palette)
{
  // macosx doesn't run in paletted mode
}

void
_LCDRepaintRegion(rectangle *rect)
{
  g_LCDRepaintRegion(rect);
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

  size = ((uint32)g_width_rb * g_height_rb / (LCD_DEPTH >> 3));

  // initialize offscreen window
  g_lcd_pm = NewPixMap();  
  (**g_lcd_pm).baseAddr    = malloc(size);
  (**g_lcd_pm).rowBytes    = g_width_rb | 0x8000;  // pixmap
  (**g_lcd_pm).bounds      = g_windowRect;
  (**g_lcd_pm).pmVersion   = 0;
  (**g_lcd_pm).packType    = 0;            // no packing (raw data)
  (**g_lcd_pm).packSize    = 0;
  (**g_lcd_pm).hRes        = 0x00480000;   // 72dpi
  (**g_lcd_pm).vRes        = 0x00480000;   // 72dpi
  (**g_lcd_pm).pixelType   = RGBDirect;
  (**g_lcd_pm).pixelSize   = LCD_DEPTH;
  (**g_lcd_pm).cmpCount    = 3;
  (**g_lcd_pm).cmpSize     = LCD_COLOR_BITS; // all components = same
#if   (LCD_COLOR_BITS == 5)
  (**g_lcd_pm).pixelFormat = k16LE555PixelFormat;
#elif (LCD_COLOR_BITS == 8)
  (**g_lcd_pm).pixelFormat = k32RGBAPixelFormat; 
#else
  #error - unsupported display bits
#endif
  (**g_lcd_pm).pmTable     = NULL;
  (**g_lcd_pm).pmExt       = NULL;

  g_lcd_bits     = (short *)GetPixBaseAddr(g_lcd_pm);

  // allocate the offscreen window (for use with rotation of display)
  g_lcd_off_bits = malloc(size);

  // fill the windows (both) with white color
  memset(g_lcd_bits,     0x00, size);  // black window
  memset(g_lcd_off_bits, 0xff, size);  // white window (should copy to lcd)
}

static void
_LCDTerminate_RESIZE()
{
  // delete offscreen window
  if (g_lcd_off_bits) free(g_lcd_off_bits);
  g_lcd_off_bits = NULL;

  // remove screen buffer
  if (g_lcd_bits)     free(g_lcd_bits);
  g_lcd_bits = NULL;
  (**g_lcd_pm).baseAddr = g_lcd_bits;
  DisposePixMap(g_lcd_pm);
  g_lcd_pm   = NULL;
}

// repaint a specific region from offscreen to the LCD display
static void
_LCDRepaintRegion16(rectangle *rect)
{
  uint16 *pS, *pD;
  uint16  rw,  rh;
  uint32  rw_h;
  int     i, j;
  Rect    _rect;
  GrafPtr pCGrafSave;

  // entry requirements
  if (g_window == NULL) return;

  GetPort(&pCGrafSave);
  SetPortWindowPort(g_window);

  // default:: standard mode
  switch (g_lcd_orientation)
  {
    case lcd_rotate0:

         // we do nothing! (already pointing to LCD)

/**
 ** OLD: using offscreen window
 **
         // ensure boundary clipping
         if  (rect->x > g_width)                  rect->x      = g_width;
         if  (rect->y > g_height)                 rect->y      = g_height;
         if ((rect->x + rect->width)  > g_width)  rect->width  = g_width  - rect->x;
         if ((rect->y + rect->height) > g_height) rect->height = g_height - rect->y;

         // ensure we are on a 4-byte boundary
         rw = (uint16)(g_width_rb / (LCD_DEPTH >> 3));
     
         // initialize pointers         
         pS = (uint16 *)g_lcd_off_bits + (rect->y * rw) + rect->x;
         pD = (uint16 *)g_lcd_bits     + (rect->y * rw) + rect->x;

         i = rect->height;
         while (i--)
         {
           j = rect->width;
           while (j--)
           {
             *pD++ = *pS++;
           }
           pD += (rw - rect->width);
           pS += (rw - rect->width); 
         }
  **/

         // draw screen buffer to the display (just area we are interested in)
         _rect.left   = rect->x;
         _rect.top    = rect->y;
         _rect.right  = rect->x + rect->width;
         _rect.bottom = rect->y + rect->height;
         CopyBits((BitMap *)*g_lcd_pm, 
                  GetPortBitMapForCopyBits(GetWindowPort(g_window)), 
                  &_rect, &_rect, srcCopy, 0);
         break;

    case lcd_rotate90:

         // ensure boundary clipping
         if  (rect->y > g_width)                  rect->y      = g_width;
         if  (rect->x > g_height)                 rect->x      = g_height;
         if ((rect->y + rect->height) > g_width)  rect->height = g_width  - rect->y;
         if ((rect->x + rect->width)  > g_height) rect->width  = g_height - rect->x;

         // ensure we are on a 4-byte boundary
         rw   = (uint16)(g_height_rb / (LCD_DEPTH >> 3));
         rh   = (uint16)(g_width_rb  / (LCD_DEPTH >> 3));
         rw_h = rect->height * rw;  // optimization
         
         // initialize pointers         
         pD  = (uint16 *)g_lcd_bits     + ((rh - rect->y - 1) + (rect->x * rh));
         pS  = (uint16 *)g_lcd_off_bits + (rect->x + (rect->y * rw));
         if (g_width & 3) pD -= (4 - (g_width & 3));

         i = rect->width;
         while (i--)
         {
           j = rect->height;
           while (j--)
           {
             *pD-- = *pS;
             pS += rw;
           }
           pS -= (rw_h - 1);
           pD += (rect->height + rh);
         }

         // draw screen buffer to the display (just area we are interested in)
         _rect.left   = rect->y;
         _rect.top    = rect->x;
         _rect.right  = _rect.left + rect->height;
         _rect.bottom = _rect.top  + rect->width;
         CopyBits((BitMap *)*g_lcd_pm, 
                  GetPortBitMapForCopyBits(GetWindowPort(g_window)), 
                  &_rect, &_rect, srcCopy, 0);
                
         break;

    case lcd_rotate180:

         // ensure boundary clipping
         if  (rect->x > g_width)                  rect->x      = g_width;
         if  (rect->y > g_height)                 rect->y      = g_height;
         if ((rect->x + rect->width)  > g_width)  rect->width  = g_width  - rect->x;
         if ((rect->y + rect->height) > g_height) rect->height = g_height - rect->y;

         // ensure we are on a 4-byte boundary
         rw = (uint16)(g_width_rb / (LCD_DEPTH >> 3));

         // initialize pointers         
         pS = (uint16 *)g_lcd_off_bits + ((rect->y + rect->height) * rw) + rect->x - 1;
         pD = (uint16 *)g_lcd_bits     +  (rect->y                 * rw) + rect->x;

         // offset for wider screens (due to odd sized screen)
         pS -= (rw - rect->width);
         
         i = rect->height;
         while (i--)
         {
           j = rect->width;
           while (j--)
           {
             *pD++ = *pS--;
           }
           pD += (rw - rect->width);
           pS -= (rw - rect->width); 
         }

         // draw screen buffer to the display (just area we are interested in)
         _rect.left   = rect->x;
         _rect.top    = rect->y;
         _rect.right  = rect->x + rect->width;
         _rect.bottom = rect->y + rect->height;
         CopyBits((BitMap *)*g_lcd_pm, 
                  GetPortBitMapForCopyBits(GetWindowPort(g_window)), 
                  &_rect, &_rect, srcCopy, 0);

         break;

    case lcd_rotate270:

         // ensure boundary clipping
         if  (rect->y > g_width)                  rect->y      = g_width;
         if  (rect->x > g_height)                 rect->x      = g_height;
         if ((rect->y + rect->height) > g_width)  rect->height = g_width  - rect->y;
         if ((rect->x + rect->width)  > g_height) rect->width  = g_height - rect->x;

         // ensure we are on a 4-byte boundary
         rw   = (uint16)(g_height_rb / (LCD_DEPTH >> 3));
         rh   = (uint16)(g_width_rb  / (LCD_DEPTH >> 3));
         rw_h = rect->height * rw;  // optimization

         // initialize pointers         
         pD  = (uint16 *)g_lcd_bits     + (rect->y + ((rw - rect->x - 1) * rh));
         pS  = (uint16 *)g_lcd_off_bits + (rect->x + (rect->y * rw));
         if (g_height & 3) pD -= ((4 - (g_height & 3)) * rh);

         i = rect->width;
         while (i--)
         { 
           j = rect->height;
           while (j--)
           {
             *pD++ = *pS;
             pS += rw;
           }
           pS -= (rw_h - 1);
           pD -= (rect->height + rh);
         }

         // draw screen buffer to the display (just area we are interested in)
         _rect.left   = rect->y;
         _rect.top    = rect->x;
         if (g_height & 3) _rect.top -= (4 - (g_height & 3));
         _rect.right  = _rect.left + rect->height;
         _rect.bottom = _rect.top + rect->width;
         CopyBits((BitMap *)*g_lcd_pm, 
                  GetPortBitMapForCopyBits(GetWindowPort(g_window)), 
                  &_rect, &_rect, srcCopy, 0);

         break;

    default:
         break;
  }

  SetPort(pCGrafSave);
}

// repaint a specific region from offscreen to the LCD display
static void
_LCDRepaintRegion32(rectangle *rect)
{
  uint32 *pS, *pD;
  uint16  rw,  rh;
  uint32  rw_h;
  int     i, j;
  Rect    _rect;
  GrafPtr pCGrafSave;

  // entry requirements
  if (g_window == NULL) return;

  GetPort(&pCGrafSave);
  SetPortWindowPort(g_window);

  // default:: standard mode
  switch (g_lcd_orientation)
  {
    case lcd_rotate0:

         // we do nothing! (already pointing to LCD)

/**
 ** OLD: using offscreen window
 **
         // ensure boundary clipping
         if  (rect->x > g_width)                  rect->x      = g_width;
         if  (rect->y > g_height)                 rect->y      = g_height;
         if ((rect->x + rect->width)  > g_width)  rect->width  = g_width  - rect->x;
         if ((rect->y + rect->height) > g_height) rect->height = g_height - rect->y;

         // ensure we are on a 4-byte boundary
         rw = (uint16)(g_width_rb / (LCD_DEPTH >> 3));
     
         // initialize pointers         
         pS = (uint32 *)g_lcd_off_bits + (rect->y * rw) + rect->x;
         pD = (uint32 *)g_lcd_bits     + (rect->y * rw) + rect->x;

         i = rect->height;
         while (i--)
         {
           j = rect->width;
           while (j--)
           {
             *pD++ = *pS++;
           }
           pD += (rw - rect->width);
           pS += (rw - rect->width); 
         }
  **/

         // draw screen buffer to the display (just area we are interested in)
         _rect.left   = rect->x;
         _rect.top    = rect->y;
         _rect.right  = rect->x + rect->width;
         _rect.bottom = rect->y + rect->height;
         CopyBits((BitMap *)*g_lcd_pm, 
                  GetPortBitMapForCopyBits(GetWindowPort(g_window)), 
                  &_rect, &_rect, srcCopy, NULL);
         break;

    case lcd_rotate90:

         // ensure boundary clipping
         if  (rect->y > g_width)                  rect->y      = g_width;
         if  (rect->x > g_height)                 rect->x      = g_height;
         if ((rect->y + rect->height) > g_width)  rect->height = g_width  - rect->y;
         if ((rect->x + rect->width)  > g_height) rect->width  = g_height - rect->x;

         // ensure we are on a 4-byte boundary
         rw   = (uint16)(g_height_rb / (LCD_DEPTH >> 3));
         rh   = (uint16)(g_width_rb  / (LCD_DEPTH >> 3));
         rw_h = rect->height * rw;  // optimization
         
         // initialize pointers         
         pD  = (uint32 *)g_lcd_bits     + ((rh - rect->y - 1) + (rect->x * rh));
         pS  = (uint32 *)g_lcd_off_bits + (rect->x + (rect->y * rw));
         if (g_width & 3) pD -= (4 - (g_width & 3));

         i = rect->width;
         while (i--)
         {
           j = rect->height;
           while (j--)
           {
             *pD-- = *pS;
             pS += rw;
           }
           pS -= (rw_h - 1);
           pD += (rect->height + rh);
         }

         // draw screen buffer to the display (just area we are interested in)
         _rect.left   = rect->y;
         _rect.top    = rect->x;
         _rect.right  = _rect.left + rect->height;
         _rect.bottom = _rect.top  + rect->width;
         CopyBits((BitMap *)*g_lcd_pm, 
                  GetPortBitMapForCopyBits(GetWindowPort(g_window)), 
                  &_rect, &_rect, srcCopy, NULL);
                
         break;

    case lcd_rotate180:

         // ensure boundary clipping
         if  (rect->x > g_width)                  rect->x      = g_width;
         if  (rect->y > g_height)                 rect->y      = g_height;
         if ((rect->x + rect->width)  > g_width)  rect->width  = g_width  - rect->x;
         if ((rect->y + rect->height) > g_height) rect->height = g_height - rect->y;

         // ensure we are on a 4-byte boundary
         rw = (uint16)(g_width_rb / (LCD_DEPTH >> 3));

         // initialize pointers         
         pS = (uint32 *)g_lcd_off_bits + ((rect->y + rect->height) * rw) + rect->x - 1;
         pD = (uint32 *)g_lcd_bits     +  (rect->y                 * rw) + rect->x;

         // offset for wider screens (due to odd sized screen)
         pS -= (rw - rect->width);
         
         i = rect->height;
         while (i--)
         {
           j = rect->width;
           while (j--)
           {
             *pD++ = *pS--;
           }
           pD += (rw - rect->width);
           pS -= (rw - rect->width); 
         }

         // draw screen buffer to the display (just area we are interested in)
         _rect.left   = rect->x;
         _rect.top    = rect->y;
         _rect.right  = rect->x + rect->width;
         _rect.bottom = rect->y + rect->height;
         CopyBits((BitMap *)*g_lcd_pm, 
                  GetPortBitMapForCopyBits(GetWindowPort(g_window)), 
                  &_rect, &_rect, srcCopy, NULL);

         break;

    case lcd_rotate270:

         // ensure boundary clipping
         if  (rect->y > g_width)                  rect->y      = g_width;
         if  (rect->x > g_height)                 rect->x      = g_height;
         if ((rect->y + rect->height) > g_width)  rect->height = g_width  - rect->y;
         if ((rect->x + rect->width)  > g_height) rect->width  = g_height - rect->x;

         // ensure we are on a 4-byte boundary
         rw   = (uint16)(g_height_rb / (LCD_DEPTH >> 3));
         rh   = (uint16)(g_width_rb  / (LCD_DEPTH >> 3));
         rw_h = rect->height * rw;  // optimization

         // initialize pointers         
         pD  = (uint32 *)g_lcd_bits     + (rect->y + ((rw - rect->x - 1) * rh));
         pS  = (uint32 *)g_lcd_off_bits + (rect->x + (rect->y * rw));
         if (g_height & 3) pD -= ((4 - (g_height & 3)) * rh);

         i = rect->width;
         while (i--)
         { 
           j = rect->height;
           while (j--)
           {
             *pD++ = *pS;
             pS += rw;
           }
           pS -= (rw_h - 1);
           pD -= (rect->height + rh);
         }

         // draw screen buffer to the display (just area we are interested in)
         _rect.left   = rect->y;
         _rect.top    = rect->x;
         if (g_height & 3) _rect.top -= (4 - (g_height & 3));
         _rect.right  = _rect.left + rect->height;
         _rect.bottom = _rect.top + rect->width;
         CopyBits((BitMap *)*g_lcd_pm, 
                  GetPortBitMapForCopyBits(GetWindowPort(g_window)), 
                  &_rect, &_rect, srcCopy, NULL);

         break;

    default:
         break;
  }

  SetPort(pCGrafSave);
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-display.inc"

/********************************* EOF ***********************************/
