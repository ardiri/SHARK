/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-gfx.c
 */

#include "../../SHARK.h"

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

#define GFX_LIBRARY_ID  0x5F676678 // '_gfx'

// -----------------------------------------------------------------------
// compile time options
// -----------------------------------------------------------------------

// GFX_GET_RGB_FAST
//
// _Gfx_XXX_GetRGBFromIndex
//   -> this routine converts a color index into a 0..255 RGB components
//      it can either just simply bitshift the values (fast) or actually
//      do extrapolation of the values (slow, more accurate)
//
//      = 0 - more accurate conversion (0..255)
//      = 1 - quick/dirty - bits shifted, lower bits are '0

   #define GFX_GET_RGB_FAST        0      // best quality/slow

#define ROUND_CORNER_DEFINE             \
  int    corner[25] = {                 \
                        1, 0, 0, 0, 0,  \
                        2, 1, 0, 0, 0,  \
                        3, 1, 1, 0, 0,  \
                        4, 2, 1, 1, 0,  \
                        5, 3, 2, 1, 1   \
                      }

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct _gfx_window
{
  coord   width;
  coord   height;
  void   *bits;
  uint16  rowBytes;

  boolean allocated;
} _gfx_window;

typedef struct _GfxGlobals
{
  gfx_window   *LCD;               // the display window (used for blitting)
  coord         width;
  coord         height;

  void         *LCD_bits;
  coord         LCD_width;
  coord         LCD_height;
  uint16        LCD_rowBytes;

  gfx_window   *win;               // the active draw window
  rectangle     clip;              // the active clipping region

  void         *extension;

  boolean     (*fnGfxInitialize)(void);
  void        (*fnGfxTerminate)(void);
  gfx_window *(*fnGfxCreateWindow)(coord, coord);
  gfx_window *(*fnGfxCreateWindowFromBuffer)(coord, coord, void *);
  gfx_window *(*fnGfxCreateWindowFromBufferDisposable)(coord, coord, void *);
  gfx_window *(*fnGfxCreateWindowFromBitmapStream)(coord width, coord height, lcd_format *format, void *bits);
  void        (*fnGfxDeleteWindow)(gfx_window *);

  void        (*fnGfxGetPalette)(uint8 *);
  void        (*fnGfxSetPalette)(uint8 *);
  color       (*fnGfxGetPaletteIndex)(uint8, uint8, uint8);
  void        (*fnGfxGetRGBFromIndex)(color, uint8 *, uint8 *, uint8 *);

  void        (*fnGfxFillRegion)(rectangle *, color);
  void        (*fnGfxFillRegionRounded)(rectangle *, color);
  void        (*fnGfxInvertRegion)(rectangle *);
  void        (*fnGfxCopyRegion)(gfx_window *, gfx_window *, rectangle *, coord, coord, draw_operation);

  void        (*fnGfxSetPixel)(coord, coord, color);
  color       (*fnGfxGetPixel)(coord, coord);

  void        (*fnGfxCopyToDisplay)(gfx_window *, rectangle *, coord, coord);
  void        (*fnGfxBuildDisplayLookup)(void);

} _GfxGlobals;

#define GFX_GLOBALS_ACCESS \
  _GfxGlobals *g_gfx = (_GfxGlobals *)_LibGetGlobals(GFX_LIBRARY_ID);

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibGetGlobals() API.
//------------------------------------------------------------------------

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static void    _GfxInitialize_RESIZE();
static void    _GfxTerminate_RESIZE();

static boolean _GfxDriverConfiguration(lcd_format *format);
static boolean _GfxHandleEvent(event *e);

boolean
_GfxInitialize(lcd_format *format)
{
  boolean      init_ok;
  _GfxGlobals *g_gfx;

  // assume everything is ok
  init_ok = true;

  // lets see if we have already been initialized
  g_gfx = (_GfxGlobals *)_LibGetGlobals(GFX_LIBRARY_ID);

  // entry condition
  if ((format == NULL) || (g_gfx != NULL)) return false;

  // register the library
  if (_LibRegister(GFX_LIBRARY_ID, "_GfxLibrary", (void *)_GfxHandleEvent))
  {
    // allocate globals memory for the library
    g_gfx = (_GfxGlobals *)_MemPtrNew(sizeof(_GfxGlobals), false);
    init_ok &= (g_gfx != NULL); if (!init_ok) goto GFX_INIT_ABORT;
    _LibSetGlobals(GFX_LIBRARY_ID, (void *)g_gfx);

    // initialize the _Gfx driver
    init_ok = _GfxDriverConfiguration(format);
    if (!init_ok) goto GFX_INIT_ABORT;

    // initialize _Gfx internally
    _GfxInitialize_RESIZE();
  }
  else
    init_ok = false;

GFX_INIT_ABORT:

  return init_ok;
}

void
_GfxTerminate()
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return;

  // terminate _Gfx internally
  _GfxTerminate_RESIZE();

  // shutdown the sub-graphics layer
  if (g_gfx->fnGfxTerminate != NULL) g_gfx->fnGfxTerminate();

  // free the globals used
  _MemPtrFree(g_gfx); g_gfx = NULL;
  _LibSetGlobals(GFX_LIBRARY_ID, (void *)g_gfx);

  // shutdown the library
  _LibShutdown(GFX_LIBRARY_ID);
}

gfx_window *
_GfxCreateWindow(coord width, coord height)
{
  gfx_window *result;
  GFX_GLOBALS_ACCESS;

  // default return value
  result = NULL;

  // entry requirement (must call _GfxInitialize())
  if ((g_gfx == NULL) || (width == 0) || (height == 0)) return result;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxCreateWindow != NULL)
    result = g_gfx->fnGfxCreateWindow(width, height);

  return result;
}

gfx_window *
_GfxCreateWindowFromBuffer(coord width, coord height, void *bits)
{
  gfx_window *result;
  GFX_GLOBALS_ACCESS;

  // default return value
  result = NULL;

  // entry requirement (must call _GfxInitialize())
  if ((g_gfx == NULL) || 
      (width == 0) || (height == 0) || (bits == NULL)) return result;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxCreateWindowFromBuffer != NULL)
    result = g_gfx->fnGfxCreateWindowFromBuffer(width, height, bits);

  return result;
}

gfx_window *
_GfxCreateWindowFromBufferDisposable(coord width, coord height, void *bits)
{
  gfx_window *result;
  GFX_GLOBALS_ACCESS;

  // default return value
  result = NULL;

  // entry requirement (must call _GfxInitialize())
  if ((g_gfx == NULL) || 
      (width == 0) || (height == 0) || (bits == NULL)) return result;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxCreateWindowFromBufferDisposable != NULL)
    result = g_gfx->fnGfxCreateWindowFromBufferDisposable(width, height, bits);

  return result;
}

gfx_window *
_GfxCreateWindowFromBitmapStream(coord width, coord height, lcd_format *format, void *bits)
{
  gfx_window *result;
  GFX_GLOBALS_ACCESS;

  // default return value
  result = NULL;

  // entry requirement (must call _GfxInitialize())
  if ((g_gfx == NULL) || 
      (width == 0) || (height == 0) || 
      (bits == NULL) || (format == NULL)) return result;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxCreateWindowFromBitmapStream != NULL)
    result = g_gfx->fnGfxCreateWindowFromBitmapStream(width, height, format, bits);

  return result;
}

gfx_window *
_GfxCreateWindowFromFile(uint32 disk, char *filename, rectangle *bounds)
{
  gfx_window *result;
  coord       width, height;
  lcd_format  img_format;
  uint8      *img_data;
  char       *nfo_data, *p;
  file       *fn, *fi;
  uint32      size;

  // default return value
  result = NULL;

  // entry conditions
  if ((filename == NULL) || (bounds == NULL)) return result;

  // load the nfo file to get the right parameters
  fn = _FileGetHandle(disk, NULL, filename);
  if (fn != NULL)
  {
    size = _FileGetSize(fn);
    nfo_data = _MemPtrNew(size, false);
    if ((nfo_data != NULL) && (_FileLoadIntoBuffer(fn, nfo_data, size)))
    {
      // lets verify the nfo file format (rough, but works)
      if ((_StrChr(nfo_data,    'x') != NULL) &&
          (_StrChr(nfo_data,    ':') != NULL) &&
          (_StrStr(nfo_data, ".rgb") != NULL))
      {
        p = nfo_data;
        width  = (int16)_StrAToI(p); p = _StrChr(p, 'x'); p++;
        height = (int16)_StrAToI(p); p = _StrChr(p, ':'); p++;
        *(_StrStr(p, ".rgb") + 4) = 0;  // ensure it ends with .rgb

        // open the raw "rgb" file and load into memory buffer
        fi = _FileGetHandle(disk, NULL, p);
        if (fi != NULL)
        {
          size = _FileGetSize(fi);

          img_data = _MemPtrNew(size, false);
          if ((img_data != NULL) && (_FileLoadIntoBuffer(fi, img_data, size)))
          {
            // image verification (size)
            if (size == (uint32)(width * height * 3L))
            {
              _MemSet(&img_format, 0, sizeof(lcd_format));
              img_format.type  = lcd_direct;
              img_format.depth = 24;
              img_format.data.direct.alpha = 0;
              img_format.data.direct.red   = 8;
              img_format.data.direct.green = 8;
              img_format.data.direct.blue  = 8;

              // create the window, based on the raw data
              result = (gfx_window *)
                _GfxCreateWindowFromBitmapStream(width, height,
                                                 &img_format, img_data);

              // define the image boundaries
              bounds -> x      = 0;
              bounds -> y      = 0;
              bounds -> width  = width;
              bounds -> height = height;
            }
          }

          if (img_data != NULL) _MemPtrFree(img_data);
          _FileReleaseHandle(fi);
        }
      }
    }
    if (nfo_data != NULL) _MemPtrFree(nfo_data);
    _FileReleaseHandle(fn);
  }

  return result;
}

void
_GfxDeleteWindow(gfx_window *win)
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if ((g_gfx == NULL) || (win == NULL)) return;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxDeleteWindow != NULL)
    g_gfx->fnGfxDeleteWindow(win);
}

void
_GfxGetWindowInfo(gfx_window *win, void **bits, coord *width, coord *height, uint16 *rowBytes)
{
  _gfx_window *_win = (_gfx_window *)win;
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if ((g_gfx == NULL) || (win == NULL)) return;

  if (width    != NULL) *width     = _win->width;
  if (height   != NULL) *height    = _win->height;
  if (bits     != NULL) *bits      = _win->bits;
  if (rowBytes != NULL) *rowBytes  = _win->rowBytes;
}

gfx_window *
_GfxGetDisplayWindow()
{
  gfx_window *result;
  GFX_GLOBALS_ACCESS;

  // default return value
  result = NULL;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return result;

  // get a handle to the LCD window
  result = g_gfx->LCD;

  return result;
}

void
_GfxSetDrawWindow(gfx_window *window)
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if ((g_gfx == NULL) || (window == NULL)) return;

  // set the new active window and reset the clipping regions
  g_gfx->win = window;
  _GfxResetClip();
}

gfx_window *
_GfxGetDrawWindow()
{
  gfx_window *result;
  GFX_GLOBALS_ACCESS;

  // default return value
  result = NULL;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return result;

  // obtain a reference to the active graphics window
  result = g_gfx->win;

  return result;
}

void
_GfxGetPalette(uint8 *rgb)
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxGetPalette != NULL)
    g_gfx->fnGfxGetPalette(rgb);
}

void
_GfxSetPalette(uint8 *rgb)
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxSetPalette != NULL)
    g_gfx->fnGfxSetPalette(rgb);
}

color
_GfxGetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  color result;
  GFX_GLOBALS_ACCESS;

  // default return value
  result = 0;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return result;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxGetPaletteIndex != NULL)
    result = g_gfx->fnGfxGetPaletteIndex(r, g, b);

  return result;
}

void
_GfxGetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if ((g_gfx == NULL) || (r == NULL) || (g == NULL) || (b == NULL)) return;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxGetRGBFromIndex != NULL)
    g_gfx->fnGfxGetRGBFromIndex(c, r, g, b);
}

void
_GfxClearWindow()
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return;

  // fill the window (based on clipping region with white)
  _GfxFillRegion(&g_gfx->clip, _GfxGetPaletteIndex(255, 255, 255));
}

void
_GfxFillRegion(rectangle *rect, color c)
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxFillRegion != NULL)
    g_gfx->fnGfxFillRegion(rect, c);
}

void
_GfxFillRegionRounded(rectangle *rect, color c)
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxFillRegionRounded != NULL)
    g_gfx->fnGfxFillRegionRounded(rect, c);
}

void
_GfxInvertRegion(rectangle *rect)
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxInvertRegion != NULL)
    g_gfx->fnGfxInvertRegion(rect);
}

void
_GfxCopyRegion(gfx_window *srcWin, gfx_window *dstWin, rectangle *rect,
               coord x, coord y, draw_operation mode)
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxCopyRegion != NULL)
    g_gfx->fnGfxCopyRegion(srcWin, dstWin, rect, x, y, mode);
}

void
_GfxSetClip(rectangle *rect)
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if ((g_gfx == NULL) || (rect == NULL)) return;

  g_gfx->clip.x      = rect->x;
  g_gfx->clip.y      = rect->y;
  g_gfx->clip.width  = rect->width;
  g_gfx->clip.height = rect->height;
}

void
_GfxMergeClip(rectangle *rect)
{
  coord oldX, oldY;
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if ((g_gfx == NULL) || (rect == NULL)) return;

  // the boundaries *never* cross over
  if ((rect->x       > (g_gfx->clip.x + g_gfx->clip.width )) ||
      (g_gfx->clip.x > (rect->x       + rect->width       )) ||
      (rect->y       > (g_gfx->clip.y + g_gfx->clip.height)) ||
      (g_gfx->clip.y > (rect->y       + rect->height      )))
  {
    g_gfx->clip.x      = 0;
    g_gfx->clip.y      = 0;
    g_gfx->clip.width  = 0;
    g_gfx->clip.height = 0;  // no clipping
  }
  else
  {
    oldX = g_gfx->clip.x;
    oldY = g_gfx->clip.y;

    g_gfx->clip.x = MAX(g_gfx->clip.x, rect->x);
    g_gfx->clip.y = MAX(g_gfx->clip.y, rect->y);

    g_gfx->clip.width  = MIN(rect->width,
                             g_gfx->clip.width -  (g_gfx->clip.x - oldX));
    g_gfx->clip.height = MIN(rect->height,
                             g_gfx->clip.height - (g_gfx->clip.y - oldY));
  }
}

void
_GfxGetClip(rectangle *rect)
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if ((rect == NULL) || (g_gfx == NULL)) return;

  rect->x      = g_gfx->clip.x;
  rect->y      = g_gfx->clip.y;
  rect->width  = g_gfx->clip.width;
  rect->height = g_gfx->clip.height;
}

void
_GfxResetClip()
{
  _gfx_window *win;
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if ((g_gfx == NULL) || (g_gfx->win == NULL)) return;

  // we need to have an active window
  win = (_gfx_window *)g_gfx->win;

  g_gfx->clip.x      = 0;
  g_gfx->clip.y      = 0;
  g_gfx->clip.width  = win->width;
  g_gfx->clip.height = win->height;
}

void
_GfxSetPixel(coord x, coord y, color c)
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxSetPixel != NULL)
    g_gfx->fnGfxSetPixel(x, y, c);
}

color
_GfxGetPixel(coord x, coord y)
{
  color result;
  GFX_GLOBALS_ACCESS;

  // default return value
  result = 0;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return result;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxGetPixel != NULL)
    result = g_gfx->fnGfxGetPixel(x, y);

  return result;
}

void
_GfxDrawLine(coord x1, coord y1, coord x2, coord y2, color c)
{
  int16 i, dx, dy, sdx, sdy, dxabs, dyabs, x, y;
  coord px, py;
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return;

  dx    = x2 - x1;               // the horizontal distance of the line
  dy    = y2 - y1;               // the vertical distance of the line
  dxabs = (dx < 0) ? -dx : dx;
  dyabs = (dy < 0) ? -dy : dy;
  sdx   = (dx < 0) ? -1 : 1;
  sdy   = (dy < 0) ? -1 : 1;
  x     = dyabs >> 1;
  y     = dxabs >> 1;
  px    = x1;
  py    = y1;

  _GfxSetPixel(px, py, c);

  // its the same damn point, just get out
  if ((x1 == x2) && (y1 == y2)) return;

  // the line is more horizontal than vertical
  if (dxabs >= dyabs)
  {
    i = dxabs;
    do
    {
      y += dyabs;
      if (y >= dxabs)
      {
        y -= dxabs;
        py += sdy;
      }
      px += sdx;
      _GfxSetPixel(px, py, c);
    }
    while (--i);
  }

  // the line is more vertical than horizontal
  else
  {
    i = dyabs;
    do
    {
      x += dxabs;
      if (x >= dyabs)
      {
        x -= dyabs;
        px += sdx;
      }
      py += sdy;
      _GfxSetPixel(px, py, c);
    }
    while (--i);
  }
}

void
_GfxDrawRectangle(rectangle *rect, color c)
{
  coord x1, y1, x2, y2;
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if ((g_gfx == NULL) || (rect == NULL)) return;

  x1 = rect->x;
  y1 = rect->y;
  x2 = rect->x + rect->width  - 1;
  y2 = rect->y + rect->height - 1;

  _GfxDrawLine(x1, y1, x2, y1, c);
  _GfxDrawLine(x1, y1, x1, y2, c);
  _GfxDrawLine(x2, y1, x2, y2, c);
  _GfxDrawLine(x1, y2, x2, y2, c);
}

void
_GfxDrawRectangleRounded(rectangle *rect, color c)
{
  int    cut, val, i, ofs;
  coord  x1, y1, x2, y2;
  ROUND_CORNER_DEFINE;
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if ((g_gfx == NULL) || (rect == NULL)) return;

  x1 = rect->x;
  y1 = rect->y;
  x2 = rect->x + rect->width  - 1;
  y2 = rect->y + rect->height - 1;

  // can we do a "rounded" region?
  cut = MAX(1, MIN((y2-y1)/2, (x2-x1)/2));
  cut = MIN(5, cut);
  ofs = (cut-1) * 5;

  if (cut < 1)
    _GfxDrawRectangle(rect, c);
  else
  {
    val = corner[ofs];
    _GfxDrawLine((coord)(x1+val), y1, (coord)(x2-val), y1, c);
    _GfxDrawLine(x1, (coord)(y1+val), x1, (coord)(y2-val), c);
    _GfxDrawLine(x2, (coord)(y1+val), x2, (coord)(y2-val), c);
    _GfxDrawLine((coord)(x1+val), y2, (coord)(x2-val), y2, c);
  
    for (i=0; i<cut; i++)
    {
      val = corner[ofs + i];

      _GfxSetPixel((coord)(x1+val), (coord)(y1+i), c);
      _GfxSetPixel((coord)(x2-val), (coord)(y1+i), c);
      _GfxSetPixel((coord)(x1+val), (coord)(y2-i), c);
      _GfxSetPixel((coord)(x2-val), (coord)(y2-i), c);
      _GfxSetPixel((coord)(x1+i), (coord)(y1+val), c);
      _GfxSetPixel((coord)(x2-i), (coord)(y1+val), c);
      _GfxSetPixel((coord)(x1+i), (coord)(y2-val), c);
      _GfxSetPixel((coord)(x2-i), (coord)(y2-val), c);
    }
  }
}

void
_GfxRepaint()
{
  rectangle rect;
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return;

  // define the repaint boundaries
  rect.x      = 0;
  rect.y      = 0;
  rect.width  = g_gfx->width;
  rect.height = g_gfx->height;

  // do the repaint
  _GfxRepaintRegion(&rect);
}

void
_GfxRepaintRegion(rectangle *rect)
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if ((g_gfx == NULL) || (rect == NULL)) return;

  // do the repaint
  _GfxCopyToDisplay(g_gfx->LCD, rect, rect->x, rect->y);
  _LCDRepaintRegion(rect);
}

void
_GfxCopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  GFX_GLOBALS_ACCESS;

  // entry requirement (must call _GfxInitialize())
  if (g_gfx == NULL) return;

  // execute the sub-graphics layer command
  if (g_gfx->fnGfxCopyToDisplay != NULL)
    g_gfx->fnGfxCopyToDisplay(win, rect, scr_x, scr_y);
}

static void
_GfxInitialize_RESIZE()
{
  GFX_GLOBALS_ACCESS;

  // we need to know the LCD properties
  _LCDGetProperties(&g_gfx->LCD_bits, 
                    &g_gfx->LCD_width, &g_gfx->LCD_height, 
                    &g_gfx->LCD_rowBytes, NULL, NULL, NULL);

  // the _Gfx LCD window is the same size as the true LCD window
  g_gfx->width  = g_gfx->LCD_width;
  g_gfx->height = g_gfx->LCD_height;

  // we need to create the LCD window
  g_gfx->LCD = _GfxCreateWindow(g_gfx->width, g_gfx->height);

  // the active window is the display window right now
  g_gfx->win = _GfxGetDisplayWindow();
  _GfxResetClip();
}

static void
_GfxTerminate_RESIZE()
{
  GFX_GLOBALS_ACCESS;

  // dispose of the *old* _Gfx window
  if (g_gfx->LCD != NULL) _GfxDeleteWindow(g_gfx->LCD);
}

/*************************************************************************
 *
 * _Gfx GetPaletteIndex implementations
 *
 *************************************************************************/

static color
_Gfx_D333_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  color result;

  // generate a direct color reference
  result = (((uint16)(r & 0xe0) << 1) |   // 0000000111000000
            ((uint16)(g & 0xe0) >> 2) |   // 0000000000111000
            ((uint16)(b & 0xe0) >> 5));   // 0000000000000111

  return result;
}

/**
 ** not used right now
 **
static color
_Gfx_D333x_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  color result;

  // generate a direct color reference
  result = (((uint16)(b & 0xe0) << 8) |   // 1110000000000000
            ((uint16)(g & 0xe0) << 5) |   // 0001110000000000
            ((uint16)(r & 0xe0) << 2));   // 0000001110000000

  return result;
}
 **/

static color
_Gfx_D444_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  color result;

  // generate a direct color reference
  result = (((uint16)(r & 0xF0) << 4) |   // 0000111100000000
            ((uint16)(g & 0xF0)     ) |   // 0000000011110000
            ((uint16)(b & 0xF0) >> 4));   // 0000000000001111

  return result;
}

static color
_Gfx_D444x_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  color result;

  // generate a direct color reference
  result = (((uint16)(b & 0xF0) << 4) |   // 0000111100000000
            ((uint16)(g & 0xF0)     ) |   // 0000000011110000
            ((uint16)(r & 0xF0) >> 4));   // 0000000000001111

  return result;
}

static color
_Gfx_D555_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  color result;

  // generate a direct color reference
  result = (((uint16)(r & 0xF8) << 7) |   // 0111110000000000
            ((uint16)(g & 0xF8) << 2) |   // 0000001111100000
            ((uint16)(b & 0xF8) >> 3));   // 0000000000011111

  return result;
}

static color
_Gfx_D555x_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  color result;

  // generate a direct color reference
  result = (((uint16)(b & 0xF8) << 7) |   // 0111110000000000
            ((uint16)(g & 0xF8) << 2) |   // 0000001111100000
            ((uint16)(r & 0xF8) >> 3));   // 0000000000011111

  return result;
}

static color
_Gfx_D565_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  color result;

  // generate a direct color reference
  result = (((uint16)(r & 0xF8) << 8) |   // 1111100000000000
            ((uint16)(g & 0xFC) << 3) |   // 0000011111100000
            ((uint16)(b & 0xF8) >> 3));   // 0000000000011111

  return result;
}

static color
_Gfx_D565x_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  color result;

  // generate a direct color reference
  result = (((uint16)(b & 0xF8) << 8) |   // 1111100000000000
            ((uint16)(g & 0xFC) << 3) |   // 0000011111100000
            ((uint16)(r & 0xF8) >> 3));   // 0000000000011111

  return result;
}

static color
_Gfx_D888_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  color result;

  // generate a direct color reference
  result = (((uint32)r << 16) |
            ((uint32)g << 8)  |
            ((uint32)b));

  return result;
}

static color
_Gfx_D888x_GetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  color result;

  // generate a direct color reference
  result = (((uint32)b << 16) |
            ((uint32)g <<  8) |
            ((uint32)r));

  return result;
}

/*************************************************************************
 *
 * _Gfx GetRGBFromIndex implementations
 *
 *************************************************************************/

static void
_Gfx_D333_GetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  // 0x00 .. 0x07 --> 0x00 .. 0xff = (((x+1) << 8) - 1) >> 3

  // get the RGB from the source material
#if (GFX_GET_RGB_FAST == 0)
  *b = ((((((uint16)c & 0x0007) + 0x0001) << 8) - 1) >> 3) - 15;
  *g = ((((((uint16)c & 0x0038) + 0x0008) << 5) - 1) >> 3) - 15;
  *r = ((((((uint16)c & 0x01c0) + 0x0040) << 2) - 1) >> 3) - 15;
#else
  *b = (uint16)(c & 0x0007) << 5;
  *g = (uint16)(c & 0x0038) << 2;
  *r = (uint16)(c & 0x01c0) >> 1;
#endif

  // special cases (extremes)
  if (((uint16)c & 0x0007) == 0x0000) *b = 0;
  if (((uint16)c & 0x0007) == 0x0007) *b = 255;
  if (((uint16)c & 0x0038) == 0x0000) *g = 0;
  if (((uint16)c & 0x0038) == 0x0038) *g = 255;
  if (((uint16)c & 0x01c0) == 0x0000) *r = 0;
  if (((uint16)c & 0x01c0) == 0x01c0) *r = 255;
}

/**
 ** not used right now
 **
static void
_Gfx_D333x_GetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  // 0x00 .. 0x07 --> 0x00 .. 0xff = (((x+1) << 8) - 1) >> 3

  // get the RGB from the source material
#if (GFX_GET_RGB_FAST == 0)
  *r = ((((((uint16)c & 0x0007) + 0x0001) << 8) - 1) >> 3) - 15;
  *g = ((((((uint16)c & 0x0038) + 0x0008) << 5) - 1) >> 3) - 15;
  *b = ((((((uint16)c & 0x01c0) + 0x0040) << 2) - 1) >> 3) - 15;
#else
  *r = (uint16)(c & 0x0007) << 5;
  *g = (uint16)(c & 0x0038) << 2;
  *b = (uint16)(c & 0x01c0) >> 1;
#endif

  // special cases (extremes)
  if (((uint16)c & 0x0007) == 0x0000) *r = 0;
  if (((uint16)c & 0x0007) == 0x0007) *r = 255;
  if (((uint16)c & 0x0038) == 0x0000) *g = 0;
  if (((uint16)c & 0x0038) == 0x0038) *g = 255;
  if (((uint16)c & 0x01c0) == 0x0000) *b = 0;
  if (((uint16)c & 0x01c0) == 0x01c0) *b = 255;
}
 **/

static void
_Gfx_D444_GetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  // 0x00 .. 0x0f --> 0x00 .. 0xff = (((x+1) << 8) - 1) >> 4

  // get the RGB from the source material
#if (GFX_GET_RGB_FAST == 0)
  *b = ((((((uint16)c & 0x000f) + 0x0001) << 8) - 1) >> 4) - 7;
  *g = ((((((uint16)c & 0x00f0) + 0x0010) << 4) - 1) >> 4) - 7;
  *r = ((((((uint16)c & 0x0f00) + 0x0100)     ) - 1) >> 4) - 7;
#else
  *b = (uint16)(c & 0x000f) << 4;
  *g =  (uint8)(c & 0x00f0);
  *r = (uint16)(c & 0x0f00) >> 4;
#endif

  // special cases (extremes)
  if (((uint16)c & 0x000f) == 0x0000) *b = 0;
  if (((uint16)c & 0x000f) == 0x000f) *b = 255;
  if (((uint16)c & 0x00f0) == 0x0000) *g = 0;
  if (((uint16)c & 0x00f0) == 0x00f0) *g = 255;
  if (((uint16)c & 0x0f00) == 0x0000) *r = 0;
  if (((uint16)c & 0x0f00) == 0x0f00) *r = 255;
}

/**
 ** not used right now
 **
static void
_Gfx_D444x_GetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  // 0x00 .. 0x0f --> 0x00 .. 0xff = (((x+1) << 8) - 1) >> 4

  // get the RGB from the source material
#if (GFX_GET_RGB_FAST == 0)
  *r = ((((((uint16)c & 0x000f) + 0x0001) << 8) - 1) >> 4) - 7;
  *g = ((((((uint16)c & 0x00f0) + 0x0010) << 4) - 1) >> 4) - 7;
  *b = ((((((uint16)c & 0x0f00) + 0x0100)     ) - 1) >> 4) - 7;
#else
  *r = (uint16)(c & 0x000f) << 4;
  *g =  (uint8)(c & 0x00f0);
  *b = (uint16)(c & 0x0f00) >> 4;
#endif

  // special cases (extremes)
  if (((uint16)c & 0x000f) == 0x0000) *r = 0;
  if (((uint16)c & 0x000f) == 0x000f) *r = 255;
  if (((uint16)c & 0x00f0) == 0x0000) *g = 0;
  if (((uint16)c & 0x00f0) == 0x00f0) *g = 255;
  if (((uint16)c & 0x0f00) == 0x0000) *b = 0;
  if (((uint16)c & 0x0f00) == 0x0f00) *b = 255;
}
 **/

static void
_Gfx_D555_GetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  // 0x00 .. 0x1f --> 0x00 .. 0xff = (((x+1) << 8) - 1) >> 5

  // get the RGB from the source material
#if (GFX_GET_RGB_FAST == 0)
  *b = ((((((uint16)c & 0x001f) + 0x0001) << 8) - 1) >> 5) - 3;
  *g = ((((((uint16)c & 0x03e0) + 0x0020) << 3) - 1) >> 5) - 3;
  *r = ((((((uint16)c & 0x7c00) + 0x0400) >> 2) - 1) >> 5) - 3;
#else
  *b = (uint16)(c & 0x001f) << 3;
  *g = (uint16)(c & 0x03e0) >> 2;
  *r = (uint16)(c & 0x7c00) >> 7;
#endif

  // special cases (extremes)
  if (((uint16)c & 0x001f) == 0x0000) *b = 0;
  if (((uint16)c & 0x001f) == 0x001f) *b = 255;
  if (((uint16)c & 0x03e0) == 0x0000) *g = 0;
  if (((uint16)c & 0x03e0) == 0x03e0) *g = 255;
  if (((uint16)c & 0x7c00) == 0x0000) *r = 0;
  if (((uint16)c & 0x7c00) == 0x7c00) *r = 255;
}

/**
 ** not used right now
 **
static void
_Gfx_D555x_GetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  // 0x00 .. 0x1f --> 0x00 .. 0xff = (((x+1) << 8) - 1) >> 5

  // get the RGB from the source material
#if (GFX_GET_RGB_FAST == 0)
  *r = ((((((uint16)c & 0x001f) + 0x0001) << 8) - 1) >> 5) - 3;
  *g = ((((((uint16)c & 0x03e0) + 0x0020) << 3) - 1) >> 5) - 3;
  *b = ((((((uint16)c & 0x7c00) + 0x0400) >> 2) - 1) >> 5) - 3;
#else
  *r = (uint16)(c & 0x001f) << 3;
  *g = (uint16)(c & 0x03e0) >> 2;
  *b = (uint16)(c & 0x7c00) >> 7;
#endif

  // special cases (extremes)
  if (((uint16)c & 0x001f) == 0x0000) *r = 0;
  if (((uint16)c & 0x001f) == 0x001f) *r = 255;
  if (((uint16)c & 0x03e0) == 0x0000) *g = 0;
  if (((uint16)c & 0x03e0) == 0x03e0) *g = 255;
  if (((uint16)c & 0x7c00) == 0x0000) *b = 0;
  if (((uint16)c & 0x7c00) == 0x7c00) *b = 255;
}
 **/

static void
_Gfx_D565_GetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  // 0x00 .. 0x1f --> 0x00 .. 0xff = (((x+1) << 8) - 1) >> 5
  // 0x00 .. 0x3f --> 0x00 .. 0xff = (((x+1) << 8) - 1) >> 6

  // get the RGB from the source material
#if (GFX_GET_RGB_FAST == 0)
  *b = ((((((uint16)c & 0x001f) + 0x0001) << 8) - 1) >> 5) - 3;
  *g = ((((((uint16)c & 0x07e0) + 0x0020) << 3) - 1) >> 6) - 1;
  *r = ((((((uint16)c & 0xf800) + 0x0800) >> 3) - 1) >> 5) - 3;
#else
  *b = (uint16)(c & 0x001f) << 3;
  *g = (uint16)(c & 0x07e0) >> 3;
  *r = (uint16)(c & 0xf800) >> 8;
#endif

  // special cases (extremes)
  if (((uint16)c & 0x001f) == 0x0000) *b = 0;
  if (((uint16)c & 0x001f) == 0x001f) *b = 255;
  if (((uint16)c & 0x07e0) == 0x0000) *g = 0;
  if (((uint16)c & 0x07e0) == 0x07e0) *g = 255;
  if (((uint16)c & 0xf800) == 0x0000) *r = 0;
  if (((uint16)c & 0xf800) == 0xf800) *r = 255;
}

/**
 ** not used right now
 **
static void
_Gfx_D565x_GetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b)
{
  // 0x00 .. 0x1f --> 0x00 .. 0xff = (((x+1) << 8) - 1) >> 5
  // 0x00 .. 0x3f --> 0x00 .. 0xff = (((x+1) << 8) - 1) >> 6

  // get the RGB from the source material
#if (GFX_GET_RGB_FAST == 0)
  *r = ((((((uint16)c & 0x001f) + 0x0001) << 8) - 1) >> 5) - 3;
  *g = ((((((uint16)c & 0x07e0) + 0x0020) << 3) - 1) >> 6) - 1;
  *b = ((((((uint16)c & 0xf800) + 0x0800) >> 3) - 1) >> 5) - 3;
#else
  *r = (uint16)(c & 0x001f) << 3;
  *g = (uint16)(c & 0x07e0) >> 3;
  *b = (uint16)(c & 0xf800) >> 8;
#endif

  // special cases (extremes)
  if (((uint16)c & 0x001f) == 0x0000) *r = 0;
  if (((uint16)c & 0x001f) == 0x001f) *r = 255;
  if (((uint16)c & 0x07e0) == 0x0000) *g = 0;
  if (((uint16)c & 0x07e0) == 0x07e0) *g = 255;
  if (((uint16)c & 0xf800) == 0x0000) *b = 0;
  if (((uint16)c & 0xf800) == 0xf800) *b = 255;
}
 **/

#include "SHARK-gfx-i8.inc"
#include "SHARK-gfx-d16.inc"

static boolean
_GfxDriverConfiguration(lcd_format *format)
{
  boolean     result;
  lcd_format  lcd;
  boolean     lcd_xRGB;
  GFX_GLOBALS_ACCESS;

  // default return value
  result = false;

  // we need to know the LCD properties (just want lcd_format)
  _LCDGetProperties(NULL, NULL, NULL, NULL, &lcd, NULL, NULL);

  // we will assume xRGB for most displays, we should check this could be BGRx
  lcd_xRGB = true;
  if (lcd.data.direct.r_mask & 0x01) lcd_xRGB = false;

  switch (format->type)
  {
    case lcd_palette:

         // whats the requested depth/palette size
         switch (format->depth)
         {
           case 8:

                // initialize the _Gfx routines
                g_gfx->fnGfxInitialize                       = (void *)_Gfx_I8_Initialize;
                g_gfx->fnGfxTerminate                        = (void *)_Gfx_I8_Terminate;
                g_gfx->fnGfxCreateWindow                     = (void *)_Gfx_I8_CreateWindow;
                g_gfx->fnGfxCreateWindowFromBuffer           = (void *)_Gfx_I8_CreateWindowFromBuffer;
                g_gfx->fnGfxCreateWindowFromBufferDisposable = (void *)_Gfx_I8_CreateWindowFromBufferDisposable;
                g_gfx->fnGfxCreateWindowFromBitmapStream     = (void *)_Gfx_I8_CreateWindowFromBitmapStream;
                g_gfx->fnGfxDeleteWindow                     = (void *)_Gfx_I8_DeleteWindow;
                g_gfx->fnGfxFillRegion                       = (void *)_Gfx_I8_FillRegion;
                g_gfx->fnGfxFillRegionRounded                = (void *)_Gfx_I8_FillRegionRounded;
                g_gfx->fnGfxInvertRegion                     = (void *)_Gfx_I8_InvertRegion;
                g_gfx->fnGfxCopyRegion                       = (void *)_Gfx_I8_CopyRegion;
                g_gfx->fnGfxSetPixel                         = (void *)_Gfx_I8_SetPixel;
                g_gfx->fnGfxGetPixel                         = (void *)_Gfx_I8_GetPixel;
                g_gfx->fnGfxGetPalette                       = (void *)_Gfx_I8_GetPalette;
                g_gfx->fnGfxGetPaletteIndex                  = (void *)_Gfx_I8_GetPaletteIndex;
                g_gfx->fnGfxGetRGBFromIndex                  = (void *)_Gfx_I8_GetRGBFromIndex;
                g_gfx->fnGfxSetPalette                       = (void *)_Gfx_I8_SetPalette;

                switch (lcd.type)
                {
                  case lcd_palette:
                       break;

                  case lcd_direct:
                       if ((lcd.depth             == 16) &&
                           (lcd.data.direct.alpha == 0) &&
                           (lcd.data.direct.red   == 4) &&
                           (lcd.data.direct.green == 4) &&
                           (lcd.data.direct.blue  == 4))
                       {
                         if (lcd_xRGB)
                           g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_I8_D444_BuildDisplayLookup;
                         else
                           g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_I8_D444x_BuildDisplayLookup;
                         g_gfx->fnGfxCopyToDisplay        = (void *)_Gfx_I8_D16_CopyToDisplay;
                       }
                       else
                       if ((lcd.depth             == 16) &&
                           (lcd.data.direct.alpha == 0) &&
                           (lcd.data.direct.red   == 5) &&
                           (lcd.data.direct.green == 5) &&
                           (lcd.data.direct.blue  == 5))
                       {
                         if (lcd_xRGB)
                           g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_I8_D555_BuildDisplayLookup;
                         else
                           g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_I8_D555x_BuildDisplayLookup;
                         g_gfx->fnGfxCopyToDisplay        = (void *)_Gfx_I8_D16_CopyToDisplay;
                       }
                       else
                       if ((lcd.depth             == 16) &&
                           (lcd.data.direct.alpha == 0) &&
                           (lcd.data.direct.red   == 5) &&
                           (lcd.data.direct.green == 6) &&
                           (lcd.data.direct.blue  == 5))
                       {
                         if (lcd_xRGB)
                           g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_I8_D565_BuildDisplayLookup;
                         else
                           g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_I8_D565x_BuildDisplayLookup;
                         g_gfx->fnGfxCopyToDisplay        = (void *)_Gfx_I8_D16_CopyToDisplay;
                       }
                       else
                       if ((lcd.depth             == 32) &&
                           (lcd.data.direct.alpha == 0) &&
                           (lcd.data.direct.red   == 8) &&
                           (lcd.data.direct.green == 8) &&
                           (lcd.data.direct.blue  == 8))
                       {
                         if (lcd_xRGB)
                           g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_I8_D888_BuildDisplayLookup;
                         else
                           g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_I8_D888x_BuildDisplayLookup;
                         g_gfx->fnGfxCopyToDisplay        = (void *)_Gfx_I8_D32_CopyToDisplay;
                       }
                       break;
                }
                break;

           default:
                break;
         }
         break;

    case lcd_direct:

         if (format->depth == 16)
         {
           // initialize the _Gfx routines
           g_gfx->fnGfxInitialize                       = (void *)_Gfx_D16_Initialize;
           g_gfx->fnGfxTerminate                        = (void *)_Gfx_D16_Terminate;
           g_gfx->fnGfxCreateWindow                     = (void *)_Gfx_D16_CreateWindow;
           g_gfx->fnGfxCreateWindowFromBuffer           = (void *)_Gfx_D16_CreateWindowFromBuffer;
           g_gfx->fnGfxCreateWindowFromBufferDisposable = (void *)_Gfx_D16_CreateWindowFromBufferDisposable;
           g_gfx->fnGfxCreateWindowFromBitmapStream     = (void *)_Gfx_D16_CreateWindowFromBitmapStream;
           g_gfx->fnGfxDeleteWindow                     = (void *)_Gfx_D16_DeleteWindow;
           g_gfx->fnGfxFillRegion                       = (void *)_Gfx_D16_FillRegion;
           g_gfx->fnGfxFillRegionRounded                = (void *)_Gfx_D16_FillRegionRounded;
           g_gfx->fnGfxInvertRegion                     = (void *)_Gfx_D16_InvertRegion;
           g_gfx->fnGfxCopyRegion                       = (void *)_Gfx_D16_CopyRegion;
           g_gfx->fnGfxSetPixel                         = (void *)_Gfx_D16_SetPixel;
           g_gfx->fnGfxGetPixel                         = (void *)_Gfx_D16_GetPixel;
           g_gfx->fnGfxGetPalette                       = (void *)_Gfx_D16_GetPalette;
           g_gfx->fnGfxSetPalette                       = (void *)_Gfx_D16_SetPalette;

           if ((format->data.direct.alpha == 0) &&
               (format->data.direct.red   == 3) &&
               (format->data.direct.green == 3) &&
               (format->data.direct.blue  == 3))
           {
             g_gfx->fnGfxGetPaletteIndex = (void *)_Gfx_D333_GetPaletteIndex;
             g_gfx->fnGfxGetRGBFromIndex = (void *)_Gfx_D333_GetRGBFromIndex;

             switch (lcd.type)
             {
               case lcd_palette:
                    break;

               case lcd_direct:
                    if ((lcd.depth             == 16) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 4) &&
                        (lcd.data.direct.green == 4) &&
                        (lcd.data.direct.blue  == 4))
                    {
                      if (lcd_xRGB)
                        g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_D333_D444_BuildDisplayLookup;
                      else
                        g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_D333_D444x_BuildDisplayLookup;
                      g_gfx->fnGfxCopyToDisplay        = (void *)_Gfx_D333_D16_CopyToDisplay;
                    }
                    else
                    if ((lcd.depth             == 16) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 5) &&
                        (lcd.data.direct.green == 5) &&
                        (lcd.data.direct.blue  == 5))
                    {
                      if (lcd_xRGB)
                        g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_D333_D555_BuildDisplayLookup;
                      else
                        g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_D333_D555x_BuildDisplayLookup;
                      g_gfx->fnGfxCopyToDisplay        = (void *)_Gfx_D333_D16_CopyToDisplay;
                    }
                    else
                    if ((lcd.depth             == 16) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 5) &&
                        (lcd.data.direct.green == 6) &&
                        (lcd.data.direct.blue  == 5))
                    {
                      if (lcd_xRGB)
                        g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_D333_D565_BuildDisplayLookup;
                      else
                        g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_D333_D565x_BuildDisplayLookup;
                      g_gfx->fnGfxCopyToDisplay        = (void *)_Gfx_D333_D16_CopyToDisplay;
                    }
                    else
                    if ((lcd.depth             == 32) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 8) &&
                        (lcd.data.direct.green == 8) &&
                        (lcd.data.direct.blue  == 8))
                    {
                      if (lcd_xRGB)
                        g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_D333_D888_BuildDisplayLookup;
                      else
                        g_gfx->fnGfxBuildDisplayLookup = (void *)_Gfx_D333_D888x_BuildDisplayLookup;
                      g_gfx->fnGfxCopyToDisplay        = (void *)_Gfx_D333_D32_CopyToDisplay;
                    } 
                    break;
             }
           }
           else
           if ((format->data.direct.alpha == 0) &&
               (format->data.direct.red   == 4) &&
               (format->data.direct.green == 4) &&
               (format->data.direct.blue  == 4))
           {
             g_gfx->fnGfxGetPaletteIndex = (void *)_Gfx_D444_GetPaletteIndex;
             g_gfx->fnGfxGetRGBFromIndex = (void *)_Gfx_D444_GetRGBFromIndex;

             switch (lcd.type)
             {
               case lcd_palette:
                    break;

               case lcd_direct:
                    if ((lcd.depth             == 16) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 4) &&
                        (lcd.data.direct.green == 4) &&
                        (lcd.data.direct.blue  == 4))
                    {
                      if (lcd_xRGB)
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D16_D16_CopyToDisplay;
                      else
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D444_D444x_CopyToDisplay;
                      g_gfx->fnGfxBuildDisplayLookup = (void *)NULL;
                    }
                    else
                    if ((lcd.depth             == 16) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 5) &&
                        (lcd.data.direct.green == 5) &&
                        (lcd.data.direct.blue  == 5))
                    {
                      // 444 -> 555 = loss of quality, use generic functions
                      if (lcd_xRGB)
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D444_D555_CopyToDisplay;
                      else
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D444_D555x_CopyToDisplay;
                      g_gfx->fnGfxBuildDisplayLookup = (void *)NULL;
                    }
                    else
                    if ((lcd.depth             == 16) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 5) &&
                        (lcd.data.direct.green == 6) &&
                        (lcd.data.direct.blue  == 5))
                    {
                      // 444 -> 565 = loss of quality, use generic functions
                      if (lcd_xRGB)
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D444_D565_CopyToDisplay;
                      else
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D444_D565x_CopyToDisplay;
                      g_gfx->fnGfxBuildDisplayLookup = (void *)NULL;
                    }
                    else
                    if ((lcd.depth             == 32) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 8) &&
                        (lcd.data.direct.green == 8) &&
                        (lcd.data.direct.blue  == 8))
                    {
                      // 444 -> 888 = loss of quality, use generic functions
                      if (lcd_xRGB)
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D444_D888_CopyToDisplay;
                      else
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D444_D888x_CopyToDisplay;
                      g_gfx->fnGfxBuildDisplayLookup = (void *)NULL;
                    }
                    break;
             }
           }
           else
           if ((format->data.direct.alpha == 0) &&
               (format->data.direct.red   == 5) &&
               (format->data.direct.green == 5) &&
               (format->data.direct.blue  == 5))
           {
             g_gfx->fnGfxGetPaletteIndex = (void *)_Gfx_D555_GetPaletteIndex;
             g_gfx->fnGfxGetRGBFromIndex = (void *)_Gfx_D555_GetRGBFromIndex;

             switch (lcd.type)
             {
               case lcd_palette:
                    break;

               case lcd_direct:
                    if ((lcd.depth             == 16) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 4) &&
                        (lcd.data.direct.green == 4) &&
                        (lcd.data.direct.blue  == 4))
                    {
                      if (lcd_xRGB)
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D555_D444_CopyToDisplay;
                      else
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D555_D444x_CopyToDisplay;
                      g_gfx->fnGfxBuildDisplayLookup = (void *)NULL;
                    }
                    else
                    if ((lcd.depth             == 16) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 5) &&
                        (lcd.data.direct.green == 5) &&
                        (lcd.data.direct.blue  == 5))
                    {
                      if (lcd_xRGB)
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D16_D16_CopyToDisplay;
                      else
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D555_D555x_CopyToDisplay;
                      g_gfx->fnGfxBuildDisplayLookup = (void *)NULL;
                    }
                    else
                    if ((lcd.depth             == 16) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 5) &&
                        (lcd.data.direct.green == 6) &&
                        (lcd.data.direct.blue  == 5))
                    {
                      // 555 -> 565 = loss of quality, use generic functions
                      if (lcd_xRGB)
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D555_D565_CopyToDisplay;
                      else
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D555_D565x_CopyToDisplay;
                      g_gfx->fnGfxBuildDisplayLookup = (void *)NULL;
                    }
                    else
                    if ((lcd.depth             == 32) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 8) &&
                        (lcd.data.direct.green == 8) &&
                        (lcd.data.direct.blue  == 8))
                    {
                      // 555 -> 888 = loss of quality, use generic functions
                      if (lcd_xRGB)
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D555_D888_CopyToDisplay;
                      else
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D555_D888x_CopyToDisplay;
                      g_gfx->fnGfxBuildDisplayLookup = (void *)NULL;
                    }
                    break;
             }
           }
           else
           if ((format->data.direct.alpha == 0) &&
               (format->data.direct.red   == 5) &&
               (format->data.direct.green == 6) &&
               (format->data.direct.blue  == 5))
           {
             g_gfx->fnGfxGetPaletteIndex = (void *)_Gfx_D565_GetPaletteIndex;
             g_gfx->fnGfxGetRGBFromIndex = (void *)_Gfx_D565_GetRGBFromIndex;

             switch (lcd.type)
             {
               case lcd_palette:
                    break;

               case lcd_direct:
                    if ((lcd.depth             == 16) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 4) &&
                        (lcd.data.direct.green == 4) &&
                        (lcd.data.direct.blue  == 4))
                    {
                      if (lcd_xRGB)
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D565_D444_CopyToDisplay;
                      else
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D565_D444x_CopyToDisplay;
                      g_gfx->fnGfxBuildDisplayLookup = (void *)NULL;
                    }
                    else
                    if ((lcd.depth             == 16) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 5) &&
                        (lcd.data.direct.green == 5) &&
                        (lcd.data.direct.blue  == 5))
                    {
                      if (lcd_xRGB)
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D565_D555_CopyToDisplay;
                      else
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D565_D555x_CopyToDisplay;
                      g_gfx->fnGfxBuildDisplayLookup = (void *)NULL;
                    }
                    else
                    if ((lcd.depth             == 16) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 5) &&
                        (lcd.data.direct.green == 6) &&
                        (lcd.data.direct.blue  == 5))
                    {
                      if (lcd_xRGB)
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D16_D16_CopyToDisplay;
                      else
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D565_D565x_CopyToDisplay;
                      g_gfx->fnGfxBuildDisplayLookup = (void *)NULL;
                    }
                    else
                    if ((lcd.depth             == 32) &&
                        (lcd.data.direct.alpha == 0) &&
                        (lcd.data.direct.red   == 8) &&
                        (lcd.data.direct.green == 8) &&
                        (lcd.data.direct.blue  == 8))
                    {
                      // 565 -> 888 = loss of quality, use generic functions
                      if (lcd_xRGB)
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D565_D888_CopyToDisplay;
                      else
                        g_gfx->fnGfxCopyToDisplay    = (void *)_Gfx_D565_D888x_CopyToDisplay;
                      g_gfx->fnGfxBuildDisplayLookup = (void *)NULL;
                    }
                    break;
             }
           }
         }
         break;

    default:
         break;
  }

  // this is confirmation that we support this mode
  result = (g_gfx->fnGfxCopyToDisplay != NULL);

  // initialize the sub-graphics layer
  if (result && (g_gfx->fnGfxInitialize != NULL))
    result = g_gfx->fnGfxInitialize();

  return result;
}

static boolean
_GfxHandleEvent(event *e)
{
  switch (e->eType)
  {
    case _keyEvent:
         switch (e->data.key.chr)
         {
           case _key_gfx_change:
                _GfxTerminate_RESIZE();
                _GfxInitialize_RESIZE();
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return false;
}

/********************************* EOF ***********************************/
