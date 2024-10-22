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
  void             *bits;          // bitmap buffer pointer
  void             *off_bits;      // offscreen window (drawing)

  lcd_orientation   orientation;
  lcd_dpi           dpi;

  void            (*LCDRepaintRegion)(rectangle *rect);
} LCDGlobalsType;

#define GLOBALS_DEFINE_LCD LCDGlobalsType *g_lcd;
#define GLOBALS_ACCESS_LCD GLOBALS_ACCESS; \
                           LCDGlobalsType *g_lcd = (LCDGlobalsType *)g->lcd;

#define g_lcd_bits         g_lcd->bits
#define g_lcd_off_bits     g_lcd->off_bits
#define g_lcd_orientation  g_lcd->orientation
#define g_lcd_dpi          g_lcd->dpi
#define g_LCDRepaintRegion g_lcd->LCDRepaintRegion

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
  boolean init_ok;
  GLOBALS_ACCESS;
  GLOBALS_DEFINE_LCD;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _LCDInitialize");
#endif

  // assume everything is ok
  init_ok = true;

  // globals:: allocate appropriate memory
  g_lcd = (LCDGlobalsType *)malloc(sizeof(LCDGlobalsType));
  init_ok = (g_lcd != NULL); if (!init_ok) goto LCD_INIT_ABORT;
  memset(g_lcd, 0, sizeof(LCDGlobalsType));
  g->lcd = g_lcd;

  // lets determine the DPI of the display
  g_lcd_dpi = lcd_dpi96;
  if (g_width > 208) g_lcd_dpi = lcd_dpi192; // hires versions

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

LCD_INIT_ABORT:

  return init_ok;
}

static void
_LCDSuspend()
{
  // symbian - no suspend option, handled by window manager
}

static void
_LCDResume()
{
  // symbian - no resume option, handled by window manager
  _LCDRepaint();
}

// set the display orientation, forced _key_gfx_change event
void
_LCDSetOrientation(lcd_orientation orientation)
{
  event e;
  GLOBALS_ACCESS_LCD;
  
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
  GLOBALS_ACCESS_LCD;

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
  // symbian doesn't run in paletted mode
}

void
_LCDRepaintRegion(rectangle *rect)
{
  GLOBALS_ACCESS_LCD;

  // we can only do something if we are active
  if (!g_active) return;

  g_LCDRepaintRegion(rect);

  // force changes to real display
  TRawEvent redraw;
  redraw.Set(TRawEvent::ERedraw);
  UserSvr::AddEvent(redraw);
}

// terminate the LCD display driver
static void
_LCDTerminate()
{
//GLOBALS_ACCESS_LCD;

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
  GLOBALS_ACCESS_LCD;

  size = (uint32)g_width_rb * (g_height_rb / (LCD_DEPTH >> 3));

  // initialize offscreen window
  TScreenInfoV01 screenInfo;
  TPckg<TScreenInfoV01> sInfo(screenInfo);
  UserSvr::ScreenInfo(sInfo);

  g_lcd_bits = (void *)((uint8 *)screenInfo.iScreenAddress + 32);

  // allocate the offscreen window (for use with rotation of display)
  g_lcd_off_bits = malloc(size);

  // fill the windows (both) with white color
  memset(g_lcd_bits,     0x00, size);  // black window
  memset(g_lcd_off_bits, 0xff, size);  // white window (should copy to lcd)
}

static void
_LCDTerminate_RESIZE()
{
  GLOBALS_ACCESS_LCD;

  if (g_lcd != NULL)
  {
    // delete offscreen window
    if (g_lcd_off_bits != NULL) free(g_lcd_off_bits);
    g_lcd_off_bits = NULL;
  }

  // globals:: clean up memory
  if (g_lcd != NULL) free(g_lcd); 
  g_lcd = NULL; g->lcd = g_lcd;
}

// repaint a specific region from offscreen to the LCD display
static void
_LCDRepaintRegion16(rectangle *rect)
{
  uint16 *pS, *pD;
  uint16  rw,  rh;
  uint32  rw_h;
  int     i, j;
  GLOBALS_ACCESS_LCD;

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
         break;

    default:
         break;
  }
}

// repaint a specific region from offscreen to the LCD display
static void
_LCDRepaintRegion32(rectangle *rect)
{
  uint32 *pS, *pD;
  uint16  rw,  rh;
  uint32  rw_h;
  int     i, j;
  GLOBALS_ACCESS_LCD;

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
         break;

    default:
         break;
  }
}

#undef GLOBALS_DEFINE_LCD
#undef GLOBALS_ACCESS_LCD

#undef g_lcd_bits
#undef g_lcd_off_bits
#undef g_lcd_orientation
#undef g_lcd_dpi
#undef g_LCDRepaintRegion

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-display.inc"

/********************************* EOF ***********************************/
