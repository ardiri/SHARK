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
  SDL_Surface  *lcd_screen;

#define g_lcd_screen           (LCD_GLOBALS) -> lcd_screen

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

  // setup the video mode
  g_lcd_screen = SDL_SetVideoMode(width, height, 0, SDL_SWSURFACE);

  g_lcd_type      = lcd_direct;
  g_lcd_depth     = g_lcd_screen -> format -> BitsPerPixel;
  g_lcd_depth_bus = (uint16)((g_lcd_depth + 15) & ~0xf);
  g_lcd_a         = bitsset(g_lcd_screen -> format -> Amask);
  g_lcd_r         = bitsset(g_lcd_screen -> format -> Rmask);
  g_lcd_g         = bitsset(g_lcd_screen -> format -> Gmask);
  g_lcd_b         = bitsset(g_lcd_screen -> format -> Bmask);
  g_lcd_a_mask    = g_lcd_screen -> format -> Amask;
  g_lcd_r_mask    = g_lcd_screen -> format -> Rmask;
  g_lcd_g_mask    = g_lcd_screen -> format -> Gmask;
  g_lcd_b_mask    = g_lcd_screen -> format -> Bmask;

  // SPECIAL: SDL does not report it correctly for host mode
  g_lcd_a_mask    = (g_lcd_screen -> format -> Rmask == 0x0000ff00) ? 1 : 0;

  g_lcd_inverted  = false;

  // lets determine the DPI of the display
  g_lcd_dpi    = lcd_dpi96;

  return init_ok;
}

static void
DAL_LCDInitialize_RESIZE(uint16 width, uint16 height)
{
  // now we know the size of the display
  g_lcd_width     = width;
  g_lcd_width_rb  = (((g_lcd_width  + 3) & ~3) * (g_lcd_depth_bus >> 3));
  g_lcd_height    = height;
  g_lcd_height_rb = (((g_lcd_height + 3) & ~3) * (g_lcd_depth_bus >> 3));

  // we have a static LCD address and it is double buffering?
  g_lcd_doublebuffered  = true;
  g_lcd_requirelock     = SDL_MUSTLOCK(g_lcd_screen);
  g_lcd_framebuffer_dir = g_lcd_screen -> pixels;
}

static void
DAL_LCDTerminate_RESIZE()
{
}

static void
DAL_LCDFlipDisplay()
{
  SDL_Flip(g_lcd_screen);
}

static void *
DAL_LCDLock()
{
  if (SDL_MUSTLOCK(g_lcd_screen))
    SDL_LockSurface(g_lcd_screen);

  return g_lcd_framebuffer_dir;
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
  if (SDL_MUSTLOCK(g_lcd_screen))
    SDL_UnlockSurface(g_lcd_screen);
}

static void 
DAL_LCDTerminate()
{
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef LCD_GLOBALS

/********************************* EOF ***********************************/
