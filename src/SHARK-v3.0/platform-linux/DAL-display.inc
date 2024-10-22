/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-display.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define LCD_GLOBALS           g_lcd
#define GLOBALS_UNINITIALIZED (LCD_GLOBALS == NULL)
#define GLOBALS_DEFINITION    LCDGlobals *g_lcd = NULL

// LCDGlobals::   platform additions
#define DAL_LCD_GLOBALS \
  XImage  *lcd_bitmap;          \
                                \
  void    *win_framebuffer_dir; \
  XImage  *win_bitmap;      

#define g_lcd_bitmap           (LCD_GLOBALS) -> lcd_bitmap

#define g_win_bitmap           (LCD_GLOBALS) -> win_bitmap
#define g_win_framebuffer_dir  (LCD_GLOBALS) -> win_framebuffer_dir

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-display.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
DAL_LCDInitialize(uint16 width, uint16 height)
{
  boolean init_ok;
  int     depth;
  Visual *visual;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(LCD_GLOBALS, 0, sizeof(LCDGlobals));

  depth  = DefaultDepth(g_display, g_screen);
  visual = DefaultVisual(g_display, g_screen);

  g_lcd_type      = lcd_direct;
  g_lcd_depth     = depth;
  g_lcd_depth_bus = (uint16)((depth + 15) & ~0xf);
  g_lcd_a         = 0;
  g_lcd_r         = bitsset(visual -> red_mask);
  g_lcd_g         = bitsset(visual -> green_mask);
  g_lcd_b         = bitsset(visual -> blue_mask);
  g_lcd_a_mask    = 0;
  g_lcd_r_mask    = visual -> red_mask;
  g_lcd_g_mask    = visual -> green_mask;
  g_lcd_b_mask    = visual -> blue_mask;

  g_lcd_inverted  = false;

  // lets determine the DPI of the display
  g_lcd_dpi    = lcd_dpi96;

  return init_ok;
}

static void
DAL_LCDInitialize_RESIZE(uint16 width, uint16 height)
{
  uint32  size;
  void   *bits;

  //
  // BACKGROUND WINDOW
  //

  // now we know the size of the display
  g_lcd_width     = APPLICATION_WINDOW_WIDTH;
  g_lcd_width_rb  = (((g_lcd_width  + 3) & ~3) * (g_lcd_depth_bus >> 3));
  g_lcd_height    = APPLICATION_WINDOW_HEIGHT;
  g_lcd_height_rb = (((g_lcd_height + 3) & ~3) * (g_lcd_depth_bus >> 3));

  // calculate the size of the window
  size = ((uint32)g_lcd_width_rb * g_lcd_height);

  // allocate the memory required for the window
  bits = (void *)malloc(size);
  if (bits != NULL)
  {
    union { uint8 c[2]; uint16 s; } order;
 
    memset(bits, 0x00, size);
    g_win_bitmap = 
      XCreateImage(g_display, CopyFromParent, 
                   DefaultDepth(g_display, g_screen), ZPixmap, 0, 
                   bits, g_lcd_width, g_lcd_height, 
                   g_lcd_depth_bus, g_lcd_width_rb);

    // initialize the image
    XInitImage(g_win_bitmap);

    // set clients byte order
    order.s = 1;
    if (order.c[0] == 1) g_win_bitmap -> byte_order = LSBFirst;
    else                 g_win_bitmap -> byte_order = MSBFirst;
    g_win_bitmap -> bitmap_bit_order = MSBFirst;

    // we have a static LCD address and it is double buffering?
    g_win_framebuffer_dir = g_win_bitmap -> data;
  }

  //
  // LCD WINDOW
  //

  // now we know the size of the display
  g_lcd_width     = width;
  g_lcd_width_rb  = (((g_lcd_width  + 3) & ~3) * (g_lcd_depth_bus >> 3));
  g_lcd_height    = height;
  g_lcd_height_rb = (((g_lcd_height + 3) & ~3) * (g_lcd_depth_bus >> 3));

  // calculate the size of the window
  size = ((uint32)g_lcd_width_rb * g_lcd_height);

  // allocate the memory required for the window
  bits = (void *)malloc(size);
  if (bits != NULL)
  {
    union { uint8 c[2]; uint16 s; } order;
 
    memset(bits, 0x00, size);
    g_lcd_bitmap = 
      XCreateImage(g_display, CopyFromParent, 
                   DefaultDepth(g_display, g_screen), ZPixmap, 0, 
                   bits, g_lcd_width, g_lcd_height, 
                   g_lcd_depth_bus, g_lcd_width_rb);

    // initialize the image
    XInitImage(g_lcd_bitmap);

    // set clients byte order
    order.s = 1;
    if (order.c[0] == 1) g_lcd_bitmap -> byte_order = LSBFirst;
    else                 g_lcd_bitmap -> byte_order = MSBFirst;
    g_lcd_bitmap -> bitmap_bit_order = MSBFirst;

    // we have a static LCD address and it is double buffering?
    g_lcd_doublebuffered  = true;
    g_lcd_requirelock     = false;
    g_lcd_framebuffer_dir = g_lcd_bitmap -> data;
  }
}

static void
DAL_LCDTerminate_RESIZE()
{
  //
  // LCD WINDOW
  //

  // release the memory used by the bitmap
  if (g_lcd_bitmap != NULL) XDestroyImage(g_lcd_bitmap);
  g_lcd_bitmap          = NULL;
  g_lcd_framebuffer_dir = NULL;

  //
  // BACKGROUND WINDOW
  //

  // release the memory used by the bitmap
  if (g_win_bitmap != NULL) XDestroyImage(g_win_bitmap);
  g_win_bitmap          = NULL;
  g_win_framebuffer_dir = NULL;
}

#define PIXEL_565AVG(c1,c2) ((((c1) &     0xf7de) >> 1) + (((c2) &     0xf7de) >> 1))
#define PIXEL_888AVG(c1,c2) ((((c1) & 0xfffefefe) >> 1) + (((c2) & 0xfffefefe) >> 1))

static void
DAL_LCDFlipDisplay()
{
  GC                gc;
#ifdef GFX_SCALING_150 
  int               i, j;
  int32             d, s;
  XWindowAttributes wA;
  int16             x, y;
#endif

  // create the graphics context to draw to
  gc = XCreateGC(g_display, g_window, 0, NULL);

#ifdef GFX_SCALING_150 

  if (g_lcd_depth_bus == 32)
  {
    uint32 *s0, *s1;
    uint32 *d0, *d1, *d2;

    // COPY FROM LCD WINDOW TO BACKGROUND WINDOW
    s0 = g_lcd_framebuffer_dir;
    s1 = s0 +  g_lcd_width;
    d0 = g_win_framebuffer_dir;
    d1 = d0 + APPLICATION_WINDOW_WIDTH;
    d2 = d1 + APPLICATION_WINDOW_WIDTH;
    for (j=0; j<(g_lcd_height / 2); j++)
    {
      s = 0;
      d = 0;
      for (i=0; i<(g_lcd_width / 2); i++)
      {
        d0[d+0] = s0[s+0];
        d0[d+1] = PIXEL_888AVG(s0[s+0], s0[s+1]);
        d0[d+2] = s0[s+1];
        d1[d+0] = PIXEL_888AVG(s0[s+0], s1[s+0]);
        d1[d+1] = PIXEL_888AVG(s0[s+0], s1[s+1]);
        d1[d+2] = PIXEL_888AVG(s1[s+1], s1[s+1]);
        d2[d+0] = s1[s+0];
        d2[d+1] = PIXEL_888AVG(s1[s+0], s1[s+1]);
        d2[d+2] = s1[s+1];

        s += 2; 
        d += 3; 
      }
  
      s0 += (g_lcd_width * 2);
      s1 += (g_lcd_width * 2);
      d0 += (APPLICATION_WINDOW_WIDTH * 3);
      d1 += (APPLICATION_WINDOW_WIDTH * 3);
      d2 += (APPLICATION_WINDOW_WIDTH * 3);
    }
  }
  else
  if (g_lcd_depth_bus == 16)
  {
    uint16 *s0, *s1;
    uint16 *d0, *d1, *d2;

    // COPY FROM LCD WINDOW TO BACKGROUND WINDOW
    s0 = g_lcd_framebuffer_dir;
    s1 = s0 +  g_lcd_width;
    d0 = g_win_framebuffer_dir;
    d1 = d0 + APPLICATION_WINDOW_WIDTH;
    d2 = d1 + APPLICATION_WINDOW_WIDTH;
    for (j=0; j<(g_lcd_height / 2); j++)
    {
      s = 0;
      d = 0;
      for (i=0; i<(g_lcd_width / 2); i++)
      {
        d0[d+0] = s0[s+0];
        d0[d+1] = PIXEL_565AVG(s0[s+0], s0[s+1]);
        d0[d+2] = s0[s+1];
        d1[d+0] = PIXEL_565AVG(s0[s+0], s1[s+0]);
        d1[d+1] = PIXEL_565AVG(s0[s+0], s1[s+1]);
        d1[d+2] = PIXEL_565AVG(s0[s+1], s1[s+1]);
        d2[d+0] = s1[s+0];
        d2[d+1] = PIXEL_565AVG(s1[s+0], s1[s+1]);
        d2[d+2] = s1[s+1];
  
        s += 2; 
        d += 3; 
      }
  
      s0 += (g_lcd_width * 2);
      s1 += (g_lcd_width * 2);
      d0 += (APPLICATION_WINDOW_WIDTH * 3);
      d1 += (APPLICATION_WINDOW_WIDTH * 3);
      d2 += (APPLICATION_WINDOW_WIDTH * 3);
    }
  }

  // UPDATE WINDOW
  XGetWindowAttributes(g_display, g_window, &wA);
  x = (wA.width  - APPLICATION_WINDOW_WIDTH)  >> 1;
  y = (wA.height - APPLICATION_WINDOW_HEIGHT) >> 1;
  XPutImage(g_display, g_window, gc, 
            g_win_bitmap, 0, 0, x, y, APPLICATION_WINDOW_WIDTH, APPLICATION_WINDOW_HEIGHT);

#else

  XPutImage(g_display, g_window, gc, 
            g_lcd_bitmap, 0, 0, 0, 0, g_lcd_width, g_lcd_height);

#endif

  // free the graphics context 
  XFreeGC(g_display, gc);

  // flush the display
  XFlush(g_display);
}

#undef PIXEL_565AVG
#undef PIXEL_888AVG

static void *
DAL_LCDLock()
{
  return NULL;
}

static void
DAL_LCDResume()
{
}

static void
DAL_LCDSuspend()
{
}

static void
DAL_LCDUnlock()
{
}

static void 
DAL_LCDTerminate()
{
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef LCD_GLOBALS

/********************************* EOF ***********************************/
