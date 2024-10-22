/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-fb-d16.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// -----------------------------------------------------------------------
// compile time options
// -----------------------------------------------------------------------

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

// none

/*************************************************************************
 * Global Variables
 *************************************************************************/

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibGetGlobals() API.
//------------------------------------------------------------------------

/*************************************************************************
 * Generic _FB routines
 *************************************************************************/

static boolean
_FB_D16_Initialize()
{
  boolean  init_ok;
  FB_GLOBALS_ACCESS;

  // assume everything is ok
  init_ok = true;

  // allocate globals memory for the library
  g_fb -> extension = NULL;

  return init_ok;
}

static void
_FB_D16_Terminate()
{
}

/*************************************************************************
 * _FB CopyToDisplay implementations
 *************************************************************************/

// this is a generic (un-optimized) conversion routine
#define _FB_X_D16_CopyToDisplay(FB, LCD)                                  \
static void                                                               \
_FB_ ## FB ## _ ## LCD ## _CopyToDisplay(fb_region *region,               \
                                           uint16 scr_x, uint16 scr_y)    \
{                                                                         \
  uint16      *pD, *pS;                                                   \
  int16        cnt, pix, i, win_delta, lcd_delta;                         \
  color        val;                                                       \
  uint8        r, g, b;                                                   \
  uint16       x1, y1, x2, y2;                                            \
  FB_GLOBALS_ACCESS;                                                      \
                                                                          \
  /* entry condition */                                                   \
  if (region == NULL) return;                                             \
                                                                          \
  pS = (uint16 *)g_fb -> bits;                                            \
  pD = (uint16 *)g_fb -> LCD_bits;                                        \
                                                                          \
  /* whats a "valid" box area for this operation? */                      \
  x1 = region -> x;                                                       \
  y1 = region -> y;                                                       \
  x2 = MIN(g_fb -> width,  region -> x + region -> width);                \
  y2 = MIN(g_fb -> height, region -> y + region -> height);               \
                                                                          \
  /* whats a "valid" box area for this operation? - destination window */ \
  if ((scr_x+(x2-x1)) > g_fb -> width)                                    \
    { x2=x1 + (g_fb -> width  - scr_x); }                                 \
  if ((scr_y+(y2-y1)) > g_fb -> height)                                   \
    { y2=y1 + (g_fb -> height - scr_y); }                                 \
                                                                          \
  /* do we still have a valid region? */                                  \
  if ((x2 > x1) && (y2 > y1))                                             \
  {                                                                       \
    pS += (((uint32)y1    * (g_fb -> rowBytes     >> 1)) + x1);           \
    pD += (((uint32)scr_y * (g_fb -> LCD_rowBytes >> 1)) + scr_x);        \
                                                                          \
    /* if the LCD window != same size as _FB window, center it */         \
    pD +=  (g_fb -> LCD_width  - g_fb -> width)  >> 1;                    \
    pD += ((g_fb -> LCD_height - g_fb -> height) >> 1) *                  \
           (g_fb -> LCD_rowBytes >> 1);                                   \
                                                                          \
    cnt = y2-y1;                                                          \
    pix = x2-x1;                                                          \
                                                                          \
    win_delta = (g_fb -> rowBytes     >> 1) - pix;                        \
    lcd_delta = (g_fb -> LCD_rowBytes >> 1) - pix;                        \
                                                                          \
    /* do each line... */                                                 \
    do                                                                    \
    {                                                                     \
      i = pix;                                                            \
      do                                                                  \
      {                                                                   \
        val = *pS++;                                                      \
        _FB_ ## FB ## _GetRGBFromIndex(val, &r, &g, &b);                  \
        *pD++ = (uint16)_FB_ ## LCD ## _GetPaletteIndex(r, g, b);         \
      } while (--i);                                                      \
                                                                          \
      pS += win_delta;                                                    \
      pD += lcd_delta;                                                    \
    }                                                                     \
    while (--cnt);                                                        \
  }                                                                       \
}

// this is a generic (un-optimized) conversion routine
#define _FB_X_D32_CopyToDisplay(FB, LCD)                                  \
static void                                                               \
_FB_ ## FB ## _ ## LCD ## _CopyToDisplay(fb_region *region,               \
                                           uint16 scr_x, uint16 scr_y)    \
{                                                                         \
  uint32      *pD;                                                        \
  uint16      *pS;                                                        \
  int16        cnt, pix, i, win_delta, lcd_delta;                         \
  color        val;                                                       \
  uint8        r, g, b;                                                   \
  uint16       x1, y1, x2, y2;                                            \
  FB_GLOBALS_ACCESS;                                                      \
                                                                          \
  /* entry condition */                                                   \
  if (region == NULL) return;                                             \
                                                                          \
  pS = (uint16 *)g_fb -> bits;                                            \
  pD = (uint32 *)g_fb -> LCD_bits;                                        \
                                                                          \
  /* whats a "valid" box area for this operation? */                      \
  x1 = region -> x;                                                       \
  y1 = region -> y;                                                       \
  x2 = MIN(g_fb -> width,  region -> x + region -> width);                \
  y2 = MIN(g_fb -> height, region -> y + region -> height);               \
                                                                          \
  /* whats a "valid" box area for this operation? - destination window */ \
  if ((scr_x+(x2-x1)) > g_fb -> width)                                    \
    { x2=x1 + (g_fb -> width  - scr_x); }                                 \
  if ((scr_y+(y2-y1)) > g_fb -> height)                                   \
    { y2=y1 + (g_fb -> height - scr_y); }                                 \
                                                                          \
  /* do we still have a valid region? */                                  \
  if ((x2 > x1) && (y2 > y1))                                             \
  {                                                                       \
    pS += (((uint32)y1    * (g_fb -> rowBytes     >> 1)) + x1);           \
    pD += (((uint32)scr_y * (g_fb -> LCD_rowBytes >> 2)) + scr_x);        \
                                                                          \
    /* if the LCD window != same size as _FB window, center it */         \
    pD +=  (g_fb -> LCD_width  - g_fb -> width)  >> 1;                    \
    pD += ((g_fb -> LCD_height - g_fb -> height) >> 1) *                  \
           (g_fb -> LCD_rowBytes >> 2);                                   \
                                                                          \
    cnt = y2-y1;                                                          \
    pix = x2-x1;                                                          \
                                                                          \
    win_delta = (g_fb -> rowBytes     >> 1) - pix;                        \
    lcd_delta = (g_fb -> LCD_rowBytes >> 2) - pix;                        \
                                                                          \
    /* do each line... */                                                 \
    do                                                                    \
    {                                                                     \
      i = pix;                                                            \
      do                                                                  \
      {                                                                   \
        val = *pS++;                                                      \
        _FB_ ## FB ## _GetRGBFromIndex(val, &r, &g, &b);                  \
        *pD++ = (uint32)_FB_ ## LCD ## _GetPaletteIndex(r, g, b);         \
      } while (--i);                                                      \
                                                                          \
      pS += win_delta;                                                    \
      pD += lcd_delta;                                                    \
    }                                                                     \
    while (--cnt);                                                        \
  }                                                                       \
}

// this is a optimized conversion routine
#define _FB_X_D16_CopyToDisplayM(FB, LCD, RM, RS, GM, GS, BM, BS, AM)     \
static void                                                               \
_FB_ ## FB ## _ ## LCD ## _CopyToDisplay(fb_region *region,               \
                                           uint16 scr_x, uint16 scr_y)    \
{                                                                         \
  uint16      *pD, *pS;                                                   \
  int16        cnt, pix, i, win_delta, lcd_delta;                         \
  color        val;                                                       \
  uint16       x1, y1, x2, y2;                                            \
  FB_GLOBALS_ACCESS;                                                      \
                                                                          \
  /* entry condition */                                                   \
  if (region == NULL) return;                                             \
                                                                          \
  pS = (uint16 *)g_fb -> bits;                                            \
  pD = (uint16 *)g_fb -> LCD_bits;                                        \
                                                                          \
  /* whats a "valid" box area for this operation? */                      \
  x1 = region -> x;                                                       \
  y1 = region -> y;                                                       \
  x2 = MIN(g_fb -> width,  region -> x + region -> width);                \
  y2 = MIN(g_fb -> height, region -> y + region -> height);               \
                                                                          \
  /* whats a "valid" box area for this operation? - destination window */ \
  if ((scr_x+(x2-x1)) > g_fb -> width)                                    \
    { x2=x1 + (g_fb -> width  - scr_x); }                                 \
  if ((scr_y+(y2-y1)) > g_fb -> height)                                   \
    { y2=y1 + (g_fb -> height - scr_y); }                                 \
                                                                          \
  /* do we still have a valid region? */                                  \
  if ((x2 > x1) && (y2 > y1))                                             \
  {                                                                       \
    pS += (((uint32)y1    * (g_fb -> rowBytes      >> 1)) + x1);          \
    pD += (((uint32)scr_y * (g_fb -> LCD_rowBytes >> 1)) + scr_x);        \
                                                                          \
    /* if the LCD window != same size as _FB window, center it */         \
    pD +=  (g_fb -> LCD_width  - g_fb -> width)  >> 1;                    \
    pD += ((g_fb -> LCD_height - g_fb -> height) >> 1) *                  \
           (g_fb -> LCD_rowBytes >> 1);                                   \
                                                                          \
    cnt = y2-y1;                                                          \
    pix = x2-x1;                                                          \
                                                                          \
    win_delta = (g_fb -> rowBytes     >> 1) - pix;                        \
    lcd_delta = (g_fb -> LCD_rowBytes >> 1) - pix;                        \
                                                                          \
    /* do each line... */                                                 \
    do                                                                    \
    {                                                                     \
      i = pix;                                                            \
      do                                                                  \
      {                                                                   \
        val = *pS++;                                                      \
        *pD++ = (uint16)(((val & RM) RS) |                                \
                         ((val & GM) GS) |                                \
                         ((val & BM) BS) | AM);                           \
      } while (--i);                                                      \
                                                                          \
      pS += win_delta;                                                    \
      pD += lcd_delta;                                                    \
    }                                                                     \
    while (--cnt);                                                        \
  }                                                                       \
}

// 4:4:4 - 16 bit display (0x0f00, 0x00f0, 0x000f)

  _FB_X_D16_CopyToDisplayM(4444, ARGB4444, \
                           0x0f00,<< 0,0x00f0,<< 0,0x000f,<< 0,0xf000)
  _FB_X_D16_CopyToDisplayM(4444, ABGR4444, \
                           0x0f00,>> 8,0x00f0,<< 0,0x000f,<< 8,0xf000)
  _FB_X_D16_CopyToDisplayM(4444, RGBA4444, \
                           0x0f00,<< 4,0x00f0,<< 4,0x000f,<< 4,0x000f)
  _FB_X_D16_CopyToDisplayM(4444, BGRA4444, \
                           0x0f00,>> 4,0x00f0,<< 8,0x000f,<<12,0x000f)
  _FB_X_D16_CopyToDisplay(4444, ARGB1555)
  _FB_X_D16_CopyToDisplay(4444, ABGR1555)
  _FB_X_D16_CopyToDisplay(4444, RGBA5551)
  _FB_X_D16_CopyToDisplay(4444, BGRA5551)
  _FB_X_D16_CopyToDisplay(4444, RGB565)
  _FB_X_D16_CopyToDisplay(4444, BGR565)
  _FB_X_D32_CopyToDisplay(4444, ARGB8888)
  _FB_X_D32_CopyToDisplay(4444, ABGR8888)
  _FB_X_D32_CopyToDisplay(4444, RGBA8888)
  _FB_X_D32_CopyToDisplay(4444, BGRA8888)

// 5:5:5 - 16 bit display (0x7c00, 0x03e0, 0x001f)

  _FB_X_D16_CopyToDisplayM(1555, ARGB4444, \
                           0x7800,>> 3,0x03c0,>> 2,0x0017,>> 1, 0x8000)
  _FB_X_D16_CopyToDisplayM(1555, ABGR4444, \
                           0x7800,<< 7,0x03c0,>> 2,0x0017,>>11, 0x8000)
  _FB_X_D16_CopyToDisplayM(1555, RGBA4444, \
                           0x7800,<< 1,0x03c0,<< 2,0x0017,<< 3, 0x8000)
  _FB_X_D16_CopyToDisplayM(1555, BGRA4444, \
                           0x7800,<<11,0x03c0,<< 2,0x0017,>> 2, 0x8000)
  _FB_X_D16_CopyToDisplayM(1555, ARGB1555, \
                           0x7c00,<< 0,0x03e0,<< 0,0x001f,<< 0, 0x8000)
  _FB_X_D16_CopyToDisplayM(1555, ABGR1555, \
                           0x7c00,>>10,0x03e0,<< 0,0x001f,<<10, 0x8000)
  _FB_X_D16_CopyToDisplayM(1555, RGBA5551, \
                           0x7c00,<< 1,0x03e0,<< 1,0x001f,<< 1, 0x8000)
  _FB_X_D16_CopyToDisplayM(1555, BGRA5551, \
                           0x7c00,>> 9,0x03e0,<< 1,0x001f,<<11, 0x8000)
  _FB_X_D16_CopyToDisplay(1555, RGB565)
  _FB_X_D16_CopyToDisplay(1555, BGR565)
  _FB_X_D32_CopyToDisplay(1555, ARGB8888)
  _FB_X_D32_CopyToDisplay(1555, ABGR8888)
  _FB_X_D32_CopyToDisplay(1555, RGBA8888)
  _FB_X_D32_CopyToDisplay(1555, BGRA8888)

// 5:6:5 - 16 bit display (0xf800, 0x07e0, 0x001f)

  _FB_X_D16_CopyToDisplayM(565, ARGB4444, \
                           0xf000,>> 4,0x0780,>> 3,0x001e,>> 1, 0xf000)
  _FB_X_D16_CopyToDisplayM(565, ABGR4444, \
                           0xf000,>>12,0x0780,>> 3,0x001e,<< 7, 0xf000)
  _FB_X_D16_CopyToDisplayM(565, RGBA4444, \
                           0xf000,<< 0,0x0780,<< 1,0x001e,<< 3, 0x000f) 
  _FB_X_D16_CopyToDisplayM(565, BGRA4444, \
                           0xf000,>> 8,0x0780,<< 1,0x001e,<<11, 0x000f) 
  _FB_X_D16_CopyToDisplayM(565, ARGB1555, \
                           0xf800,>> 1,0x07c0,>> 1,0x001f,<< 0, 0x8000)
  _FB_X_D16_CopyToDisplayM(565, ABGR1555, \
                           0xf800,>>11,0x07c0,>> 1,0x001f,<<10, 0x8000)
  _FB_X_D16_CopyToDisplayM(565, RGBA5551, \
                           0xf800,<< 0,0x07c0,<< 0,0x001f,<< 1, 0x0001)
  _FB_X_D16_CopyToDisplayM(565, BGRA5551, \
                           0xf800,>>10,0x07c0,<< 0,0x001f,<<11, 0x0001)
//_FB_X_D16_CopyToDisplay(565, RGB565):: _FB_D16_D16_CopyToDisplay
  _FB_X_D16_CopyToDisplayM(565, BGR565,   \
                           0xf800,>>11,0x07e0,<< 0,0x001f,<<11, 0x0000)
  _FB_X_D32_CopyToDisplay(565, ARGB8888)
  _FB_X_D32_CopyToDisplay(565, ABGR8888)
  _FB_X_D32_CopyToDisplay(565, RGBA8888)
  _FB_X_D32_CopyToDisplay(565, BGRA8888)

static void
_FB_D16_D16_CopyToDisplay(fb_region *region, uint16 scr_x, uint16 scr_y)
{
  uint16 *pD, *pS;
  int16   cnt, pix, win_delta, lcd_delta;
  uint16  x1, y1, x2, y2;
  FB_GLOBALS_ACCESS;

  // entry condition
  if (region == NULL) return;

  pS = (uint16 *)g_fb -> bits;
  pD = (uint16 *)g_fb -> LCD_bits;

  // whats a "valid" box area for this operation? 
  x1 = region -> x;
  y1 = region -> y;
  x2 = MIN(g_fb -> width,  region -> x + region -> width);
  y2 = MIN(g_fb -> height, region -> y + region -> height);

  // whats a "valid" box area for this operation? - destination window 
  if ((scr_x+(x2-x1)) > g_fb -> width)  { x2=x1 + (g_fb -> width  - scr_x); }
  if ((scr_y+(y2-y1)) > g_fb -> height) { y2=y1 + (g_fb -> height - scr_y); }

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (g_fb -> rowBytes     >> 1)) + x1);
    pD += (((uint32)scr_y * (g_fb -> LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _FB window, center it
    pD +=  (g_fb -> LCD_width  - g_fb -> width)  >> 1;
    pD += ((g_fb -> LCD_height - g_fb -> height) >> 1) *
           (g_fb -> LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (g_fb -> rowBytes     >> 1);
    lcd_delta = (g_fb -> LCD_rowBytes >> 1);

    // do each line...
    do
    {
      _MemMove(pD, pS, pix);
      pD += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

/********************************* EOF ***********************************/
