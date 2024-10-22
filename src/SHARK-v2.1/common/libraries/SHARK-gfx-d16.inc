/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-gfx-d16.inc
 */

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

#define D333_TABLE_SIZE         512
#define DIRECT_TABLE_SIZE       D333_TABLE_SIZE

// -----------------------------------------------------------------------
// compile time options
// -----------------------------------------------------------------------

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct _Gfx_D16_Globals
{
  union
  {
    uint16 d16[DIRECT_TABLE_SIZE];       // lookup table for direct LCD
    uint32 d32[DIRECT_TABLE_SIZE];       // lookup table for direct LCD
  } lookup;

} _Gfx_D16_Globals;

#define GFX_GLOBALS_D16_ACCESS \
  _GfxGlobals      *g_gfx; \
  _Gfx_D16_Globals *g_d16_gfx; \
  g_gfx     = (_GfxGlobals *)_LibGetGlobals(GFX_LIBRARY_ID); \
  g_d16_gfx = (_Gfx_D16_Globals *)g_gfx->extension;

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
_Gfx_D16_Initialize()
{
  boolean          init_ok;
  _Gfx_D16_Globals *g_d16_gfx;
  GFX_GLOBALS_ACCESS;

  // assume everything is ok
  init_ok = true;

  // allocate globals memory for the library
  g_d16_gfx = (_Gfx_D16_Globals *)_MemPtrNew(sizeof(_Gfx_D16_Globals), false);
  init_ok &= (g_d16_gfx != NULL); if (!init_ok) goto GFX_D16_INIT_ABORT;
  g_gfx->extension = (void *)g_d16_gfx;

  // we need to generate the lookup tables now
  if (g_gfx->fnGfxBuildDisplayLookup != NULL)
    g_gfx->fnGfxBuildDisplayLookup();

GFX_D16_INIT_ABORT:

  return init_ok;
}

static void
_Gfx_D16_Terminate()
{
  GFX_GLOBALS_D16_ACCESS;

  // entry condition
  if (g_d16_gfx == NULL) return;

  // clean up
  _MemPtrFree(g_d16_gfx);
  g_gfx->extension = NULL;
}

static gfx_window *
_Gfx_D16_CreateWindow(coord width, coord height)
{
  _gfx_window *result;
  uint32       size;
  void        *bits;
//GFX_GLOBALS_ACCESS;

  // default return value
  result = NULL;

  // allocate memory for the window
  size = (uint32)width * (uint32)height << 1;
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
_Gfx_D16_CreateWindowFromBuffer(coord width, coord height, void *bits)
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
    result->rowBytes  = (width << 1);
    result->allocated = false;

    // we want a "white window"
    _MemSet(bits, 0xff, ((uint32)width * height << 1));
  }

  return (gfx_window *)result;
}

static gfx_window *
_Gfx_D16_CreateWindowFromBufferDisposable(coord width, coord height, void *bits)
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
_Gfx_D16_CreateWindowFromBitmapStream(coord width, coord height, lcd_format *format, void *bits)
{
  _gfx_window *result;
  boolean      valid;
  uint8       *p, *pal, msk, val;
  uint16      *pWin;
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
                    *pWin++ = (uint16)_GfxGetPaletteIndex(r,g,b);
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
                        *pWin++ = (uint16)_GfxGetPaletteIndex(r,g,b);
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
                        *pWin++ = (uint16)_GfxGetPaletteIndex(r,g,b);
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
                        *pWin++ = (uint16)_GfxGetPaletteIndex(r,g,b);
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
                        *pWin++ = (uint16)_GfxGetPaletteIndex(r,g,b);
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
                        *pWin++ = (uint16)_GfxGetPaletteIndex(r,g,b);
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
                        *pWin++ = (uint16)_GfxGetPaletteIndex(r,g,b);
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
               dat = READ_16_BE(&p[0]); p += 2;
               _Gfx_D444_GetRGBFromIndex(dat, &r, &g, &b);

               // map the RGB to the current palette
               *pWin++ = (uint16)_GfxGetPaletteIndex(r,g,b);
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
               dat = READ_16_BE(&p[0]); p += 2;
               _Gfx_D555_GetRGBFromIndex(dat, &r, &g, &b);

               // map the RGB to the current palette
               *pWin++ = (uint16)_GfxGetPaletteIndex(r,g,b);
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
               dat = READ_16_BE(&p[0]); p += 2;
               _Gfx_D565_GetRGBFromIndex(dat, &r, &g, &b);

               // map the RGB to the current palette
               *pWin++ = (uint16)_GfxGetPaletteIndex(r,g,b);
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
               *pWin++ = (uint16)_GfxGetPaletteIndex(r,g,b);
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
               p++;  // slip alpha byte
               r = *p++; g = *p++; b = *p++;
               *pWin++ = (uint16)_GfxGetPaletteIndex(r,g,b);
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
_Gfx_D16_DeleteWindow(gfx_window *win)
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
_Gfx_D16_FillRegion(rectangle *rect, color c)
{
  _gfx_window *win;
  coord        x1, y1, x2, y2;
  uint16      *ptr;
  int          i, j, cnt;
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
    ptr = (uint16 *)(win->bits) + (((uint32)y1 * (win->rowBytes >> 1)) + x1);
    cnt = (x2 - x1);

    // do each line...
    for (j=y1; j<y2; j++)
    {
      i = cnt;
      do
      {
        *ptr++ = (uint16)c;
      } while (--i);

      ptr += ((win->rowBytes >> 1) - cnt);
    }
  }
}

static void
_Gfx_D16_FillRegionRounded(rectangle *rect, color c)
{
  _gfx_window *win;
  coord        x1, y1, x2, y2;
  uint16      *ptr;
  int          i, j, cnt, cut, ofsL, ofsR, ofs, adj_u, adj_l;
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
      _Gfx_D16_FillRegion(rect, c);
    else
    {
      // whats the offset in the "corner" indexes
      ofs = (cut-1) * 5;

      // determine the "pointer" value, and how many bytes to blit
      ptr = (uint16 *)(win->bits) + (((uint32)y1 * (win->rowBytes >> 1)) + x1);
      cnt = (x2 - x1);

      // handle the "upper/lower" boundaries of the roundness
      adj_u = y1 - rect->y; 
      adj_l = (rect->y + rect->height) - y2; 

      // do each line...
      y2 -= y1;
      for (j=0; j<y2; j++)
      {
        if ((j + adj_u) < cut)      ofsL = corner[ofs + (j + adj_u)]; else
        if ((y2-j-1 + adj_l) < cut) ofsL = corner[ofs + (y2 - j - 1 + adj_l)]; 
        else ofsL = 0; ofsR = ofsL;

        // adjust for cropping
        ofsL = MIN(x2-x1, MAX(0, ofsL - (x1 - rect->x))); 
        ofsR = MIN(x2-x1, MAX(0, ofsR - ((rect->x + rect->width) - x2))); 

        ptr += ofsL;
        cnt -= (ofsL + ofsR);
        if (cnt > 0)
        {
          i = cnt;
          do
          {
            *ptr++ = (uint16)c;
          }
          while (--i);
        }
        cnt += (ofsL + ofsR);
        ptr -= ofsL;

        ptr += ((win->rowBytes >> 1) - (cnt - (ofsL + ofsR)));
      }
    }
  }
}

static void
_Gfx_D16_InvertRegion(rectangle *rect)
{
  _gfx_window *win;
  coord        x1, y1, x2, y2;
  uint16      *ptr;
  int          i, j, cnt;
  GFX_GLOBALS_ACCESS;

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
    // determine the "pointer" value, and how many bytes to blit
    ptr = (uint16 *)(win->bits) + (((uint32)y1 * (win->rowBytes >> 1)) + x1);
    cnt = (x2 - x1);

    // do each line...
    for (j=y1; j<y2; j++)
    {
      // do the inner blit
      i = cnt;
      do
      {
        *ptr = ~(*ptr);
        ptr++;
      }
      while (--i);

      ptr += ((win->rowBytes >> 1) - cnt);
    }
  }
}

static void
_Gfx_D16_CopyRegion(gfx_window *srcWin, gfx_window *dstWin, rectangle *rect, coord x, coord y, draw_operation mode)
{
  coord        x1,  y1,  x2,  y2;
  coord        _x1, _y1, _x2, _y2;
  int          i, j;
  uint16      *srcP, *dstP;
  int32        dst_delta, src_delta;
  uint16       cnt;
  color        white, black;
  _gfx_window *_srcWin = (_gfx_window *)srcWin;
  _gfx_window *_dstWin = (_gfx_window *)dstWin;
  GFX_GLOBALS_ACCESS;

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
    srcP = (uint16 *)(_srcWin->bits) + (((uint32)y1 * (_srcWin->rowBytes >> 1)) + x1);
    dstP = (uint16 *)(_dstWin->bits) + (((uint32)y  * (_dstWin->rowBytes >> 1)) + x);
    cnt  = (x2-x1);

    src_delta = ((_srcWin->rowBytes >> 1) - cnt);
    dst_delta = ((_dstWin->rowBytes >> 1) - cnt);

    switch (mode)
    {
      case gfxMask:

           white = _GfxGetPaletteIndex(255, 255, 255);
           black = _GfxGetPaletteIndex(  0,   0,   0);

           // do each line...
           j = y2 - y1;
           do
           {
             // do the inner blit
             i = cnt;
             do
             {
               if (*srcP == black) *dstP = (uint16)white;
               dstP++; srcP++;
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

           white = _GfxGetPaletteIndex(255, 255, 255);
           black = _GfxGetPaletteIndex(  0,   0,   0);

           // do each line...
           j = y2 - y1;
           do
           {
             // do the inner blit
             i = cnt;
             do
             {
               if (*srcP != white) *dstP = *srcP;
                dstP++; srcP++;
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
                 {
                   uint8  r,  g,  b;
                   uint8 _r, _g, _b;

                   _GfxGetRGBFromIndex(*srcP, & r, & g, & b);
                   _GfxGetRGBFromIndex(*dstP, &_r, &_g, &_b);

                   r = (uint8)((r + _r) >> 1);
                   g = (uint8)((g + _g) >> 1);
                   b = (uint8)((b + _b) >> 1); // 50:50 blending

                   *dstP = (uint16)_GfxGetPaletteIndex(r,g,b);
                 }
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
_Gfx_D16_SetPixel(coord x, coord y, color c)
{
  _gfx_window *win;
  uint16      *ptr;
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
    ptr += ((uint32)y * (win->rowBytes >> 1)) + x;

    *ptr = (uint16)c;
  }
}

static color
_Gfx_D16_GetPixel(coord x, coord y)
{
  color        result;
  _gfx_window *win;
  uint16      *ptr;
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
      ptr += ((uint32)y * (win->rowBytes >> 1)) + x;

      result = (color)*ptr;
    }
  }

  return result;
}

static void
_Gfx_D16_GetPalette(uint8 *rgb)
{
  // no pallete in direct mode
  return;
}

static void
_Gfx_D16_SetPalette(uint8 *rgb)
{
  // no pallete in direct mode
}

/*************************************************************************
 *
 * _Gfx CopyToDisplay implementations
 *
 *************************************************************************/

static void
_Gfx_D333_D16_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS;
  uint16      *lookup;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_D16_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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
  lookup = g_d16_gfx->lookup.d16;

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        *pD++ = *(lookup + (*pS++ & 0x01ff));
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

static void
_Gfx_D333_D32_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint32      *pD;
  uint16      *pS;
  uint32      *lookup;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_D16_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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
  lookup = g_d16_gfx->lookup.d32;

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 2)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 2) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        *pD++ = *(lookup + (*pS++ & 0x01ff));
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

static void
_Gfx_D444_D444x_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        *pD++ = (uint16)
                (((val & 0x0f00) >> 8) |  // .... RRRR .... .... -> 000000000000RRRR
                  (val & 0x00f0)       |  // .... .... GGGG .... -> 00000000GGGG0000
                 ((val & 0x000f) << 8));  // .... .... .... BBBB -> 0000BBBB00000000
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

// this is a generic (un-optimized) conversion routine
static void
_Gfx_D444_D555_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  uint8        r, g, b;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        _Gfx_D444_GetRGBFromIndex(val, &r, &g, &b);
        *pD++ = (uint16)_Gfx_D555_GetPaletteIndex(r, g, b);
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

// this is a generic (un-optimized) conversion routine
static void
_Gfx_D444_D555x_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  uint8        r, g, b;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        _Gfx_D444_GetRGBFromIndex(val, &r, &g, &b);
        *pD++ = (uint16)_Gfx_D555x_GetPaletteIndex(r, g, b);
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

// this is a generic (un-optimized) conversion routine
static void
_Gfx_D444_D565_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  uint8        r, g, b;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        _Gfx_D444_GetRGBFromIndex(val, &r, &g, &b);
        *pD++ = (uint16)_Gfx_D565_GetPaletteIndex(r, g, b);
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

// this is a generic (un-optimized) conversion routine
static void
_Gfx_D444_D565x_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  uint8        r, g, b;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        _Gfx_D444_GetRGBFromIndex(val, &r, &g, &b);
        *pD++ = (uint16)_Gfx_D565x_GetPaletteIndex(r, g, b);
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

// this is a generic (un-optimized) conversion routine
static void
_Gfx_D444_D888_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint32      *pD;
  uint16      *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  uint8        r, g, b;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 2)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 2);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 2) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        _Gfx_D444_GetRGBFromIndex(val, &r, &g, &b);
        *pD++ = (uint32)_Gfx_D888_GetPaletteIndex(r, g, b);
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

// this is a generic (un-optimized) conversion routine
static void
_Gfx_D444_D888x_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint32      *pD;
  uint16      *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  uint8        r, g, b;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 2)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 2);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 2) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        _Gfx_D444_GetRGBFromIndex(val, &r, &g, &b);
        *pD++ = (uint32)_Gfx_D888x_GetPaletteIndex(r, g, b);
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

static void
_Gfx_D555_D444_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS, val;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        *pD++ = (uint16)
                (((val & 0xc800) >> 3) |  // .RRR Rr.. .... .... -> 0000RRRR00000000
                 ((val & 0x03c0) >> 2) |  // .... ..GG GGg. .... -> 00000000GGGG0000
                 ((val & 0x001e) >> 1));  // .... .... ...B BBBb -> 000000000000BBBB
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

static void
_Gfx_D555_D444x_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS, val;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        *pD++ = (uint16)
                (((val & 0xc800) >> 11)|  // .RRR Rr.. .... .... -> 000000000000RRRR
                 ((val & 0x03c0) >> 2) |  // .... ..GG GGg. .... -> 00000000GGGG0000
                 ((val & 0x001e) << 7));  // .... .... ...B BBBb -> 0000BBBB00000000
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

// this is a generic (un-optimized) conversion routine
static void
_Gfx_D555_D565_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  uint8        r, g, b;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        _Gfx_D555_GetRGBFromIndex(val, &r, &g, &b);
        *pD++ = (uint16)_Gfx_D565_GetPaletteIndex(r, g, b);
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

static void
_Gfx_D555_D555x_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        *pD++ = (uint16)
                (((val & 0x7c00) >> 12) | // .RRR RR.. .... .... -> 00000000000RRRRR
                  (val & 0x03e0)        | // .... ..GG GGG. .... -> 000000GGGGG00000
                 ((val & 0x001f) << 12)); // .... .... ...B BBBB -> 0BBBBB0000000000
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

// this is a generic (un-optimized) conversion routine
static void
_Gfx_D555_D565x_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  uint8        r, g, b;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        _Gfx_D555_GetRGBFromIndex(val, &r, &g, &b);
        *pD++ = (uint16)_Gfx_D565x_GetPaletteIndex(r, g, b);
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

// this is a generic (un-optimized) conversion routine
static void
_Gfx_D555_D888_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint32      *pD;
  uint16      *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  uint8        r, g, b;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 2)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 2);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 2) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        _Gfx_D555_GetRGBFromIndex(val, &r, &g, &b);
        *pD++ = (uint32)_Gfx_D888_GetPaletteIndex(r, g, b);
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

// this is a generic (un-optimized) conversion routine
static void
_Gfx_D555_D888x_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint32      *pD;
  uint16      *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  uint8        r, g, b;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 2)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 2);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 2) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        _Gfx_D555_GetRGBFromIndex(val, &r, &g, &b);
        *pD++ = (uint32)_Gfx_D888x_GetPaletteIndex(r, g, b);
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

static void
_Gfx_D565_D444_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS, val;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        *pD++ = (uint16)
                (((val & 0xf000) >> 4) |  // RRRR r... .... .... -> 0000RRRR00000000
                 ((val & 0x0780) >> 3) |  // .... .GGG Ggg. .... -> 00000000GGGG0000
                 ((val & 0x001e) >> 1));  // .... .... ...B BBBb -> 000000000000BBBB
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

static void
_Gfx_D565_D444x_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS, val;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        *pD++ = (uint16)
                (((val & 0xf000) >> 12) | // RRRR r... .... .... -> 000000000000RRRR
                 ((val & 0x0780) >> 3)  | // .... .GGG Ggg. .... -> 00000000GGGG0000
                 ((val & 0x001e) << 7));  // .... .... ...B BBBb -> 0000BBBB00000000
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

static void
_Gfx_D565_D555_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS, val;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        *pD++ = (uint16)
                (((val & 0xffc0) >> 1) |  // RRRR RGGG GGg. .... -> 0RRRRRGGGGG00000
                 ((val & 0x001f)     ));  // .... .... ...B BBBB -> 00000000000BBBBB
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

static void
_Gfx_D565_D555x_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS, val;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        *pD++ = (uint16)
                (((val & 0xf100) >> 11) | // RRRR R... .... .... -> 00000000000RRRRR
                 ((val & 0x07c0) >> 1)  | // .... .GGG GGg. .... -> 000000GGGGG00000
                 ((val & 0x001e) << 12)); // .... .... ...B BBBB -> 0BBBBB0000000000

      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

static void
_Gfx_D565_D565x_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        *pD++ = (uint16)
                (((val & 0xf800) >> 13) | // RRRR R... .... .... -> 00000000000RRRRR
                  (val & 0x07e0)        | // .... .GGG GGG. .... -> 00000GGGGGG00000
                 ((val & 0x001f) << 13)); // .... .... ...B BBBB -> BBBBB00000000000
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

// this is a generic (un-optimized) conversion routine
static void
_Gfx_D565_D888_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint32      *pD;
  uint16      *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  uint8        r, g, b;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 2)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 2);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 2) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        _Gfx_D565_GetRGBFromIndex(val, &r, &g, &b);
        *pD++ = (uint32)_Gfx_D888_GetPaletteIndex(r, g, b);
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

// this is a generic (un-optimized) conversion routine
static void
_Gfx_D565_D888x_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint32      *pD;
  uint16      *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  uint8        r, g, b;
  color        val;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 2)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 2);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 2) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        val = *pS++;
        _Gfx_D565_GetRGBFromIndex(val, &r, &g, &b);
        *pD++ = (uint32)_Gfx_D888x_GetPaletteIndex(r, g, b);
      } while (--i);

      pS += win_delta;
      pD += lcd_delta;
    }
    while (--cnt);
  }
}

static void
_Gfx_D16_D16_CopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  uint16      *pD, *pS;
  int16        cnt, pix, i, win_delta, lcd_delta;
  coord        x1, y1, x2, y2;
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry condition
  if ((win == NULL) || (rect == NULL)) return;

  pS = (uint16 *)_win->bits;
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

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1    * (_win->rowBytes      >> 1)) + x1);
    pD += (((uint32)scr_y * (g_gfx->LCD_rowBytes >> 1)) + scr_x);

    // if the LCD window != same size as _Gfx LCD, center it
    pD +=  (g_gfx->LCD_width  - g_gfx->width)  >> 1;
    pD += ((g_gfx->LCD_height - g_gfx->height) >> 1) * (g_gfx->LCD_rowBytes >> 1);

    cnt = y2-y1;
    pix = x2-x1;

    win_delta = (_win->rowBytes      >> 1) - pix;
    lcd_delta = (g_gfx->LCD_rowBytes >> 1) - pix;

    // do each line...
    do
    {
      i = pix;
      do
      {
        *pD++ = *pS++;
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
_Gfx_D333_D444_BuildDisplayLookup()
{
  uint16 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_D16_ACCESS;

  p = g_d16_gfx->lookup.d16;
  for (i=0; i<D333_TABLE_SIZE; i++)
  {
    _Gfx_D333_GetRGBFromIndex(i, &r, &g, &b);
    *p++ = (uint16)_Gfx_D444_GetPaletteIndex(r, g, b);
  }
}

static void
_Gfx_D333_D444x_BuildDisplayLookup()
{
  uint16 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_D16_ACCESS;

  p = g_d16_gfx->lookup.d16;
  for (i=0; i<D333_TABLE_SIZE; i++)
  {
    _Gfx_D333_GetRGBFromIndex(i, &r, &g, &b);
    *p++ = (uint16)_Gfx_D444x_GetPaletteIndex(r, g, b);
  }
}

static void
_Gfx_D333_D555_BuildDisplayLookup()
{
  uint16 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_D16_ACCESS;

  p = g_d16_gfx->lookup.d16;
  for (i=0; i<D333_TABLE_SIZE; i++)
  {
    _Gfx_D333_GetRGBFromIndex(i, &r, &g, &b);
    *p++ = (uint16)_Gfx_D555_GetPaletteIndex(r, g, b);
  }
}

static void
_Gfx_D333_D555x_BuildDisplayLookup()
{
  uint16 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_D16_ACCESS;

  p = g_d16_gfx->lookup.d16;
  for (i=0; i<D333_TABLE_SIZE; i++)
  {
    _Gfx_D333_GetRGBFromIndex(i, &r, &g, &b);
    *p++ = (uint16)_Gfx_D555x_GetPaletteIndex(r, g, b);
  }
}

static void
_Gfx_D333_D565_BuildDisplayLookup()
{
  uint16 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_D16_ACCESS;

  p = g_d16_gfx->lookup.d16;
  for (i=0; i<D333_TABLE_SIZE; i++)
  {
    _Gfx_D333_GetRGBFromIndex(i, &r, &g, &b);
    *p++ = (uint16)_Gfx_D565_GetPaletteIndex(r, g, b);
  }
}

static void
_Gfx_D333_D565x_BuildDisplayLookup()
{
  uint16 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_D16_ACCESS;

  p = g_d16_gfx->lookup.d16;
  for (i=0; i<D333_TABLE_SIZE; i++)
  {
    _Gfx_D333_GetRGBFromIndex(i, &r, &g, &b);
    *p++ = (uint16)_Gfx_D565x_GetPaletteIndex(r, g, b);
  }
}

static void
_Gfx_D333_D888_BuildDisplayLookup()
{
  uint32 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_D16_ACCESS;

  p = g_d16_gfx->lookup.d32;
  for (i=0; i<D333_TABLE_SIZE; i++)
  {
    _Gfx_D333_GetRGBFromIndex(i, &r, &g, &b);
    *p++ = (uint32)_Gfx_D888_GetPaletteIndex(r, g, b);
  }
}

static void
_Gfx_D333_D888x_BuildDisplayLookup()
{
  uint32 *p;
  uint8   r, g, b;
  int     i;
  GFX_GLOBALS_D16_ACCESS;

  p = g_d16_gfx->lookup.d32;
  for (i=0; i<D333_TABLE_SIZE; i++)
  {
    _Gfx_D333_GetRGBFromIndex(i, &r, &g, &b);
    *p++ = (uint32)_Gfx_D888x_GetPaletteIndex(r, g, b);
  }
}

/********************************* EOF ***********************************/
