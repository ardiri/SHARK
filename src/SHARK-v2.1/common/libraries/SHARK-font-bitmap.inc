/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-font-bitmap.inc
 */

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

#define MAX_BASE_CHARS          256
#define MAX_EXTENDED_CHARS      256
#define MAX_CHARS               (MAX_BASE_CHARS + MAX_EXTENDED_CHARS)

#define MAX_GREY_LEVELS         2

#define MAX_GLYPH_WIDTH         50
#define MAX_GLYPH_HEIGHT        70

// -----------------------------------------------------------------------
// compile time options
// -----------------------------------------------------------------------

// FNT_USE_COMMON_GLYPH
//
// _FntLoadGlyph(...)
// _FntReleaseGlyph(...)
//   -> used to reduce the amount of overhead having to allocate/free
//      memory chunks related to the glyph production for the textual
//      renderer.
//
//      = 0 - means create seperate gfx_windows for every glyph
//      = 1 - means share a common pre-allocated (large) gfx_window
//
// NOTE: due to the nature of the _FntDrawString/_FntDrawChar routines
//       there is never a single glyph loaded more than once in memory.
//       this may change in future - but, currently reduces the amount
//       of memory allocation/free calls during textual renderering.

   #define FNT_USE_COMMON_GLYPH    1

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct 
{
  uint16      width;
  uint16      height;
  uint16      depth;
  uint16      chr_max;
  uint8       widths[MAX_CHARS];
  coord       offset[MAX_CHARS];
  uint16      lookup[MAX_EXTENDED_CHARS][2];

  gfx_window *win;

#if (FNT_USE_COMMON_GLYPH == 1)
  gfx_window *glyph;
#endif
} _font_info;

typedef struct 
{
  color       foreColor;
  color       grayColors[MAX_GREY_LEVELS];
} _Fnt_B_Globals;

#define FNT_GLOBALS_B_ACCESS \
  _FntGlobals     *g_fnt; \
  _Fnt_B_Globals  *g_b_fnt; \
  g_fnt   = (_FntGlobals *)_LibGetGlobals(FNT_LIBRARY_ID); \
  g_b_fnt = (_Fnt_B_Globals *)g_fnt->extension;

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibGetGlobals() API.
//------------------------------------------------------------------------

/*************************************************************************
 *
 * Generic _Fnt routines
 *
 *************************************************************************/

static boolean
_FntRemapGlyph(uint32 *chr)
{
  boolean     result;
  _font_info *f_info;
  uint32      i;
  FNT_GLOBALS_B_ACCESS;

  // default return value
  result = false;

  // get a reference to the current font handle
  f_info = (_font_info *)g_fnt->fnt_info[g_fnt->fnt].extension;

  // entry requirments
  if ((f_info == NULL) || 
      (f_info->chr_max < MAX_BASE_CHARS)) return result;

  // we need to find our character
  i = 0;
  while (i < (uint8)(f_info->chr_max - MAX_BASE_CHARS))
  {
    // did we find it?
    if (f_info->lookup[i][0] == *chr)
    {
      // do our 'cross reference'
      result = true;
      *chr   = f_info->lookup[i][1];
      break;
    }
    i++;
  }

  // debugging - if not found, remap to chr(1)
  if (!result) { *chr = 1; result = true; }  // should display a []

  return result;
}

static boolean
_Fnt_B_Initialize()
{
  boolean         init_ok;
  _Fnt_B_Globals *g_b_fnt;
  FNT_GLOBALS_ACCESS;

  // assume everything is ok
  init_ok = true;

  // allocate globals memory for the library
  g_b_fnt = (_Fnt_B_Globals *)_MemPtrNew(sizeof(_Fnt_B_Globals), false);
  init_ok &= (g_b_fnt != NULL); if (!init_ok) goto FNT_B_INIT_ABORT;
  g_fnt->extension = (void *)g_b_fnt;

FNT_B_INIT_ABORT:

  return init_ok;
}

static void
_Fnt_B_Terminate()
{
  FNT_GLOBALS_B_ACCESS;

  // clean up
  if (g_b_fnt != NULL) _MemPtrFree(g_b_fnt);
  g_fnt->extension = NULL;
}

static boolean
_Fnt_B_DefineFont(font id, void *fontData)
{
  boolean     result;
  _font_info *f_info;
  uint8      *p;
  uint32      i;
  lcd_format  format;
  uint16      ofs, cnt;
  FNT_GLOBALS_B_ACCESS;

  // default return value
  result = false;

  // entry requirements
  if (fontData == NULL) return result;

  // entry requirements (check for FNT signature)
  p = (uint8 *)fontData;
  if ((p[0] != 'F') || (p[1] != 'N') || (p[2] != 'T')) return result;
  p += 3;

  // we can only handle MAX_CHARS (hardcoded above)
  cnt = READ_16_BE(&p[5]);
  if (cnt > MAX_CHARS) return result;

  // get a reference to the current font handle
  f_info = _MemPtrNew(sizeof(_font_info), false);
  if (f_info != NULL)
  {
    g_fnt->fnt_info[id].extension = (void *)f_info;

    // basic information
    f_info->depth   = p[0] - '0';
    f_info->width   = READ_16_BE(&p[1]);
    f_info->height  = READ_16_BE(&p[3]);
    f_info->chr_max = cnt;
    p += 7;

    // copy the width information for all the characters
    _MemMove(f_info->widths, p, f_info->chr_max);
    p += f_info->chr_max;

    // load the lookup table for the unicode characters
    if (f_info->chr_max > MAX_BASE_CHARS)
    {
      for (i=0; i<(uint8)(f_info->chr_max-MAX_BASE_CHARS); i++)
      {
        f_info->lookup[i][0] = READ_16_BE(&p[0]);
        f_info->lookup[i][1] = READ_16_BE(&p[2]);
        p += 4;
      }
    }
    result = true;

    // calculate the "offsets" for the copying from bitmap window
    ofs = 0;
    for (i=0; i<f_info->chr_max; i++)
    {
      f_info->offset[i] = ofs;
      ofs += f_info->widths[i];
    }

    // we need to create a gfx_window based on these properties now
    _MemSet(&format, 0, sizeof(lcd_format));
    format.type  = lcd_palette;
    format.depth = f_info->depth;
    format.data.palette.size = 1 << format.depth;
    format.data.palette.rgb  = NULL;  // force grayscale
    f_info->win = (gfx_window *)
      _GfxCreateWindowFromBitmapStream((coord)f_info->width, 
                                       (coord)f_info->height, 
                                       &format, (uint8 *)p);
    result &= (f_info->win != NULL);

#if (FNT_USE_COMMON_GLYPH == 1)
    // our scratch glyph area
    f_info->glyph = (gfx_window *)
      _GfxCreateWindow(MAX_GLYPH_WIDTH, MAX_GLYPH_HEIGHT);
    result &= (f_info->glyph != NULL);
#endif
  }

  return result;
}

static void
_Fnt_B_ReleaseFont(font id)
{
  _font_info *f_info;
  FNT_GLOBALS_B_ACCESS;

  // get a reference to the current font handle
  f_info = (_font_info *)g_fnt->fnt_info[id].extension;

  // entry requirments
  if (f_info == NULL) return;

  // clean up
#if (FNT_USE_COMMON_GLYPH == 1)
  if (f_info->glyph != NULL) _GfxDeleteWindow(f_info->glyph); 
  f_info->glyph = NULL;
#endif
  if (f_info->win   != NULL) _GfxDeleteWindow(f_info->win);
  f_info->win = NULL;
  _MemPtrFree(f_info); f_info = NULL;
  g_fnt->fnt_info[id].extension = (void *)f_info;
}

static void
_Fnt_B_GetGlyphProperties(uint32 chr, uint32 *width, uint32 *height)
{
  _font_info *f_info;
  FNT_GLOBALS_B_ACCESS;

  // get a reference to the current font handle
  f_info = (_font_info *)g_fnt->fnt_info[g_fnt->fnt].extension;

  // entry requirments
  if (f_info == NULL) return;

  // lets make sure we can process this chr
  if ((chr > (MAX_BASE_CHARS-1)) && (!_FntRemapGlyph(&chr))) return;

  // return the necessary pieces of information
  if (height != NULL) *height = f_info->height;
  if (width  != NULL) *width  = f_info->widths[chr];
}

static void
_Fnt_B_ReleaseGlyph(gfx_window *glyph)
{
  FNT_GLOBALS_B_ACCESS;

#if (FNT_USE_COMMON_GLYPH == 0)
  if (glyph != NULL) _GfxDeleteWindow(glyph);
#endif
}

/*************************************************************************
 *
 * _Fnt SetColor implementations
 *
 *************************************************************************/

static void
_Fnt_B_SetColor(color c)
{
  int32  i;
  uint8  r,  g,  b;
  uint8 dr, dg, db;
  FNT_GLOBALS_B_ACCESS;

  // fore-ground and back-ground colors
  g_b_fnt->foreColor = c;

  // whats the r,b,g value for this color?
  _GfxGetRGBFromIndex(c, &r, &g, &b);

  // linear scale:
  //  ie: 0,0,0 --> 85,85,85 -> 170,170,170 -> 255,255,255

  dr = (255 - r) / (MAX_GREY_LEVELS + 1);
  dg = (255 - g) / (MAX_GREY_LEVELS + 1);
  db = (255 - b) / (MAX_GREY_LEVELS + 1);

  for (i=0; i<MAX_GREY_LEVELS; i++)
  {
    r += dr; g += dg; b += db;
    g_b_fnt->grayColors[i] = _GfxGetPaletteIndex(r,g,b);
  }
}

/*************************************************************************
 *
 * _Fnt LoadGlyph implementations
 *
 *************************************************************************/

#if (FNT_USE_COMMON_GLYPH == 1)
typedef struct 
{
  coord   width;
  coord   height;
  // .. (more - but, we dont need to know)  -->> taken from SHARK-gfx.c
} _gfx_window;
#endif

static gfx_window *
_Fnt_B_LoadGlyph(uint32 chr, rectangle *rect)
{
  gfx_window *result;
  _font_info *f_info;
  FNT_GLOBALS_B_ACCESS;

  // default return value
  result = NULL;

  // get a reference to the current font handle
  f_info = (_font_info *)g_fnt->fnt_info[g_fnt->fnt].extension;

  // lets make sure we can process this chr
  if ((chr > (MAX_BASE_CHARS-1)) && (!_FntRemapGlyph(&chr))) return result;

  // initialize the rectangle bounds of the glyph
  rect->x      = 0;
  rect->y      = 0;
  rect->width  = (coord)f_info->widths[chr];
  rect->height = (coord)f_info->height;

  // create the window for the glyph
#if (FNT_USE_COMMON_GLYPH == 1)
  result = (gfx_window *)f_info->glyph;
  ((_gfx_window *)result)->width  = rect->width;
  ((_gfx_window *)result)->height = rect->height;  // override these
#else
  result = _GfxCreateWindow(rect->width, rect->height);
#endif
  if (result != NULL)
  {
    // this will copy the basic version of the bitmap (mono and/or gray)
    rect->x = (coord)f_info->offset[chr];
    _GfxCopyRegion(f_info->win, result, rect, 0, 0, gfxPaint);
    rect->x = 0;
  }

  return result;
}

static gfx_window *
_Fnt_B_I8M_LoadGlyph(uint32 chr, rectangle *rect)
{
  gfx_window *result;
  _font_info *f_info;
  uint8       *pGlyph;
  coord       cnt_x, cnt_y;
  uint16      rowBytes;
  FNT_GLOBALS_B_ACCESS;

  // default return value
  result = NULL;

  // get a reference to the current font handle
  f_info = (_font_info *)g_fnt->fnt_info[g_fnt->fnt].extension;

  // entry requirements
  if ((f_info == NULL) || (rect == NULL)) return result;

  // lets do the basic gfx_window initialization
  result = _Fnt_B_LoadGlyph(chr, rect);
  if ((result != NULL) && (g_b_fnt->foreColor != _GfxGetPaletteIndex(0,0,0)))
  {
    color lookup;
    lookup = _GfxGetPaletteIndex(0,0,0);

    // we need to go through the bitmap; and, replace with color version
    _GfxGetWindowInfo(result, (void *)&pGlyph, &cnt_x, &cnt_y, &rowBytes);

    // remap the grayscale values to appopriate colors
    do
    {
      cnt_x = rect->width;
      do
      {
        if (*pGlyph == lookup) *pGlyph = (uint8)g_b_fnt->foreColor;
        pGlyph++;
      }
      while (--cnt_x);
      pGlyph += (rowBytes - rect->width);
    }
    while (--cnt_y);
  }

  return result;
}

static gfx_window *
_Fnt_B_I8A_LoadGlyph(uint32 chr, rectangle *rect)
{
  gfx_window *result;
  _font_info *f_info;
  uint8      *pGlyph;
  coord       cnt_x, cnt_y;
  uint16      rowBytes;
  FNT_GLOBALS_B_ACCESS;

  // default return value
  result = NULL;

  // get a reference to the current font handle
  f_info = (_font_info *)g_fnt->fnt_info[g_fnt->fnt].extension;

  // entry requirements
  if ((f_info == NULL) || (rect == NULL)) return result;

  // lets do the basic gfx_window initialization
  result = _Fnt_B_LoadGlyph(chr, rect);
  if ((result != NULL) && (g_b_fnt->foreColor != _GfxGetPaletteIndex(0,0,0)))
  {
    color lookup1, lookup2, lookup3;
    lookup1 = _GfxGetPaletteIndex(0,0,0);
    lookup2 = _GfxGetPaletteIndex(85,85,85);
    lookup3 = _GfxGetPaletteIndex(170,170,170);

    // we need to go through the bitmap; and, replace with color version
    _GfxGetWindowInfo(result, (void *)&pGlyph, &cnt_x, &cnt_y, &rowBytes);

    // remap the grayscale values to appopriate colors
    do
    {
      cnt_x = rect->width;
      do
      {
             if (*pGlyph == lookup1) *pGlyph = (uint8)g_b_fnt->foreColor;
        else if (*pGlyph == lookup2) *pGlyph = (uint8)g_b_fnt->grayColors[0];
        else if (*pGlyph == lookup3) *pGlyph = (uint8)g_b_fnt->grayColors[1];
        pGlyph++;
      }
      while (--cnt_x);
      pGlyph += (rowBytes - rect->width);
    }
    while (--cnt_y);
  }

  return result;
}

// the generic I8 function
static gfx_window * 
_Fnt_B_I8_LoadGlyph(uint32 chr, rectangle *rect)
{
  gfx_window *result;
  _font_info *f_info;
  FNT_GLOBALS_B_ACCESS;

  // get a reference to the current font handle
  f_info = (_font_info *)g_fnt->fnt_info[g_fnt->fnt].extension;

  if (f_info->depth == 1)
    result = _Fnt_B_I8M_LoadGlyph(chr, rect);
  else
    result = _Fnt_B_I8A_LoadGlyph(chr, rect);

  return result;
}

static gfx_window *
_Fnt_B_D16M_LoadGlyph(uint32 chr, rectangle *rect)
{
  gfx_window *result;
  _font_info *f_info;
  uint16     *pGlyph;
  coord       cnt_x, cnt_y;
  uint16      rowBytes;
  FNT_GLOBALS_B_ACCESS;

  // default return value
  result = NULL;

  // get a reference to the current font handle
  f_info = (_font_info *)g_fnt->fnt_info[g_fnt->fnt].extension;

  // entry requirements
  if ((f_info == NULL) || (rect == NULL)) return result;


  // lets do the basic gfx_window initialization
  result = _Fnt_B_LoadGlyph(chr, rect);
  if ((result != NULL) && (g_b_fnt->foreColor != _GfxGetPaletteIndex(0,0,0)))
  {
    color lookup;
    lookup = _GfxGetPaletteIndex(0,0,0);

    // we need to go through the bitmap; and, replace with color version
    _GfxGetWindowInfo(result, (void *)&pGlyph, &cnt_x, &cnt_y, &rowBytes);

    // remap the grayscale values to appopriate colors
    do
    {
      cnt_x = rect->width;
      do
      {
        if (*pGlyph == lookup) *pGlyph = (uint16)g_b_fnt->foreColor;
        pGlyph++;
      }
      while (--cnt_x);
      pGlyph += ((rowBytes >> 1) - rect->width);
    }
    while (--cnt_y);
  }

  return result;
}

static gfx_window *
_Fnt_B_D16A_LoadGlyph(uint32 chr, rectangle *rect)
{
  gfx_window *result;
  _font_info *f_info;
  uint16     *pGlyph;
  coord       cnt_x, cnt_y;
  uint16      rowBytes;
  FNT_GLOBALS_B_ACCESS;

  // default return value
  result = NULL;

  // get a reference to the current font handle
  f_info = (_font_info *)g_fnt->fnt_info[g_fnt->fnt].extension;

  // entry requirements
  if ((f_info == NULL) || (rect == NULL)) return result;

  // lets do the basic gfx_window initialization
  result = _Fnt_B_LoadGlyph(chr, rect);
  if ((result != NULL) && (g_b_fnt->foreColor != _GfxGetPaletteIndex(0,0,0)))
  {
    color lookup1, lookup2, lookup3;
    lookup1 = _GfxGetPaletteIndex(0,0,0);
    lookup2 = _GfxGetPaletteIndex(85,85,85);
    lookup3 = _GfxGetPaletteIndex(170,170,170);

    // we need to go through the bitmap; and, replace with color version
    _GfxGetWindowInfo(result, (void *)&pGlyph, &cnt_x, &cnt_y, &rowBytes);

    // remap the grayscale values to appopriate colors
    do
    {
      cnt_x = rect->width;
      do
      {
             if (*pGlyph == lookup1) *pGlyph = (uint16)g_b_fnt->foreColor;
        else if (*pGlyph == lookup2) *pGlyph = (uint16)g_b_fnt->grayColors[0];
        else if (*pGlyph == lookup3) *pGlyph = (uint16)g_b_fnt->grayColors[1];
        pGlyph++;
      }
      while (--cnt_x);
      pGlyph += ((rowBytes >> 1) - rect->width);
    }
    while (--cnt_y);
  }

  return result;
}

// the generic D16 function
static gfx_window * 
_Fnt_B_D16_LoadGlyph(uint32 chr, rectangle *rect)
{
  gfx_window *result;
  _font_info *f_info;
  FNT_GLOBALS_B_ACCESS;

  // get a reference to the current font handle
  f_info = (_font_info *)g_fnt->fnt_info[g_fnt->fnt].extension;

  if (f_info->depth == 1)
    result = _Fnt_B_D16M_LoadGlyph(chr, rect);
  else
    result = _Fnt_B_D16A_LoadGlyph(chr, rect);

  return result;
}

/********************************* EOF ***********************************/
