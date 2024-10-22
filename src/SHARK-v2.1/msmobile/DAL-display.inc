/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-display-DIB.c  (device independent bitmaps - for reference)
 */

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

static HDC               g_lcd_dc;            // window dc handle
static HBITMAP           g_lcd_bmp;           // bitmap handle

static uint32            g_lcd_bit_size;
static void             *g_lcd_bits;          // bitmap buffer pointer
static void             *g_lcd_off_bits;      // offscreen window (drawing)

static lcd_orientation   g_lcd_orientation;
static lcd_dpi           g_lcd_dpi;

static BITMAPINFOHEADER  g_lcd_bmiHeader;
static DWORD             g_lcd_bmiColors[4];  // bitmap information

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

  // configure the bitmap to use the currently active palette (important)
  g_lcd_bmiColors[0] = g_lcd_r_mask;
  g_lcd_bmiColors[1] = g_lcd_g_mask;
  g_lcd_bmiColors[2] = g_lcd_b_mask;

  // default DPI on windows mobile is 96
  g_lcd_dpi = lcd_dpi96;
  if (g_width > 320) g_lcd_dpi = lcd_dpi192;  // hires displays

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

void
_LCDSuspend()
{
  // DIB - no suspend option, handled by window manager
}

void
_LCDResume()
{
  // DIB - no resume option, handled by window manager
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
  // msmobile doesn't support any paletted displays
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
  BITMAPINFO *pbmi;

  // calculate the size of the window
  g_lcd_bit_size = (uint32)g_width_rb * (g_height_rb / (LCD_DEPTH >> 3));

  // verify the device + platform structures are valid
  pbmi = (BITMAPINFO *)&g_lcd_bmiHeader;
  pbmi->bmiHeader.biClrImportant  = 0;
  pbmi->bmiHeader.biClrUsed       = 0;
  pbmi->bmiHeader.biCompression   = BI_BITFIELDS;
  pbmi->bmiHeader.biPlanes        = 1;
  pbmi->bmiHeader.biBitCount      = LCD_DEPTH;
  pbmi->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
  pbmi->bmiHeader.biWidth         =  (LONG)(g_width_rb  / (LCD_DEPTH >> 3));
  pbmi->bmiHeader.biHeight        = -(LONG)(g_height_rb / (LCD_DEPTH >> 3));
  pbmi->bmiHeader.biSizeImage     =  (LONG)g_lcd_bit_size;
  pbmi->bmiHeader.biXPelsPerMeter = 3760;
  pbmi->bmiHeader.biYPelsPerMeter = 3760;

  g_lcd_dc  = CreateCompatibleDC(GetDC(g_hWnd));
  g_lcd_bmp = CreateDIBSection(g_lcd_dc, pbmi, DIB_RGB_COLORS,
                               ((void **)(&g_lcd_bits)), NULL, 0);
  SelectObject(g_lcd_dc, g_lcd_bmp);

  // allocate the offscreen window (for use with rotation of display)
  g_lcd_off_bits = (void *)malloc(g_lcd_bit_size);

  // fill the windows offscreen window with white
  memset(g_lcd_bits,     0x00, g_lcd_bit_size);
  memset(g_lcd_off_bits, 0xff, g_lcd_bit_size);
}

static void
_LCDTerminate_RESIZE()
{
  // delete offscreen window
  if (g_lcd_off_bits) free(g_lcd_off_bits);
  g_lcd_off_bits = NULL;

  // remove screen buffer
  if (g_lcd_bmp != NULL) DeleteObject(g_lcd_bmp);
  if (g_lcd_dc  != NULL) DeleteDC(g_lcd_dc);
  g_lcd_bmp = NULL;
  g_lcd_dc  = NULL;
}

// repaint a specific region from offscreen to the LCD display
static void
_LCDRepaintRegion16(rectangle *rect)
{
  uint16 *pS, *pD;
  uint16  rw,  rh;
  uint32  rw_h;
  int     i, j;

  // entry requirements
  if (g_hWnd == NULL) return;  // we must have a valid window to draw to

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

         // draw screen buffer to the display (just the area of interest)
         BitBlt(GetDC(g_hWnd),
                rect->x, rect->y, rect->width, rect->height,
                g_lcd_dc, rect->x, rect->y, SRCCOPY);

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
         rw_h = rect->height * rw; // optimization

         // initialize pointers
         pD  = (uint16 *)g_lcd_bits + ((rh - rect->y - 1) + (rect->x * rh));
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

         // draw screen buffer to the display (just the area of interest)
         BitBlt(GetDC(g_hWnd),
                rect->y, rect->x, rect->height, rect->width,
                g_lcd_dc, rect->y, rect->x, SRCCOPY);

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
         pD = (uint16 *)g_lcd_bits + (rect->y * rw) + rect->x;

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

         // draw screen buffer to the display (just the area of interest)
         BitBlt(GetDC(g_hWnd),
                rect->x, rect->y, rect->width, rect->height,
                g_lcd_dc, rect->x, rect->y, SRCCOPY);

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
         rw_h = rect->height * rw; // optimization

         // initialize pointers
         pD  = (uint16 *)g_lcd_bits + (rect->y + ((rw - rect->x - 1) * rh));
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

         // draw screen buffer to the display (just the area of interest)
         BitBlt(GetDC(g_hWnd),
                rect->y, rect->x, rect->height, rect->width,
                g_lcd_dc, rect->y, rect->x, SRCCOPY);

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

  // entry requirements
  if (g_hWnd == NULL) return;  // we must have a valid window to draw to

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

         // draw screen buffer to the display (just the area of interest)
         BitBlt(GetDC(g_hWnd),
                rect->x, rect->y, rect->width, rect->height,
                g_lcd_dc, rect->x, rect->y, SRCCOPY);

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
         rw_h = rect->height * rw; // optimization

         // initialize pointers
         pD  = (uint32 *)g_lcd_bits + ((rh - rect->y - 1) + (rect->x * rh));
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

         // draw screen buffer to the display (just the area of interest)
         BitBlt(GetDC(g_hWnd),
                rect->y, rect->x, rect->height, rect->width,
                g_lcd_dc, rect->y, rect->x, SRCCOPY);

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
         pD = (uint32 *)g_lcd_bits + (rect->y * rw) + rect->x;

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

         // draw screen buffer to the display (just the area of interest)
         BitBlt(GetDC(g_hWnd),
                rect->x, rect->y, rect->width, rect->height,
                g_lcd_dc, rect->x, rect->y, SRCCOPY);

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
         rw_h = rect->height * rw; // optimization

         // initialize pointers
         pD  = (uint32 *)g_lcd_bits + (rect->y + ((rw - rect->x - 1) * rh));
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

         // draw screen buffer to the display (just the area of interest)
         BitBlt(GetDC(g_hWnd),
                rect->y, rect->x, rect->height, rect->width,
                g_lcd_dc, rect->y, rect->x, SRCCOPY);

         break;

    default:
         break;
  }
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-display.inc"

/********************************* EOF ***********************************/
