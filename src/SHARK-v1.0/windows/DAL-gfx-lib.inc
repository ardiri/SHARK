/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)windows/DAL-gfx-lib.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

void
_GfxDisplayInfo(coord *width, coord *height)
{
  *width  = (coord)gDALGlobals.gfx.width;
  *height = (coord)gDALGlobals.gfx.height;
}

void
_GfxSetDrawWindow(gfx_window *window)
{
  gDALGlobals.gfx.activeWindow = window;
  if (window != NULL) _GfxResetClip();
}

gfx_window *
_GfxGetDrawWindow()
{
  return gDALGlobals.gfx.activeWindow;
}

void
_GfxGetPalette(uint8 *rgb)
{
  int    i;
  uint8 *rgb_ptr = rgb;

  // copy over palette entries
  for (i=0; i<PALETTE_SIZE; i++)
  {
    *rgb_ptr++ = gDALGlobals.gfx.palette[i].rgbRed;
    *rgb_ptr++ = gDALGlobals.gfx.palette[i].rgbGreen;
    *rgb_ptr++ = gDALGlobals.gfx.palette[i].rgbBlue;
  }
}

void
_GfxSetPalette(uint8 *rgb)
{
  int    i;
  uint8 *rgb_ptr = rgb;

  // copy over palette entries
  for (i=0; i<PALETTE_SIZE; i++)
  {
    gDALGlobals.gfx.palette[i].rgbRed      = *rgb_ptr++;
    gDALGlobals.gfx.palette[i].rgbGreen    = *rgb_ptr++;
    gDALGlobals.gfx.palette[i].rgbBlue     = *rgb_ptr++;
    gDALGlobals.gfx.palette[i].rgbReserved = 0;
  }

  // prepare data structures (specific to blitting) for optimization
  {
    BITMAPINFO       *pbmi;
    void             *ptr;
    void             *old_ptr;
    _WindowsWindow   *win = (_WindowsWindow *)gDALGlobals.gfx.window;  // main window

    if (win != NULL)
    {
      pbmi = (BITMAPINFO *)win->bmi;

      // configure the bitmap to use the currently active palette (important)
      memmove(&pbmi->bmiColors[0], &gDALGlobals.gfx.palette[0], (uint16)(sizeof(RGBQUAD) * PALETTE_SIZE));

      // step 1: backup existing window contents 
      old_ptr = malloc((uint32)win->width*win->height);
      memmove(old_ptr, win->bits, (uint32)win->width*win->height);

      // step 2: create a new window based on new palette
      DeleteDC(win->dc);
      DeleteObject(win->bmp);
      win->dc  = CreateCompatibleDC(gDALGlobals.LCDScreen);
      win->bmp = CreateDIBSection(win->dc, pbmi, DIB_RGB_COLORS, ((void **)(&ptr)), NULL, 0);
      SelectObject(win->dc, win->bmp);
      win->bits = ptr;

      // step 3: restore window contents (yes, is different palette - we know)
      memmove(win->bits, old_ptr, (uint32)win->width*win->height);
      free(old_ptr);
    }
  }

  // prepare data structures (specific to blitting) for optimization
  switch (gDALGlobals.gfx.depth)
  {
#ifdef GFX_12BPP_CUSTOM
    case 12:
         rgb_ptr = rgb;
         for (i=0; i<PALETTE_SIZE; i++)
         {
           // 4-4-4 12bit display
           gDALGlobals.gfx._16bpp[i] = (((uint16)(rgb_ptr[0] & 0xF0) << 8) |   // 0000111100000000
                                     ((uint16)(rgb_ptr[1] & 0xF0))      |   // 0000000011110000
                                     ((uint16)(rgb_ptr[2] & 0xF0) >> 4));   // 0000000000001111
           rgb_ptr += 3;
         }
         break;
#endif

    case 16:
         rgb_ptr = rgb;
         for (i=0; i<PALETTE_SIZE; i++)
         {
           // 5-6-5 16bit display
           gDALGlobals.gfx._16bpp[i] = (((uint16)(rgb_ptr[0] & 0xF8) << 8) |   // 1111100000000000
                                        ((uint16)(rgb_ptr[1] & 0xFC) << 3) |   // 0000011111100000
                                        ((uint16)(rgb_ptr[2] & 0xF8) >> 3));   // 0000000000011111
           rgb_ptr += 3;
         }
         break;

    default:
         break;
  }
}

color
_GfxGetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
  uint8  *pal_ptr = (uint8 *)gDALGlobals.gfx.palette;
  int     index, i;
  uint32  lowValue, *diff, diffValue;
  int     rd, gd, bd;

  // generate the color "differences" for all colors in the palette
  diff = gDALGlobals.gfx.paletteDiff;
  i    = PALETTE_SIZE;
  do
  {
    bd = ((int)*pal_ptr++ - b);
    gd = ((int)*pal_ptr++ - g);
    rd = ((int)*pal_ptr++ - r);
    pal_ptr++;

    *diff++ = ((int32)rd * rd) + ((int32)gd * gd) + ((int32)bd * bd);
  }
  while (--i);

  // find the palette index that has the smallest color "difference"
  lowValue = 0xffffffff;

  i     = PALETTE_SIZE;
  index = i-1;
  do
  {
    diff--;
    diffValue = *diff;
    if (diffValue < lowValue)
    {
      lowValue = diffValue;
      index = (i-1);
    }
  }
  while (--i);

  // special purpose cases
  if ((r == 255) && (g == 255) && (b == 255)) index = 0;
  if ((r ==   0) && (g ==   0) && (b ==   0)) index = 255;

  return (color)index;
}

void
_GfxGetWindowInfo(gfx_window *win, uint8 **bits, coord *width, coord *height)
{
  _WindowsWindow *window = (_WindowsWindow  *)win;

  *width  = window->width;
  *height = window->height;
  *bits   = window->bits;
}

void
_GfxClearWindow()
{
  _GfxFillRegion(&gDALGlobals.gfx.clip, 0);
}

void
_GfxFillRegion(rectangle *rect, color color)
{
  _WindowsWindow *win = (_WindowsWindow *)gDALGlobals.gfx.activeWindow;

  // lets make sure we have valid windows to mess around with :)
  if ((win != NULL) && (win->bits != NULL))
  {
    coord x1, y1, x2, y2;
    int   j;

    // whats a "valid" box area for this operation?
    x1 = MAX(gDALGlobals.gfx.clip.x, rect->x);
    y1 = MAX(gDALGlobals.gfx.clip.y, rect->y);
    x2 = MIN(gDALGlobals.gfx.clip.x + gDALGlobals.gfx.clip.width,  rect->x + rect->width);
    y2 = MIN(gDALGlobals.gfx.clip.y + gDALGlobals.gfx.clip.height, rect->y + rect->height);

    // do we still have a valid region?
    if ((x2 > x1) && (y2 > y1))
    {
      uint8 *ptr;
      int    cnt;

      // determine the "pointer" value, and how many bytes to blit
      ptr = (uint8 *)(win->bits) + (((uint32)y1 * win->width) + x1);
      cnt = (x2 - x1);

      // do each line...
      for (j=y1; j<y2; j++)
      {
        memset(ptr, (uint8)color, (uint16)cnt);
        ptr += win->width;
      }
    }
  }
}

void
_GfxCopyRegion(gfx_window *srcWin, gfx_window *dstWin, rectangle *rect, coord x, coord y, draw_operation mode)
{
  _WindowsWindow *win_src = (_WindowsWindow *)srcWin;
  _WindowsWindow *win_dst = (_WindowsWindow *)dstWin;

  // lets make sure we have valid windows to mess around with :)
  if (((win_src != NULL) && (win_src->bits != NULL)) &&
      ((win_dst != NULL) && (win_dst->bits != NULL)))
  {
    coord x1, y1, x2, y2;
    int   i, j;

    // whats a "valid" box area for this operation? - source window
    x1 = MAX(0, rect->x);
    y1 = MAX(0, rect->y);
    x2 = MIN((coord)win_src->width,  rect->x + rect->width);
    y2 = MIN((coord)win_src->height, rect->y + rect->height);

    // whats a "valid" box area for this operation? - destination window
    if (x < 0) { x1 = x1 - x; x = 0; }
    if (y < 0) { y1 = y1 - y; y = 0; }
    if ((x + (x2 - x1)) > (coord)win_dst->width)  { x2 = x1 + (win_dst->width  - x); }
    if ((y + (y2 - y1)) > (coord)win_dst->height) { y2 = y1 + (win_dst->height - y); }

    // active 'draw' window - clipping!
    if (gDALGlobals.gfx.activeWindow == dstWin)
    {
      coord _x1, _y1, _x2, _y2;

      _x1 = MAX(gDALGlobals.gfx.clip.x, x);
      _y1 = MAX(gDALGlobals.gfx.clip.y, y);
      if (_x1 != x) { x1 += (_x1 - x); x = _x1; }
      if (_y1 != y) { y1 += (_y1 - y); y = _y1; }

      _x2 = MIN(gDALGlobals.gfx.clip.x + gDALGlobals.gfx.clip.width,  x + (x2-x1));
      _y2 = MIN(gDALGlobals.gfx.clip.y + gDALGlobals.gfx.clip.height, y + (y2-y1));
      if (_x2 != (x + (x2-x1))) { x2 += (_x2 - (x + (x2-x1))); }
      if (_y2 != (y + (y2-y1))) { y2 += (_y2 - (y + (y2-y1))); }
    }

    // do we still have a valid region?
    if ((x2 > x1) && (y2 > y1))
    {
      uint8  *srcP, *dstP;
      int32  dst_delta, src_delta;
      uint16 cnt;

      // determine the "pointer" values..
      srcP = (uint8 *)(win_src->bits) + (((uint32)y1 * win_src->width) + x1);
      dstP = (uint8 *)(win_dst->bits) + (((uint32)y  * win_dst->width) + x);
      cnt  = (x2-x1);

      src_delta = (win_src->width - cnt);
      dst_delta = (win_dst->width - cnt);

      switch (mode)
      {
        case gfxMask:
             {
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
             }
             break;

        case gfxInvert:
             {
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
             }
             break;

        case gfxOverlay:
             {
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
             }
             break;

        case gfxPaint:
        default:
             {
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
             }
             break;
      }
    }
  }
}

void
_GfxSetClip(rectangle *rect)
{
  gDALGlobals.gfx.clip.x      = rect->x;
  gDALGlobals.gfx.clip.y      = rect->y;
  gDALGlobals.gfx.clip.width  = rect->width;
  gDALGlobals.gfx.clip.height = rect->height;
}

void
_GfxGetClip(rectangle *rect)
{
  rect->x      = gDALGlobals.gfx.clip.x;
  rect->y      = gDALGlobals.gfx.clip.y;
  rect->width  = gDALGlobals.gfx.clip.width;
  rect->height = gDALGlobals.gfx.clip.height;
}

void
_GfxResetClip()
{
  _WindowsWindow *window = (_WindowsWindow *)gDALGlobals.gfx.activeWindow;

  gDALGlobals.gfx.clip.x      = 0;
  gDALGlobals.gfx.clip.y      = 0;
  gDALGlobals.gfx.clip.width  = window->width;
  gDALGlobals.gfx.clip.height = window->height;
}

void
_GfxSetPixel(coord x, coord y, color color)
{
  _WindowsWindow *window = (_WindowsWindow *)gDALGlobals.gfx.activeWindow;
  uint8       *ptr;

  if ((x >= gDALGlobals.gfx.clip.x) &&
      (x < (gDALGlobals.gfx.clip.x + gDALGlobals.gfx.clip.width)) &&
      (y >= gDALGlobals.gfx.clip.y) &&
      (y < (gDALGlobals.gfx.clip.y + gDALGlobals.gfx.clip.height)))
  {
    ptr  = window->bits;
    ptr += ((uint32)y * window->width) + x;

    *ptr = color;
  }
}

color
_GfxGetPixel(coord x, coord y)
{
  _WindowsWindow *window = (_WindowsWindow *)gDALGlobals.gfx.activeWindow;
  color        result = 0;
  uint8       *ptr;

  if ((x >= gDALGlobals.gfx.clip.x) &&
      (x < (gDALGlobals.gfx.clip.x + gDALGlobals.gfx.clip.width)) &&
      (y >= gDALGlobals.gfx.clip.y) &&
      (y < (gDALGlobals.gfx.clip.y + gDALGlobals.gfx.clip.height)))
  {
    ptr  = window->bits;
    ptr += ((uint32)y * window->width) + x;

    result = *ptr;
  }

  return result;
}

void
_GfxDrawLine(coord x1, coord y1, coord x2, coord y2, color color)
{
  int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

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

  _GfxSetPixel((coord)px, (coord)py, (uint8)color);

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
      _GfxSetPixel((coord)px, (coord)py, (uint8)color);
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
      _GfxSetPixel((coord)px, (coord)py, (uint8)color);
    }
    while (--i);
  }
}

void
_GfxDrawRectangle(rectangle *rect, color c)
{
  coord x1, y1, x2, y2;

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
_GfxRepaint()
{
  rectangle rect;

  rect.x      = 0;
  rect.y      = 0;
  rect.width  = (coord)gDALGlobals.gfx.width;
  rect.height = (coord)gDALGlobals.gfx.height;

  _GfxCopyToDisplay(gDALGlobals.gfx.window, &rect, 0, 0);
}

void
_GfxRepaintRegion(rectangle *r)
{
  _GfxCopyToDisplay(gDALGlobals.gfx.window, r, r->x, r->y);
}

/********************************* EOF ***********************************/
