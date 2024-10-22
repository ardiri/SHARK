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
  g_lcd_dpi    = lcd_dpi96;

  return init_ok;
}

static void
DAL_LCDInitialize_RESIZE(uint16 width, uint16 height)
{
  ApplicationView *v = (ApplicationView *)g_view;

  // now we know the size of the display
  g_lcd_width     = width;

  g_lcd_width_rb  = TEXTURE_WIDTH << 1;
  g_lcd_height    = height;
  g_lcd_height_rb = (((g_lcd_height + 3) & ~3) * (g_lcd_depth_bus >> 3));

  // we have a dynamic address and it is double buffering?
  g_lcd_doublebuffered  = true;
  g_lcd_requirelock     = false;

  // initialize the framebuffer
#if (IOS_ALTERNATE_BUFFERS == 0)
  g_lcd_framebuffer_dir  = v -> _spriteFrameBuffer;
#else
  g_lcd_framebuffer_dir  = v -> _spriteFrameBuffer[ v -> _spriteFrameIndex ];
  v -> _spriteFrameIndex = 1 - v -> _spriteFrameIndex;
#endif
}

static void
DAL_LCDTerminate_RESIZE()
{
}

static void
DAL_LCDFlipDisplay()
{
#if (IOS_ALTERNATE_BUFFERS == 1)
  ApplicationView *v = (ApplicationView *)g_view;

  // for the next time around; use the other framebuffer
  g_lcd_framebuffer_dir = v -> _spriteFrameBuffer[ v -> _spriteFrameIndex ];
  v -> _spriteFrameIndex = 1 - v -> _spriteFrameIndex;
#endif

  [ g_view performSelectorOnMainThread:@selector(swapBuffers) 
           withObject:NULL waitUntilDone: NO ];
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
