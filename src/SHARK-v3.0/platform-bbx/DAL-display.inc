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
  screen_buffer_t              lcd_buffer; 

#define g_lcd_buffer           (LCD_GLOBALS) -> lcd_buffer

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

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(LCD_GLOBALS, 0, sizeof(LCDGlobals));

  g_lcd_type      = lcd_direct;
  g_lcd_depth     = 16;
  g_lcd_depth_bus = (uint16)((g_lcd_depth + 15) & ~0xf);
  g_lcd_a         = 0;
  g_lcd_r         = 5;
  g_lcd_g         = 6;
  g_lcd_b         = 5;
  g_lcd_a_mask    = 0;
  g_lcd_r_mask    = 0xf800;
  g_lcd_g_mask    = 0x07e0;
  g_lcd_b_mask    = 0x001f;

  g_lcd_inverted  = false;

  // lets determine the DPI of the display
  g_lcd_dpi       = lcd_dpi192;

  // create a the buffer required for rendering the window
  screen_create_window_buffers(g_window, 1);
  screen_get_window_property_pv(g_window,
                                SCREEN_PROPERTY_RENDER_BUFFERS,
                                (void **)&g_lcd_buffer);

  return init_ok;
}

static void
DAL_LCDInitialize_RESIZE(uint16 width, uint16 height)
{
  int pair[2];
  int param;
 
  // obtain the bounds of the buffer
  screen_get_buffer_property_iv(g_lcd_buffer,
                                SCREEN_PROPERTY_BUFFER_SIZE, pair);

  // obtain the rowbytes value (stride) between vertical lines
  screen_get_buffer_property_iv(g_lcd_buffer,
                                SCREEN_PROPERTY_STRIDE, &param);

  g_lcd_width     = pair[0];
  g_lcd_width_rb = (int16)param;
  g_lcd_height    = pair[1];
  g_lcd_height_rb = (((g_lcd_height + 3) & ~3) * (g_lcd_depth_bus >> 3));

  // we have a static LCD adress and it isn't double buffered
  g_lcd_doublebuffered  = false;
  g_lcd_requirelock     = false;

  // obtain the direct framebuffer address of the window buffer
  screen_get_buffer_property_pv(g_lcd_buffer,
                                SCREEN_PROPERTY_POINTER, 
                                (void **)&g_lcd_framebuffer_dir);
}

static void
DAL_LCDTerminate_RESIZE()
{
}

static void
DAL_LCDFlipDisplay()
{
  int rect[4] = { 0, 0, g_width, g_height };
  screen_post_window(g_window, g_lcd_buffer, 1, rect, 0);
}

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
