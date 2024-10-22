/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-fb-i8.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

#define PALETTE_SIZE            256

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

typedef struct _FB_I8_Globals
{
  uint8  palette[PALETTE_SIZE * 3];           // raw palette
  uint32 palette_diff[PALETTE_SIZE];          // lookup table 

  // volatile information (just stored here for optimization)
  uint16 palette_d16[PALETTE_SIZE];           // lookup table for direct LCD
  uint32 palette_d32[PALETTE_SIZE];           // lookup table for direct LCD

} _FB_I8_Globals;

#define FB_GLOBALS_I8_ACCESS \
  _FBGlobals     *g_fb; \
  _FB_I8_Globals *g_i8_fb; \
  g_fb    = (_FBGlobals *)_LibGetGlobals(FB_LIBRARY_ID); \
  g_i8_fb = (_FB_I8_Globals *)g_fb -> extension;
#define GLOBALS_I8_UNINITIALIZED (g_i8_fb == NULL)

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

const uint8 fb_i8_sys_palette[PALETTE_SIZE * 3] =
{
  255, 255, 255, 255, 204, 255, 255, 153, 255, 255, 102, 255,
  255,  51, 255, 255,   0, 255, 255, 255, 204, 255, 204, 204,
  255, 153, 204, 255, 102, 204, 255,  51, 204, 255,   0, 204,
  255, 255, 153, 255, 204, 153, 255, 153, 153, 255, 102, 153,
  255,  51, 153, 255,   0, 153, 204, 255, 255, 204, 204, 255,
  204, 153, 255, 204, 102, 255, 204,  51, 255, 204,   0, 255,
  204, 255, 204, 204, 204, 204, 204, 153, 204, 204, 102, 204,
  204,  51, 204, 204,   0, 204, 204, 255, 153, 204, 204, 153,
  204, 153, 153, 204, 102, 153, 204,  51, 153, 204,   0, 153,
  153, 255, 255, 153, 204, 255, 153, 153, 255, 153, 102, 255,
  153,  51, 255, 153,   0, 255, 153, 255, 204, 153, 204, 204,
  153, 153, 204, 153, 102, 204, 153,  51, 204, 153,   0, 204,
  153, 255, 153, 153, 204, 153, 153, 153, 153, 153, 102, 153,
  153,  51, 153, 153,   0, 153, 102, 255, 255, 102, 204, 255,
  102, 153, 255, 102, 102, 255, 102,  51, 255, 102,   0, 255,
  102, 255, 204, 102, 204, 204, 102, 153, 204, 102, 102, 204,
  102,  51, 204, 102,   0, 204, 102, 255, 153, 102, 204, 153,
  102, 153, 153, 102, 102, 153, 102,  51, 153, 102,   0, 153,
   51, 255, 255,  51, 204, 255,  51, 153, 255,  51, 102, 255,
   51,  51, 255,  51,   0, 255,  51, 255, 204,  51, 204, 204,
   51, 153, 204,  51, 102, 204,  51,  51, 204,  51,   0, 204,
   51, 255, 153,  51, 204, 153,  51, 153, 153,  51, 102, 153,
   51,  51, 153,  51,   0, 153,   0, 255, 255,   0, 204, 255,
    0, 153, 255,   0, 102, 255,   0,  51, 255,   0,   0, 255,
    0, 255, 204,   0, 204, 204,   0, 153, 204,   0, 102, 204,
    0,  51, 204,   0,   0, 204,   0, 255, 153,   0, 204, 153,
    0, 153, 153,   0, 102, 153,   0,  51, 153,   0,   0, 153,
  255, 255, 102, 255, 204, 102, 255, 153, 102, 255, 102, 102,
  255,  51, 102, 255,   0, 102, 255, 255,  51, 255, 204,  51,
  255, 153,  51, 255, 102,  51, 255,  51,  51, 255,   0,  51,
  255, 255,   0, 255, 204,   0, 255, 153,   0, 255, 102,   0,
  255,  51,   0, 255,   0,   0, 204, 255, 102, 204, 204, 102,
  204, 153, 102, 204, 102, 102, 204,  51, 102, 204,   0, 102,
  204, 255,  51, 204, 204,  51, 204, 153,  51, 204, 102,  51,
  204,  51,  51, 204,   0,  51, 204, 255,   0, 204, 204,   0,
  204, 153,   0, 204, 102,   0, 204,  51,   0, 204,   0,   0,
  153, 255, 102, 153, 204, 102, 153, 153, 102, 153, 102, 102,
  153,  51, 102, 153,   0, 102, 153, 255,  51, 153, 204,  51,
  153, 153,  51, 153, 102,  51, 153,  51,  51, 153,   0,  51,
  153, 255,   0, 153, 204,   0, 153, 153,   0, 153, 102,   0,
  153,  51,   0, 153,   0,   0, 102, 255, 102, 102, 204, 102,
  102, 153, 102, 102, 102, 102, 102,  51, 102, 102,   0, 102,
  102, 255,  51, 102, 204,  51, 102, 153,  51, 102, 102,  51,
  102,  51,  51, 102,   0,  51, 102, 255,   0, 102, 204,   0,
  102, 153,   0, 102, 102,   0, 102,  51,   0, 102,   0,   0,
   51, 255, 102,  51, 204, 102,  51, 153, 102,  51, 102, 102,
   51,  51, 102,  51,   0, 102,  51, 255,  51,  51, 204,  51,
   51, 153,  51,  51, 102,  51,  51,  51,  51,  51,   0,  51,
   51, 255,   0,  51, 204,   0,  51, 153,   0,  51, 102,   0,
   51,  51,   0,  51,   0,   0,   0, 255, 102,   0, 204, 102,
    0, 153, 102,   0, 102, 102,   0,  51, 102,   0,   0, 102,
    0, 255,  51,   0, 204,  51,   0, 153,  51,   0, 102,  51,
    0,  51,  51,   0,   0,  51,   0, 255,   0,   0, 204,   0,
    0, 153,   0,   0, 102,   0,   0,  51,   0,  17,  17,  17,
   34,  34,  34,  68,  68,  68,  85,  85,  85, 119, 119, 119,
  136, 136, 136, 170, 170, 170, 187, 187, 187, 221, 221, 221,
  238, 238, 238, 192, 192, 192, 128,   0,   0, 128,   0, 128,
    0, 128,   0,   0, 128, 128,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  255, 255, 255, 238, 238, 238, 221, 221, 221, 204, 204, 204,
  187, 187, 187, 170, 170, 170, 153, 153, 153, 136, 136, 136,
  119, 119, 119, 102, 102, 102,  85,  85,  85,  68,  68,  68,
   51,  51,  51,  34,  34,  34,  17,  17,  17,   0,   0,   0
};

static boolean
_FB_I8_Initialize()
{
  boolean          init_ok;
  _FB_I8_Globals *g_i8_fb;
  FB_GLOBALS_ACCESS;

  // assume everything is ok
  init_ok = true;

  // allocate globals memory for the library
  g_i8_fb = (_FB_I8_Globals *)_MemPtrNew(sizeof(_FB_I8_Globals), false);
  init_ok &= (g_i8_fb != NULL); if (!init_ok) goto FB_I8_INIT_ABORT;
  g_fb -> extension = (void *)g_i8_fb;

  // set the default palette (which, builds the lookup tables)
  _FBSetPalette((uint8 *)fb_i8_sys_palette);

FB_I8_INIT_ABORT:

  return init_ok;
}

static void
_FB_I8_Terminate()
{
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  // clean up
  _MemPtrFree(g_i8_fb);
  g_fb -> extension = NULL;
}

static void
_FB_I8_GetPalette(uint8 *rgb)
{
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  // entry condition
  if (rgb == NULL) return;

  // copy over palette entries
  _MemMove(rgb, g_i8_fb -> palette, (PALETTE_SIZE * 3));
}

static color
_FB_I8_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  uint8  *pal;
  int     i;
  color   result;
  uint32  lowValue, *diff, diffValue;
  int     rd, gd, bd;
  FB_GLOBALS_I8_ACCESS;

  // default value
  result = 0;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return result;

  // special purpose cases (these are important for _FBCopyRegion)
  if ((r == 255) && (g == 255) && (b == 255)) return 0;
  if ((r ==   0) && (g ==   0) && (b ==   0)) return 255;

  // generate the color "differences" for all colors in the palette
  pal  = g_i8_fb -> palette;
  diff = g_i8_fb -> palette_diff;
  i    = PALETTE_SIZE;
  do
  {
    bd = ((int)*pal++ - r);
    gd = ((int)*pal++ - g);
    rd = ((int)*pal++ - b);

    *diff++ = ((int32)rd * rd) + ((int32)gd * gd) + ((int32)bd * bd);
  }
  while (--i);

  // find the palette index that has the smallest color "difference"
  i        = PALETTE_SIZE;
  result   = i-1;
  lowValue = 0xffffffff;
  do
  {
    diff--;
    diffValue = *diff;
    if (diffValue < lowValue)
    {
      lowValue = diffValue;
      result   = (i-1);
    }
  }
  while (--i);

  return result;
}

static void
_FB_I8_GetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  uint8 *pal;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  // entry condition
  if ((r == NULL) || (g == NULL) || (b == NULL)) return;

  pal  = g_i8_fb -> palette;
  pal += c * 3; // jump to the right color

  // extract the rgb values
  *r = *pal++;
  *g = *pal++;
  *b = *pal++;
}

static void
_FB_I8_SetPalette(uint8 *rgb)
{
  uint8 *_p;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  // copy over palette entries
  _MemMove(g_i8_fb -> palette, rgb, (PALETTE_SIZE * 3));

  // we must force 0=255,255,255 and 255=0,0,0
  _p = &g_i8_fb -> palette[0];   *_p++ = 255; *_p++ = 255; *_p++ = 255;
  _p = &g_i8_fb -> palette[765]; *_p++ = 0;   *_p++ = 0;   *_p++ = 0;

  // we need to re-generate the lookup tables now
  if (g_fb -> fnFBDisplayLookup != NULL)
    g_fb -> fnFBDisplayLookup();
}

/*************************************************************************
 * _FB CopyToDisplay implementations
 *************************************************************************/

static void
_FB_I8_D16_CopyToDisplay(fb_region *region, uint16 scr_x, uint16 scr_y)
{
  uint8  *pS;
  uint16 *pD, *pal;
  int16   cnt, pix, i, win_delta, lcd_delta;
  uint16  x1, y1, x2, y2;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  // entry condition
  if (region == NULL) return;

  pS = (uint8  *)g_fb -> bits;
  pD = (uint16 *)g_fb -> LCD_bits;

  // whats a "valid" box area for this operation?
  x1 = region -> x;
  y1 = region -> y;
  x2 = MIN(g_fb -> width,  region -> x + region -> width);
  y2 = MIN(g_fb -> height, region -> y + region -> height);

  // whats a "valid" box area for this operation? - destination window
  if ((scr_x+(x2-x1)) > g_fb -> width)  { x2=x1 + (g_fb -> width  - scr_x); }
  if ((scr_y+(y2-y1)) > g_fb -> height) { y2=y1 + (g_fb -> height - scr_y); }

  // get access to the palette lookup table
  pal  = g_i8_fb -> palette_d16;

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * g_fb -> rowBytes) + x1);
    pD += (((uint32)scr_y * (g_fb -> LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _FB LCD, center it
    pD +=  (g_fb -> LCD_width  - g_fb -> width)  >> 1;
    pD += ((g_fb -> LCD_height - g_fb -> height) >> 1) * 
           (g_fb -> LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = g_fb -> rowBytes            - pix;
    lcd_delta = (g_fb -> LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        *pD++ = *(pal + *pS++);
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

static void
_FB_I8_D32_CopyToDisplay(fb_region *region, uint16 scr_x, uint16 scr_y)
{
  uint8   *pS;
  uint32  *pD, *pal;
  int16   cnt, pix, i, win_delta, lcd_delta;
  uint16  x1, y1, x2, y2;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  // entry condition
  if (region == NULL) return;

  pS = (uint8  *)g_fb -> bits;
  pD = (uint32 *)g_fb -> LCD_bits;

  // whats a "valid" box area for this operation?
  x1 = region -> x;
  y1 = region -> y;
  x2 = MIN(g_fb -> width,  region -> x + region -> width);
  y2 = MIN(g_fb -> height, region -> y + region -> height);

  // whats a "valid" box area for this operation? - destination window
  if ((scr_x+(x2-x1)) > g_fb -> width)  { x2=x1 + (g_fb -> width  - scr_x); }
  if ((scr_y+(y2-y1)) > g_fb -> height) { y2=y1 + (g_fb -> height - scr_y); }

  // get access to the palette lookup table
  pal  = g_i8_fb -> palette_d32;

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * g_fb -> rowBytes) + x1);
    pD += (((uint32)scr_y * (g_fb -> LCD_rowBytes >> 2)) + scr_x);

    // if the LCD window != same size as _FB LCD, center it
    pD +=  (g_fb -> LCD_width  - g_fb -> width)  >> 1;
    pD += ((g_fb -> LCD_height - g_fb -> height) >> 1) * 
           (g_fb -> LCD_rowBytes >> 2);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = g_fb -> rowBytes            - pix;
    lcd_delta = (g_fb -> LCD_rowBytes >> 2) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        *pD++ = *(pal + *pS++);
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

/*************************************************************************
 * _FB DisplayLookup implementations
 *************************************************************************/

// 4:4:4 - 16 bit displays

static void
_FB_I8_ARGB4444_DisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  _p = g_i8_fb -> palette;
   p = g_i8_fb -> palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_FB_ARGB4444_GetPaletteIndex(r, g, b);
  }
}

static void
_FB_I8_ABGR4444_DisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  _p = g_i8_fb -> palette;
   p = g_i8_fb -> palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_FB_ABGR4444_GetPaletteIndex(r, g, b);
  }
}

static void
_FB_I8_RGBA4444_DisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  _p = g_i8_fb -> palette;
   p = g_i8_fb -> palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_FB_RGBA4444_GetPaletteIndex(r, g, b);
  }
}

static void
_FB_I8_BGRA4444_DisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  _p = g_i8_fb -> palette;
   p = g_i8_fb -> palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_FB_BGRA4444_GetPaletteIndex(r, g, b);
  }
}

// 5:5:5 - 16 bit displays

static void
_FB_I8_ARGB1555_DisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  _p = g_i8_fb -> palette;
   p = g_i8_fb -> palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_FB_ARGB1555_GetPaletteIndex(r, g, b);
  }
}

static void
_FB_I8_ABGR1555_DisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  _p = g_i8_fb -> palette;
   p = g_i8_fb -> palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_FB_ABGR1555_GetPaletteIndex(r, g, b);
  }
}

static void
_FB_I8_RGBA5551_DisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  _p = g_i8_fb -> palette;
   p = g_i8_fb -> palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_FB_RGBA5551_GetPaletteIndex(r, g, b);
  }
}

static void
_FB_I8_BGRA5551_DisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  _p = g_i8_fb -> palette;
   p = g_i8_fb -> palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_FB_BGRA5551_GetPaletteIndex(r, g, b);
  }
}

// 5:6:5 - 16 bit display

static void
_FB_I8_RGB565_DisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  _p = g_i8_fb -> palette;
   p = g_i8_fb -> palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_FB_RGB565_GetPaletteIndex(r, g, b);
  }
}

static void
_FB_I8_BGR565_DisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  _p = g_i8_fb -> palette;
   p = g_i8_fb -> palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_FB_BGR565_GetPaletteIndex(r, g, b);
  }
}

// 8:8:8 - 32 bit display

static void
_FB_I8_ARGB8888_DisplayLookup()
{
  uint8  *_p;
  uint32 *p;
  uint8   r, g, b;
  int     i;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  _p = g_i8_fb -> palette;
   p = g_i8_fb -> palette_d32;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint32)_FB_ARGB8888_GetPaletteIndex(r, g, b);
  }
}

static void
_FB_I8_ABGR8888_DisplayLookup()
{
  uint8  *_p;
  uint32 *p;
  uint8   r, g, b;
  int     i;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  _p = g_i8_fb -> palette;
   p = g_i8_fb -> palette_d32;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint32)_FB_ABGR8888_GetPaletteIndex(r, g, b);
  }
}

static void
_FB_I8_RGBA8888_DisplayLookup()
{
  uint8  *_p;
  uint32 *p;
  uint8   r, g, b;
  int     i;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  _p = g_i8_fb -> palette;
   p = g_i8_fb -> palette_d32;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint32)_FB_RGBA8888_GetPaletteIndex(r, g, b);
  }
}

static void
_FB_I8_BGRA8888_DisplayLookup()
{
  uint8  *_p;
  uint32 *p;
  uint8   r, g, b;
  int     i;
  FB_GLOBALS_I8_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_I8_UNINITIALIZED) return;

  _p = g_i8_fb -> palette;
   p = g_i8_fb -> palette_d32;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint32)_FB_BGRA8888_GetPaletteIndex(r, g, b);
  }
}

#undef FB_GLOBALS_I8_ACCESS
#undef GLOBALS_I8_UNINITIALIZED

#undef PALETTE_SIZE

/********************************* EOF ***********************************/
