/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-gfx-i8.inc
 */

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

#define PALETTE_SIZE            256

// -----------------------------------------------------------------------
// compile time options
// -----------------------------------------------------------------------

// GFX_ALPHABLEND_REALTIME
//
// _GfxCopyRegion(..., gfxTransparent)
//   -> used for overlaying text onto graphics, optimized specific for
//      anti-aliased text - non white/black colors are alpha-blended to
//      give a smoother overlay.
//
//      = 0 - means a 65536 table is generated every _GfxSetPalette()
//      = 1 - means the alpha-blending is done in realtime at pixel blit
//
// NOTE: antialiased text contains approprimately 2% which needs to be
//       adjusted based on this algorithm. the overhead of a table and
//       time used to generate table makes = 0 less preferred.

   #define GFX_ALPHABLEND_REALTIME 1      // alpha-blending (gfxTransparent)

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct _Gfx_I8_Globals
{
  uint8  palette[PALETTE_SIZE * 3];           // raw palette
  uint16 palette_d16[PALETTE_SIZE];           // lookup table for direct LCD
  uint32 palette_d32[PALETTE_SIZE];           // lookup table for direct LCD
  uint8  palette_invert[PALETTE_SIZE];        // best match for invert

#if (GFX_ALPHABLEND_REALTIME == 0)
  uint8  palette_blending[PALETTE_SIZE *
                          PALETTE_SIZE];      // gfxTransparent blending lookup
#endif

  // volatile information (just stored here for optimization)
  uint32 palette_diff[PALETTE_SIZE];          // lookup table for_GfxGetPaletteIndex
} _Gfx_I8_Globals;

#define GFX_GLOBALS_I8_ACCESS \
  _GfxGlobals     *g_gfx; \
  _Gfx_I8_Globals *g_i8_gfx; \
  g_gfx    = (_GfxGlobals *)_LibGetGlobals(GFX_LIBRARY_ID); \
  g_i8_gfx = (_Gfx_I8_Globals *)g_gfx->extension;

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibGetGlobals() API.
//------------------------------------------------------------------------

/*************************************************************************
 *
 * Generic _Gfx routines
 *
 *************************************************************************/

static boolean
_Gfx_I8_Initialize()
{
#include "SHARK-gfx-i8-palette.inc"

  boolean          init_ok;
  _Gfx_I8_Globals *g_i8_gfx;
  GFX_GLOBALS_ACCESS;

  // assume everything is ok
  init_ok = true;

  // allocate globals memory for the library
  g_i8_gfx = (_Gfx_I8_Globals *)_MemPtrNew(sizeof(_Gfx_I8_Globals), false);
  init_ok &= (g_i8_gfx != NULL); if (!init_ok) goto GFX_I8_INIT_ABORT;
  g_gfx->extension = (void *)g_i8_gfx;

  // set the default palette (which, builds the lookup tables)
  _GfxSetPalette(sys_palette);

GFX_I8_INIT_ABORT:

  return init_ok;
}

static void
_Gfx_I8_Terminate()
{
  GFX_GLOBALS_I8_ACCESS;

  // entry condition
  if (g_i8_gfx == NULL) return;

  // clean up
  _MemPtrFree(g_i8_gfx);
  g_gfx->extension = NULL;
}

static gfx_window *
_Gfx_I8_CreateWindow(coord width, coord height)
{
  _gfx_window *result;
  uint32       size;
  void        *bits;
//GFX_GLOBALS_ACCESS;

  // default return value
  result = NULL;

  // allocate memory for the window
  size = (uint32)width * (uint32)height;
  bits = (void *)_MemPtrNew(size, false);

  // create the window using the buffer
  if (bits != NULL)
  {
    result = (_gfx_window *)_GfxCreateWindowFromBuffer(width, height, bits);
    if (result != NULL)
      result->allocated = true;
  }

  return (gfx_window *)result;
}

static gfx_window *
_Gfx_I8_CreateWindowFromBuffer(coord width, coord height, void *bits)
{
  _gfx_window *result;
//GFX_GLOBALS_ACCESS;

  // default return value
  result = NULL;

  // create the window using the buffer
  result = (_gfx_window *)_MemPtrNew(sizeof(_gfx_window), false);
  if (result != NULL)
  {
    _MemSet(result, 0, sizeof(_gfx_window));
    result->width     = width;
    result->height    = height;
    result->bits      = bits;
    result->rowBytes  = width;
    result->allocated = false;

    // we want a "white window"
    _MemSet(bits, 0x00, ((uint32)width * height));
  }

  return (gfx_window *)result;
}

static gfx_window *
_Gfx_I8_CreateWindowFromBufferDisposable(coord width, coord height, void *bits)
{
  _gfx_window *result;
//GFX_GLOBALS_ACCESS;

  // default return value
  result = NULL;

  // create the window using the buffer
  result = (_gfx_window *)_GfxCreateWindowFromBuffer(width, height, bits);
  if (result != NULL)
    result->allocated = true;

  return (gfx_window *)result;
}

static gfx_window *
_Gfx_I8_CreateWindowFromBitmapStream(coord width, coord height, lcd_format *format, void *bits)
{
  _gfx_window *result;
  boolean      valid;
  uint8       *p, *pWin, *pal, msk, val;
  uint16       i, dat;
  int32        cnt, cnt_ii, xtr;
  uint8        r, g, b;
//GFX_GLOBALS_ACCESS;

  // default return value
  result = NULL;

  // lets create the window as requested
  valid  = false;
  result = (_gfx_window *)_GfxCreateWindow(width, height);
  if (result != NULL)
  {
    switch (format->type)
    {
      case lcd_palette:

           // whats the requested size of the palette
           switch (format->depth)
           {
             case 8:
                  // initialize pointers
                  p    = bits;
                  pWin = result->bits;

                  // process every pixel
                  cnt = result->width * result->height;
                  while (cnt--)
                  {
                    dat = *p++;

                    if (format->data.palette.rgb != NULL)
                    {
                      pal = format->data.palette.rgb + (dat * 3);
                      r = *pal++; g = *pal++; b = *pal++;
                    }
                    else
                      r = g = b = 255 - dat;   // 255 -> 0, 1 decrement (easy)

                    // map the RGB to the current palette
                    *pWin++ = (uint8)_GfxGetPaletteIndex(r,g,b);
                  }
                  valid = true;
                  break;

             case 4:
                  // initialize pointers
                  p    = bits;
                  pWin = result->bits;

                  // process every pixel
                  cnt_ii = result->height;
                  while (cnt_ii--)
                  {
                    cnt = result->width >> 1;
                    xtr = result->width & 0x01;

                    while (cnt--)
                    {
                      dat = *p++;

                      // process 2 pixels per byte, in order
                      msk = 0xf0; i = 2;
                      while (i--)
                      {
                        val = dat & msk;
                        val >>= (i * 4); msk >>= 4;

                        // do we have colorization?
                        if (format->data.palette.rgb != NULL)
                        {
                          pal = format->data.palette.rgb + (val * 3);
                          r = *pal++; g = *pal++; b = *pal++;
                        }
                        else
                          r = g = b = 255 - (17 * val); // 255 -> 0, decrements of 17

                        // map the RGB to the current palette
                        *pWin++ = (uint8)_GfxGetPaletteIndex(r,g,b);
                      }
                    }

                    // process 'xtr' pixels per byte, in order
                    if (xtr != 0)
                    {
                      dat = *p++;

                      msk = 0xf0; i = (uint16)xtr;
                      while (i--)
                      {
                        val = dat & msk;
                        val >>= ((i + (2-xtr)) * 4); msk >>= 4;

                        // do we have colorization?
                        if (format->data.palette.rgb != NULL)
                        {
                          pal = format->data.palette.rgb + (val * 3);
                          r = *pal++; g = *pal++; b = *pal++;
                        }
                        else
                          r = g = b = 255 - (17 * val); // 255 -> 0, decrements of 17

                        // map the RGB to the current palette
                        *pWin++ = (uint8)_GfxGetPaletteIndex(r,g,b);
                      }
                    }
                  }
                  valid = true;
                  break;

             case 2:
                  // initialize pointers
                  p    = bits;
                  pWin = result->bits;

                  // process every pixel
                  cnt_ii = result->height;
                  while (cnt_ii--)
                  {
                    cnt = result->width >> 2;
                    xtr = result->width & 0x03;

                    while (cnt--)
                    {
                      dat = *p++;

                      // process 4 pixels per byte, in order
                      msk = 0xc0; i = 4;
                      while (i--)
                      {
                        val = dat & msk;
                        val >>= (i * 2); msk >>= 2;

                        // do we have colorization?
                        if (format->data.palette.rgb != NULL)
                        {
                          pal = format->data.palette.rgb + (val * 3);
                          r = *pal++; g = *pal++; b = *pal++;
                        }
                        else
                          r = g = b = 255 - (85 * val); // 255 -> 0, decrements of 85

                        // map the RGB to the current palette
                        *pWin++ = (uint8)_GfxGetPaletteIndex(r,g,b);
                      }
                    }

                    // process 'xtr' pixels per byte, in order
                    if (xtr != 0)
                    {
                      dat = *p++;

                      msk = 0xc0; i = (uint16)xtr;
                      while (i--)
                      {
                        val = dat & msk;
                        val >>= ((i + (4-xtr)) * 2); msk >>= 2;

                        // do we have colorization?
                        if (format->data.palette.rgb != NULL)
                        {
                          pal = format->data.palette.rgb + (val * 3);
                          r = *pal++; g = *pal++; b = *pal++;
                        }
                        else
                          r = g = b = 255 - (85 * val); // 255 -> 0, decrements of 85

                        // map the RGB to the current palette
                        *pWin++ = (uint8)_GfxGetPaletteIndex(r,g,b);
                      }
                    }
                  }
                  valid = true;
                  break;

             case 1:
                  // initialize pointers
                  p    = bits;
                  pWin = result->bits;

                  // process every pixel
                  cnt_ii = result->height;
                  while (cnt_ii--)
                  {
                    cnt = result->width >> 3;
                    xtr = result->width & 0x07;

                    while (cnt--)
                    {
                      dat = *p++;

                      // process 8 pixels per byte, in order
                      msk = 0x80; i = 8;
                      while (i--)
                      {
                        val = dat & msk;
                        val >>= i; msk >>= 1;

                        // do we have colorization?
                        if (format->data.palette.rgb != NULL)
                        {
                          pal = format->data.palette.rgb + (val * 3);
                          r = *pal++; g = *pal++; b = *pal++;
                        }
                        else
                          r = g = b = 255 - (255 * val); // 255 or 0

                        // map the RGB to the current palette
                        *pWin++ = (uint8)_GfxGetPaletteIndex(r,g,b);
                      }
                    }

                    // process 'xtr' pixels per byte, in order
                    if (xtr != 0)
                    {
                      dat = *p++;

                      msk = 0x80; i = (uint16)xtr;
                      while (i--)
                      {
                        val = dat & msk;
                        val >>= (i + (8-xtr)); msk >>= 1;

                        // do we have colorization?
                        if (format->data.palette.rgb != NULL)
                        {
                          pal = format->data.palette.rgb + (val * 3);
                          r = *pal++; g = *pal++; b = *pal++;
                        }
                        else
                          r = g = b = 255 - (255 * val); // 255 or 0

                        // map the RGB to the current palette
                        *pWin++ = (uint8)_GfxGetPaletteIndex(r,g,b);
                      }
                    }
                  }
                  valid = true;
                  break;

             default:
                  break;
           }

      case lcd_direct:
           if ((format->depth == 16) &&
               (format->data.direct.alpha == 0) &&
               (format->data.direct.red   == 4) &&
               (format->data.direct.green == 4) &&
               (format->data.direct.blue  == 4)) // 12 bit, 16 bits/pixel
           {
             // initialize pointers
             p    = bits;
             pWin = result->bits;

             // process every pixel
             cnt = result->width * result->height;
             while (cnt--)
             {
               dat = (((uint16)p[0] << 8) | p[1]); p += 2;
               _Gfx_D444_GetRGBFromIndex(dat, &r, &g, &b);

               // map the RGB to the current palette
               *pWin++ = (uint8)_GfxGetPaletteIndex(r,g,b);
             }
             valid = true;
           }
           else
           if ((format->depth == 16) &&
               (format->data.direct.alpha == 0) &&
               (format->data.direct.red   == 5) &&
               (format->data.direct.green == 5) &&
               (format->data.direct.blue  == 5)) // 15 bit, 16 bits/pixel
           {
             // initialize pointers
             p    = bits;
             pWin = result->bits;

             // process every pixel
             cnt = result->width * result->height;
             while (cnt--)
             {
               dat = (((uint16)p[0] << 8) | p[1]); p += 2;
               _Gfx_D555_GetRGBFromIndex(dat, &r, &g, &b);

               // map the RGB to the current palette
               *pWin++ = (uint8)_GfxGetPaletteIndex(r,g,b);
             }
             valid = true;
           }
           else
           if ((format->depth == 16) &&
               (format->data.direct.alpha == 0) &&
               (format->data.direct.red   == 5) &&
               (format->data.direct.green == 6) &&
               (format->data.direct.blue  == 5)) // 16 bit, 16 bits/pixel
           {
             // initialize pointers
             p    = bits;
             pWin = result->bits;

             // process every pixel
             cnt = result->width * result->height;
             while (cnt--)
             {
               dat = (((uint16)p[0] << 8) | p[1]); p += 2;
               _Gfx_D565_GetRGBFromIndex(dat, &r, &g, &b);

               // map the RGB to the current palette
               *pWin++ = (uint8)_GfxGetPaletteIndex(r,g,b);
             }
             valid = true;
           }
           else
           if ((format->depth == 24) &&
               (format->data.direct.alpha == 0) &&
               (format->data.direct.red   == 8) &&
               (format->data.direct.green == 8) &&
               (format->data.direct.blue  == 8)) // 24 bit, 24 bits/pixel
           {
             // initialize pointers
             p    = bits;
             pWin = result->bits;

             // process every pixel
             cnt = result->width * result->height;
             while (cnt--)
             {
               // get the RGB from the source material
               r = *p++; g = *p++; b = *p++;
               *pWin++ = (uint8)_GfxGetPaletteIndex(r,g,b);
             }
             valid = true;
           }
           else
           if ((format->depth == 32) &&
               (format->data.direct.alpha == 0) &&
               (format->data.direct.red   == 8) &&
               (format->data.direct.green == 8) &&
               (format->data.direct.blue  == 8)) // 32 bit, 32 bits/pixel
           {
             // initialize pointers
             p    = bits;
             pWin = result->bits;

             // process every pixel
             cnt = result->width * result->height;
             while (cnt--)
             {
               // get the RGB from the source material
               p++;  // skip alpha
               r = *p++; g = *p++; b = *p++;
               *pWin++ = (uint8)_GfxGetPaletteIndex(r,g,b);
             }
             valid = true;
           }

      default:
           break;
    }

    // did something go wrong? invalid format used in import?
    if (!valid)
    {
      _GfxDeleteWindow((gfx_window *)result);
      result = NULL;
    }
  }

  return (gfx_window *)result;
}

static void
_Gfx_I8_DeleteWindow(gfx_window *win)
{
  _gfx_window *_win = (_gfx_window *)win;
  void        *bits;
//GFX_GLOBALS_ACCESS;

  // entry condition
  if (win == NULL) return;

  // clean up!
  bits = NULL;
  if (_win->allocated) bits = _win->bits;

  // we want to free in opposite order it was allocated
  _MemPtrFree(_win);
  if (bits != NULL) _MemPtrFree(bits);
  bits = NULL;
}

static void
_Gfx_I8_FillRegion(rectangle *rect, color c)
{
  _gfx_window *win;
  coord        x1, y1, x2, y2;
  uint8       *ptr;
  int          j, cnt;
  GFX_GLOBALS_ACCESS;

  // entry condition, cannot have a NULL active window
  if ((g_gfx->win == NULL) || (rect == NULL)) return;
  win = (_gfx_window *)g_gfx->win;

  // whats a "valid" box area for this operation?
  x1 = MAX(0, rect->x);
  y1 = MAX(0, rect->y);
  x2 = MIN(win->width,  rect->x + rect->width);
  y2 = MIN(win->height, rect->y + rect->height);

  // active 'draw' window - clipping!
  x1 = MAX(g_gfx->clip.x, x1);
  y1 = MAX(g_gfx->clip.y, y1);
  x2 = MIN(g_gfx->clip.x + g_gfx->clip.width,  x1 + (x2-x1));
  y2 = MIN(g_gfx->clip.y + g_gfx->clip.height, y1 + (y2-y1));

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    // determine the "pointer" value, and how many bytes to blit
    ptr = (uint8 *)(win->bits) + (((uint32)y1 * win->rowBytes) + x1);
    cnt = (x2 - x1);

    // do each line...
    for (j=y1; j<y2; j++)
    {
      _MemSet(ptr, (uint8)c, cnt);  // we can do this, as its 8bpp = 1 byte
      ptr += win->rowBytes;
    }
  }
}

static void
_Gfx_I8_FillRegionRounded(rectangle *rect, color c)
{
  _gfx_window *win;
  coord        x1, y1, x2, y2;
  uint8       *ptr;
  int          i, cnt, cut, ofsL, ofsR, ofs, adj_u, adj_l;
  ROUND_CORNER_DEFINE;
  GFX_GLOBALS_ACCESS;

  // entry condition, cannot have a NULL active window
  if ((g_gfx->win == NULL) || (rect == NULL)) return;
  win = (_gfx_window *)g_gfx->win;

  // whats a "valid" box area for this operation?
  x1 = MAX(0, rect->x);
  y1 = MAX(0, rect->y);
  x2 = MIN(win->width,  rect->x + rect->width);
  y2 = MIN(win->height, rect->y + rect->height);

  // active 'draw' window - clipping!
  x1 = MAX(g_gfx->clip.x, x1);
  y1 = MAX(g_gfx->clip.y, y1);
  x2 = MIN(g_gfx->clip.x + g_gfx->clip.width,  x1 + (x2-x1));
  y2 = MIN(g_gfx->clip.y + g_gfx->clip.height, y1 + (y2-y1));

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    // can we do a "rounded" region?
    cut = MAX(1, MIN(rect->height/2, rect->width/2));
    cut = MIN(5, cut);

    if (cut < 1)
      _Gfx_I8_FillRegion(rect, c);
    else
    {
      // whats the offset in the "corner" indexes
      ofs = (cut-1) * 5;

      // determine the "pointer" value, and how many bytes to blit
      ptr = (uint8 *)(win->bits) + (((uint32)y1 * win->rowBytes) + x1);
      cnt = (x2 - x1);

      // handle the "upper/lower" boundaries of the roundness
      adj_u = y1 - rect->y;
      adj_l = (rect->y + rect->height) - y2;

      // do each line...
      y2 -= y1;
      for (i=0; i<y2; i++)
      {
        if ((i + adj_u) < cut)      ofsL = corner[ofs + (i + adj_u)]; else
        if ((y2-i-1 + adj_l) < cut) ofsL = corner[ofs + (y2 - i - 1 + adj_l)];
        else ofsL = 0; ofsR = ofsL;

        // adjust for cropping
        ofsL = MIN(x2-x1, MAX(0, ofsL - (x1 - rect->x)));
        ofsR = MIN(x2-x1, MAX(0, ofsR - ((rect->x + rect->width) - x2)));

        ptr += ofsL;
        cnt -= (ofsL + ofsR);
        if (cnt > 0)
          _MemSet(ptr, (uint8)c, cnt);  // we can do this, as its 8bpp = 1 byte
        cnt += (ofsL + ofsR);
        ptr -= ofsL;

        ptr += win->rowBytes;
      }
    }
  }
}

static void
_Gfx_I8_InvertRegion(rectangle *rect)
{
  _gfx_window *win;
  coord        x1, y1, x2, y2;
  uint8       *ptr, *invert;
  int          i, j, cnt;
  GFX_GLOBALS_I8_ACCESS;

  // entry condition, cannot have a NULL active window
  if (g_gfx->win == NULL) return;
  win = (_gfx_window *)g_gfx->win;

  // whats a "valid" box area for this operation?
  x1 = MAX(0, rect->x);
  y1 = MAX(0, rect->y);
  x2 = MIN(win->width,  rect->x + rect->width);
  y2 = MIN(win->height, rect->y + rect->height);

  // active 'draw' window - clipping!
  x1 = MAX(g_gfx->clip.x, x1);
  y1 = MAX(g_gfx->clip.y, y1);
  x2 = MIN(g_gfx->clip.x + g_gfx->clip.width,  x1 + (x2-x1));
  y2 = MIN(g_gfx->clip.y + g_gfx->clip.height, y1 + (y2-y1));

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    // we need a handle to the invert table
    invert = g_i8_gfx->palette_invert;

    // determine the "pointer" value, and how many bytes to blit
    ptr = (uint8 *)(win->bits) + (((uint32)y1 * win->rowBytes) + x1);
    cnt = (x2 - x1);

    // do each line...
    for (j=y1; j<y2; j++)
    {
      // do the inner blit
      i = cnt;
      do
      {
        *ptr = *(invert + *ptr); ptr++;
      }
      while (--i);

      ptr += (win->rowBytes - cnt);
    }
  }
}

static void
_Gfx_I8_CopyRegion(gfx_window *srcWin, gfx_window *dstWin, rectangle *rect, coord x, coord y, draw_operation mode)
{
  coord        x1,  y1,  x2,  y2;
  coord        _x1, _y1, _x2, _y2;
  int          i, j;
  uint8       *srcP, *dstP;
  int32        dst_delta, src_delta;
  uint16       cnt;
  color        white, black;
  _gfx_window *_srcWin = (_gfx_window *)srcWin;
  _gfx_window *_dstWin = (_gfx_window *)dstWin;
  GFX_GLOBALS_I8_ACCESS;

  // entry conditions, NULL window or bits doesn't make sense
  if ((_srcWin == NULL) || (_srcWin->bits == NULL) ||
      (_dstWin == NULL) || (_dstWin->bits == NULL) || (rect == NULL)) return;

  // whats a "valid" box area for this operation? - source window
  x1 = MAX(0, rect->x);
  y1 = MAX(0, rect->y);
  x2 = MIN((coord)_srcWin->width,  rect->x + rect->width);
  y2 = MIN((coord)_srcWin->height, rect->y + rect->height);

  // whats a "valid" box area for this operation? - destination window
  if (x < 0) { x1 = x1 - x; x = 0; }
  if (y < 0) { y1 = y1 - y; y = 0; }
  if ((x + (x2 - x1)) > _dstWin->width)  { x2 = x1 + (_dstWin->width  - x); }
  if ((y + (y2 - y1)) > _dstWin->height) { y2 = y1 + (_dstWin->height - y); }

  // active 'draw' window - clipping!
  if ((_gfx_window *)g_gfx->win == _dstWin)
  {
    _x1 = MAX(g_gfx->clip.x, x);
    _y1 = MAX(g_gfx->clip.y, y);
    if (_x1 != x) { x1 += (_x1 - x); x = _x1; }
    if (_y1 != y) { y1 += (_y1 - y); y = _y1; }

    _x2 = MIN(g_gfx->clip.x + g_gfx->clip.width,  x + (x2-x1));
    _y2 = MIN(g_gfx->clip.y + g_gfx->clip.height, y + (y2-y1));
    if (_x2 != (x + (x2-x1))) { x2 += (_x2 - (x + (x2-x1))); }
    if (_y2 != (y + (y2-y1))) { y2 += (_y2 - (y + (y2-y1))); }
  }

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    // determine the "pointer" values..
    srcP = (uint8 *)(_srcWin->bits) + (((uint32)y1 * _srcWin->rowBytes) + x1);
    dstP = (uint8 *)(_dstWin->bits) + (((uint32)y  * _dstWin->rowBytes) + x);
    cnt  = (x2-x1);

    src_delta = (_srcWin->rowBytes - cnt);
    dst_delta = (_dstWin->rowBytes - cnt);

    switch (mode)
    {
      case gfxMask:

           // do each line...
           j = y2 - y1;
           do
           {
             // do the inner blit
             i = cnt;
             do
             {
               // get - adjust - set
               *dstP = *dstP & ~*srcP++;
                dstP++;
             }
             while (--i);

             // adjust "pointers"...
             srcP += src_delta;
             dstP += dst_delta;
           }
           while (--j);

           break;

      case gfxInvert:

           // do each line...
           j = y2 - y1;
           do
           {
             // do the inner blit
             i = cnt;
             do
             {
               // get - adjust - set
               *dstP = *dstP ^ *srcP++;
                dstP++;
             }
             while (--i);

             // adjust "pointers"...
             srcP += src_delta;
             dstP += dst_delta;
           }
           while (--j);

           break;

      case gfxOverlay:

           // do each line...
           j = y2 - y1;
           do
           {
             // do the inner blit
             i = cnt;
             do
             {
               // get - adjust - set
               *dstP = *dstP | *srcP++;
                dstP++;
             }
             while (--i);

             // adjust "pointers"...
             srcP += src_delta;
             dstP += dst_delta;
           }
           while (--j);

           break;

      case gfxTransparent:

           //
           // this is a customized alpha-blending
           //
           //   http://www.visionengineer.com/comp/alpha_blending.shtml

           white = _GfxGetPaletteIndex(255, 255, 255);
           black = _FntGetColor();

           // do each line...
           j = y2 - y1;
           do
           {
             // do the inner blit
             i = cnt;
             do
             {
               // if source has no color; just leave background as-is
               if (*srcP != white)
               {
                 // if destination has no color; copy 1-1
                 if ((*dstP == white) || (*srcP == black)) *dstP = *srcP;

                 // else; alpha blending!
                 else
#if (GFX_ALPHABLEND_REALTIME == 0)
                   *dstP = g_i8_gfx->palette_blending[(*dstP * PALETTE_SIZE) + *srcP];
#else
                 {
                   uint8  r,  g,  b;
                   uint8 _r, _g, _b;

                   _GfxGetRGBFromIndex(*srcP, & r, & g, & b);
                   _GfxGetRGBFromIndex(*dstP, &_r, &_g, &_b);

                   r = (uint8)((r + _r) >> 1);
                   g = (uint8)((g + _g) >> 1);
                   b = (uint8)((b + _b) >> 1); // 50:50 blending

                   *dstP = (uint8)_GfxGetPaletteIndex(r,g,b);
                 }
#endif
               }
               dstP++; srcP++;
             }
             while (--i);

             // adjust "pointers"...
             srcP += src_delta;
             dstP += dst_delta;
           }
           while (--j);

           break;

      case gfxPaint:
      default:

           // do each line...
           j = y2 - y1;
           do
           {
             // do the inner blit
             i = cnt;
             do
             {
               // get - adjust - set
               *dstP++ = *srcP++;
             }
             while (--i);

             // adjust "pointers"...
             srcP += src_delta;
             dstP += dst_delta;
           }
           while (--j);

           break;
    }
  }
}

static void
_Gfx_I8_SetPixel(coord x, coord y, color c)
{
  _gfx_window *win;
  uint8       *ptr;
  GFX_GLOBALS_ACCESS;

  // entry condition, cannot have a NULL active window
  if (g_gfx->win == NULL) return;
  win = (_gfx_window *)g_gfx->win;

  if ((x >= g_gfx->clip.x) &&
      (x < (g_gfx->clip.x + g_gfx->clip.width)) &&
      (y >= g_gfx->clip.y) &&
      (y < (g_gfx->clip.y + g_gfx->clip.height)))
  {
    ptr  = win->bits;
    ptr += ((uint32)y * win->rowBytes) + x;

    *ptr = (uint8)c;
  }
}

static color
_Gfx_I8_GetPixel(coord x, coord y)
{
  color        result;
  _gfx_window *win;
  uint8       *ptr;
  GFX_GLOBALS_ACCESS;

  // default return value
  result = 0;

  // can only do this on an active window
  if (g_gfx->win != NULL)
  {
    win = (_gfx_window *)g_gfx->win;

    if ((x >= g_gfx->clip.x) &&
        (x < (g_gfx->clip.x + g_gfx->clip.width)) &&
        (y >= g_gfx->clip.y) &&
        (y < (g_gfx->clip.y + g_gfx->clip.height)))
    {
      ptr  = win->bits;
      ptr += ((uint32)y * win->rowBytes) + x;

      result = (color)*ptr;
    }
  }

  return result;
}

static void
_Gfx_I8_GetPalette(uint8 *rgb)
{
  GFX_GLOBALS_I8_ACCESS;

  // entry condition
  if (rgb == NULL) return;

  // copy over palette entries
  _MemMove(rgb, g_i8_gfx->palette, (PALETTE_SIZE * 3));
}

static color
_Gfx_I8_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  uint8  *pal;
  int     i;
  color   result;
  uint32  lowValue, *diff, diffValue;
  int     rd, gd, bd;
  GFX_GLOBALS_I8_ACCESS;

  // special purpose cases (these are important for _GfxCopyRegion)
  if ((r == 255) && (g == 255) && (b == 255)) return 0;
  if ((r ==   0) && (g ==   0) && (b ==   0)) return 255;

  // default value
  result = 0;

  // generate the color "differences" for all colors in the palette
  pal  = g_i8_gfx->palette;
  diff = g_i8_gfx->palette_diff;
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
_Gfx_I8_GetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  uint8 *pal;
  GFX_GLOBALS_I8_ACCESS;

  // entry condition
  if ((r == NULL) || (g == NULL) || (b == NULL)) return;

  pal  = g_i8_gfx->palette;
  pal += c * 3; // jump to the right color

  // extract the rgb values
  *r = *pal++;
  *g = *pal++;
  *b = *pal++;
}

static void
_Gfx_I8_SetPalette(uint8 *rgb)
{
  uint8 *invert, *_p;
  uint8  r, g, b;
  int16  i;
#if (GFX_ALPHABLEND_REALTIME == 0)
  uint8 *alpha;
  uin8   _r, _g, _b;
  int16  j;
#endif
  GFX_GLOBALS_I8_ACCESS;

  // copy over palette entries
  _MemMove(g_i8_gfx->palette, rgb, (PALETTE_SIZE * 3));

  // we must force 0=255,255,255 and 255=0,0,0
  _p = &g_i8_gfx->palette[0];   *_p++ = 255; *_p++ = 255; *_p++ = 255;
  _p = &g_i8_gfx->palette[765]; *_p++ = 0;   *_p++ = 0;   *_p++ = 0;

  // we need to built the appropriate 'invert lookup table' _GfxInvertRegion
  _p     = g_i8_gfx->palette;
  invert = g_i8_gfx->palette_invert;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *invert = (uint8)_GfxGetPaletteIndex((uint8)(255 - r),
                                         (uint8)(255 - g),
                                         (uint8)(255 - b));
    invert++;
  }

#if (GFX_ALPHABLEND_REALTIME == 0)
  // build the appropriate alpha blending lookup table
  alpha = g_i8_gfx->palette_blending;
  for (j=0; j<PALETTE_SIZE; j++)
  {
    for (i=0; i<PALETTE_SIZE; i++)
    {
      _GfxGetRGBFromIndex(i, & r, & g, & b);
      _GfxGetRGBFromIndex(j, &_r, &_g, &_b);

      r = (uint8)((r + _r) >> 1);
      g = (uint8)((g + _g) >> 1);
      b = (uint8)((b + _b) >> 1); // 50:50 blending

      *alpha++ = (uint8)_GfxGetPaletteIndex(r,g,b);
    }
  }
#endif

  // we need to re-generate the lookup tables now
  if (g_gfx->fnGfxBuildDisplayLookup != NULL)
    g_gfx->fnGfxBuildDisplayLookup();
}

/*************************************************************************
 *
 * _Gfx CopyToDisplay implementations
 *
 *************************************************************************/

static void
_Gfx_I8_D16_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint8       *pS;
  uint16      *pD, *pal;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_I8_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint8 *)_win->bits;
  pD = (uint16 *)g_gfx->LCD_bits;

  // whats a "valid" box area for this operation?
  x1 = MAX(0, rect->x);
  y1 = MAX(0, rect->y);
  x2 = MIN(_win->width,  rect->x + rect->width);
  y2 = MIN(_win->height, rect->y + rect->height);

  // whats a "valid" box area for this operation? - destination window
  if (scr_x < 0) { x1 = x1 - scr_x; scr_x = 0; }
  if (scr_y < 0) { y1 = y1 - scr_y; scr_y = 0; }
  if ((scr_x + (x2 - x1)) > g_gfx->width)  { x2 = x1 + (g_gfx->width  - scr_x); }
  if ((scr_y + (y2 - y1)) > g_gfx->height) { y2 = y1 + (g_gfx->height - scr_y); }

  // get access to the palette lookup table
  pal  = g_i8_gfx->palette_d16;

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * _win->rowBytes) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = _win->rowBytes             - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

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
_Gfx_I8_D32_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint8       *pS;
  uint32      *pD, *pal;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_I8_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint8 *)_win->bits;
  pD = (uint32 *)g_gfx->LCD_bits;

  // whats a "valid" box area for this operation?
  x1 = MAX(0, rect->x);
  y1 = MAX(0, rect->y);
  x2 = MIN(_win->width,  rect->x + rect->width);
  y2 = MIN(_win->height, rect->y + rect->height);

  // whats a "valid" box area for this operation? - destination window
  if (scr_x < 0) { x1 = x1 - scr_x; scr_x = 0; }
  if (scr_y < 0) { y1 = y1 - scr_y; scr_y = 0; }
  if ((scr_x + (x2 - x1)) > g_gfx->width)  { x2 = x1 + (g_gfx->width  - scr_x); }
  if ((scr_y + (y2 - y1)) > g_gfx->height) { y2 = y1 + (g_gfx->height - scr_y); }

  // get access to the palette lookup table
  pal  = g_i8_gfx->palette_d32;

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * _win->rowBytes) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 2)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 2);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = _win->rowBytes             - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 2) - pix;

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
 *
 * _Gfx BuildDisplayLookup implementations
 *
 *************************************************************************/

static void
_Gfx_I8_D444_BuildDisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_I8_ACCESS;

  _p = g_i8_gfx->palette;
   p = g_i8_gfx->palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_Gfx_D444_GetPaletteIndex(r, g, b);
  }
}

static void
_Gfx_I8_D444x_BuildDisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_I8_ACCESS;

  _p = g_i8_gfx->palette;
   p = g_i8_gfx->palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_Gfx_D444x_GetPaletteIndex(r, g, b);
  }
}

static void
_Gfx_I8_D555_BuildDisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_I8_ACCESS;

  _p = g_i8_gfx->palette;
   p = g_i8_gfx->palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_Gfx_D555_GetPaletteIndex(r, g, b);
  }
}

static void
_Gfx_I8_D555x_BuildDisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_I8_ACCESS;

  _p = g_i8_gfx->palette;
   p = g_i8_gfx->palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_Gfx_D555x_GetPaletteIndex(r, g, b);
  }
}

static void
_Gfx_I8_D565_BuildDisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_I8_ACCESS;

  _p = g_i8_gfx->palette;
   p = g_i8_gfx->palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_Gfx_D565_GetPaletteIndex(r, g, b);
  }
}

static void
_Gfx_I8_D565x_BuildDisplayLookup()
{
  uint8  *_p;
  uint16 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_I8_ACCESS;

  _p = g_i8_gfx->palette;
   p = g_i8_gfx->palette_d16;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint16)_Gfx_D565x_GetPaletteIndex(r, g, b);
  }
}

static void
_Gfx_I8_D888_BuildDisplayLookup()
{
  uint8  *_p;
  uint32 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_I8_ACCESS;

  _p = g_i8_gfx->palette;
   p = g_i8_gfx->palette_d32;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint32)_Gfx_D888_GetPaletteIndex(r, g, b);
  }
}

static void
_Gfx_I8_D888x_BuildDisplayLookup()
{
  uint8  *_p;
  uint32 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_I8_ACCESS;

  _p = g_i8_gfx->palette;
   p = g_i8_gfx->palette_d32;
  for (i=0; i<PALETTE_SIZE; i++)
  {
    r = *_p++; g = *_p++; b = *_p++;
    *p++ = (uint32)_Gfx_D888x_GetPaletteIndex(r, g, b);
  }
}

/********************************* EOF ***********************************/
