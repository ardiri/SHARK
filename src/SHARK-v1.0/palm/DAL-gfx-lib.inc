/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-gfx-lib.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

void
_GfxDisplayInfo(coord *width, coord *height)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  *width  = (coord)gDALGlobals->gfx.width;
  *height = (coord)gDALGlobals->gfx.height;
}

void
_GfxSetDrawWindow(gfx_window *window)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  gDALGlobals->gfx.activeWindow = window;
  if (window != NULL) _GfxResetClip();
}

gfx_window *
_GfxGetDrawWindow()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  return gDALGlobals->gfx.activeWindow;
}

void
_GfxGetPalette(uint8 *rgb)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  uint8 *pal_ptr = (uint8 *)gDALGlobals->gfx.palette;
  uint8 *rgb_ptr = rgb;
  int    i;

  // copy over palette entries
  for (i=0; i<PALETTE_SIZE; i++)
  {
    pal_ptr++;
    *rgb_ptr++ = *pal_ptr++;
    *rgb_ptr++ = *pal_ptr++;
    *rgb_ptr++ = *pal_ptr++;
  }
}

void
_GfxSetPalette(uint8 *rgb)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  uint8 *pal_ptr = (uint8 *)gDALGlobals->gfx.palette;
  uint8 *rgb_ptr = rgb;
  int    i;
  uint32 romVersion;

  // get the rom version from the device
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

  // copy over palette entries
  for (i=0; i<PALETTE_SIZE; i++)
  {
    *pal_ptr++ = (i & 0xff);
    *pal_ptr++ = *rgb_ptr++;
    *pal_ptr++ = *rgb_ptr++;
    *pal_ptr++ = *rgb_ptr++;  // must ensure in that order exactly!
  }

  // prepare data structures (specific to blitting) for optimization
  switch (gDALGlobals->gfx.depth)
  {
    case 16:
         rgb_ptr = rgb;
         for (i=0; i<PALETTE_SIZE; i++)
         {
           // 5-6-5 16bit display
           gDALGlobals->gfx._16bpp[i] = (((uint16)(rgb_ptr[0] & 0xF8) << 8) |   // 1111100000000000
                                         ((uint16)(rgb_ptr[1] & 0xFC) << 3) |   // 0000011111100000
                                         ((uint16)(rgb_ptr[2] & 0xF8) >> 3));   // 0000000000011111
#define __byte_swap16(n) \
        ( ((((uint16) n) << 8) & 0xFF00) | \
          ((((uint16) n) >> 8) & 0x00FF) )

           // we have to do some funky stuff here if DSA (special)
           if (gDALGlobals->gfx.dsa)
           {
#ifdef _68k_
             if (romVersion >= palmos5)
               gDALGlobals->gfx._16bpp[i] = __byte_swap16(gDALGlobals->gfx._16bpp[i]);
#endif
           }
           else
#ifdef _arm_
             gDALGlobals->gfx._16bpp[i] = __byte_swap16(gDALGlobals->gfx._16bpp[i]);
#endif

           rgb_ptr += 3;
         }
         break;

    case 8:
         // set system palette
         WinPalette(winPaletteSet, 0, PALETTE_SIZE, (RGBColorType *)gDALGlobals->gfx.palette);
         break;

    case 4:
         rgb_ptr = rgb;
         for (i=0; i<PALETTE_SIZE; i++)
         {
           uint8  v;
           uint32 g;

#ifdef GFX_GRAY_DESATURATION
           uint8 min, max;

           min = *rgb_ptr; 
           max = *rgb_ptr++;
           if (*rgb_ptr < min) min = *rgb_ptr;
           if (*rgb_ptr > max) max = *rgb_ptr; rgb_ptr++;
           if (*rgb_ptr < min) min = *rgb_ptr;
           if (*rgb_ptr > max) max = *rgb_ptr; rgb_ptr++;
           g = (max + min) >> 1;
#else
//         g = (((uint32)*rgb_ptr++ * 33) + ((uint32)*rgb_ptr++ * 33) + ((uint32)*rgb_ptr++ * 33)) / 100; // EVEN
           g = (((uint32)*rgb_ptr++ * 30) + ((uint32)*rgb_ptr++ * 59) + ((uint32)*rgb_ptr++ * 11)) / 100; // NTSC
#endif

           // map to 0..15 value
           v = 15 - (g >> 4);  // true gray
//         v = (g >> 4);       // inverted (possibly better for b+w screens)

           // we have to do some funky stuff here if DSA (special)
           if (gDALGlobals->gfx.dsa)
           {
#ifdef _68k_
             if (romVersion < palmos5)
             {
               gDALGlobals->gfx._4bpp[0][i] = v << 4;
               gDALGlobals->gfx._4bpp[1][i] = v;
             }
             else
#endif
             {
               gDALGlobals->gfx._4bpp[0][i] = v;
               gDALGlobals->gfx._4bpp[1][i] = v << 4;
             }
           }
           else
           {
             gDALGlobals->gfx._4bpp[0][i] = v << 4;
             gDALGlobals->gfx._4bpp[1][i] = v;
           }
         }
         break;

    default:
         break;
  }
}

color
_GfxGetPaletteIndex(uint8 r, uint8 g, uint8 b)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  uint8  *pal_ptr = (uint8 *)gDALGlobals->gfx.palette;
  int     index, i;
  uint32  lowValue, *diff, diffValue;
  int     rd, gd, bd;

  // generate the color "differences" for all colors in the palette
  diff = gDALGlobals->gfx.paletteDiff;
  i    = PALETTE_SIZE;
  do
  {
    pal_ptr++;

    rd = ((int)*pal_ptr++ - r);
    gd = ((int)*pal_ptr++ - g);
    bd = ((int)*pal_ptr++ - b);

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
      index    = (i-1);
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
  _PalmWindow *window = (_PalmWindow *)win;

  *width  = window->width;
  *height = window->height;
  *bits   = window->bits;
}

void
_GfxClearWindow()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _GfxFillRegion(&gDALGlobals->gfx.clip, 0);
}

void
_GfxFillRegion(rectangle *rect, color color)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _PalmWindow *win = (_PalmWindow *)gDALGlobals->gfx.activeWindow;

  // lets make sure we have valid windows to mess around with :)
  if ((win != NULL) && (win->bits != NULL))
  {
    coord x1, y1, x2, y2;
    int   j;

    // whats a "valid" box area for this operation?
    x1 = MAX(gDALGlobals->gfx.clip.x, rect->x);
    y1 = MAX(gDALGlobals->gfx.clip.y, rect->y);
    x2 = MIN(gDALGlobals->gfx.clip.x + gDALGlobals->gfx.clip.width,  rect->x + rect->width);
    y2 = MIN(gDALGlobals->gfx.clip.y + gDALGlobals->gfx.clip.height, rect->y + rect->height);

    // do we still have a valid region?
    if ((x2 > x1) && (y2 > y1))
    {
      uint8 *ptr;
      int   cnt;

      // determine the "pointer" value, and how many bytes to blit
      ptr = win->bits + (((uint32)y1 * win->width) + x1);
      cnt = (x2 - x1);

      // do each line...
      j = y2 - y1;
      do
      {
        MemSet(ptr, cnt, color);
        ptr += win->width;
      }
      while (--j);
    }
  }
}

void
_GfxCopyRegion(gfx_window *srcWin, gfx_window *dstWin, rectangle *rect, coord x, coord y, draw_operation mode)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _PalmWindow *win_src = (_PalmWindow *)srcWin;
  _PalmWindow *win_dst = (_PalmWindow *)dstWin;

  // lets make sure we have valid windows to mess around with :)
  if (((win_src != NULL) && (win_src->bits != NULL)) &&
      ((win_dst != NULL) && (win_dst->bits != NULL)))
  {
    coord x1, y1, x2, y2;
    int   i, j;

    // whats a "valid" box area for this operation? - source window
    x1 = MAX(0, rect->x);
    y1 = MAX(0, rect->y);
    x2 = MIN(win_src->width,  rect->x + rect->width);
    y2 = MIN(win_src->height, rect->y + rect->height);

    // whats a "valid" box area for this operation? - destination window
    if (x < 0) { x1 = x1 - x; x = 0; }
    if (y < 0) { y1 = y1 - y; y = 0; }
    if ((x + (x2 - x1)) > (coord)win_dst->width)  { x2 = x1 + (win_dst->width  - x); }
    if ((y + (y2 - y1)) > (coord)win_dst->height) { y2 = y1 + (win_dst->height - y); }

    // active 'draw' window - clipping!
    if (gDALGlobals->gfx.activeWindow == dstWin)
    {
      coord _x1, _y1, _x2, _y2;

      _x1 = MAX(gDALGlobals->gfx.clip.x, x);
      _y1 = MAX(gDALGlobals->gfx.clip.y, y);
      if (_x1 != x) { x1 += (_x1 - x); x = _x1; }
      if (_y1 != y) { y1 += (_y1 - y); y = _y1; }

      _x2 = MIN(gDALGlobals->gfx.clip.x + gDALGlobals->gfx.clip.width,  x + (x2-x1));
      _y2 = MIN(gDALGlobals->gfx.clip.y + gDALGlobals->gfx.clip.height, y + (y2-y1));
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
      srcP = win_src->bits + (((uint32)y1 * win_src->width) + x1);
      dstP = win_dst->bits + (((uint32)y  * win_dst->width) + x);
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
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  gDALGlobals->gfx.clip.x      = rect->x;
  gDALGlobals->gfx.clip.y      = rect->y;
  gDALGlobals->gfx.clip.width  = rect->width;
  gDALGlobals->gfx.clip.height = rect->height;
}

void
_GfxGetClip(rectangle *rect)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  rect->x      = gDALGlobals->gfx.clip.x;
  rect->y      = gDALGlobals->gfx.clip.y;
  rect->width  = gDALGlobals->gfx.clip.width;
  rect->height = gDALGlobals->gfx.clip.height;
}

void
_GfxResetClip()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _PalmWindow *window = (_PalmWindow *)gDALGlobals->gfx.activeWindow;

  gDALGlobals->gfx.clip.x      = 0;
  gDALGlobals->gfx.clip.y      = 0;
  gDALGlobals->gfx.clip.width  = window->width;
  gDALGlobals->gfx.clip.height = window->height;
}

void
_GfxSetPixel(coord x, coord y, color color)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _PalmWindow *window = (_PalmWindow *)gDALGlobals->gfx.activeWindow;
  uint8       *ptr;

  if ((x >= gDALGlobals->gfx.clip.x) &&
      (x < (gDALGlobals->gfx.clip.x + gDALGlobals->gfx.clip.width)) &&
      (y >= gDALGlobals->gfx.clip.y) &&
      (y < (gDALGlobals->gfx.clip.y + gDALGlobals->gfx.clip.height)))
  {
    ptr  = window->bits;
    ptr += ((uint32)y * window->width) + x;

    *ptr = color;
  }
}

color
_GfxGetPixel(coord x, coord y)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _PalmWindow *window = (_PalmWindow *)gDALGlobals->gfx.activeWindow;
  color        result = 0;
  uint8       *ptr;

  if ((x >= gDALGlobals->gfx.clip.x) &&
      (x < (gDALGlobals->gfx.clip.x + gDALGlobals->gfx.clip.width)) &&
      (y >= gDALGlobals->gfx.clip.y) &&
      (y < (gDALGlobals->gfx.clip.y + gDALGlobals->gfx.clip.height)))
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

  _GfxSetPixel(px, py, color);

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
      _GfxSetPixel(px, py, color);
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
      _GfxSetPixel(px, py, color);
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
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  rectangle rect;

  rect.x      = 0;
  rect.y      = 0;
  rect.width  = gDALGlobals->gfx.width;
  rect.height = gDALGlobals->gfx.height;

  _GfxCopyToDisplay(gDALGlobals->gfx.window, &rect, rect.x, rect.y);
}

void
_GfxRepaintRegion(rectangle *r)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _GfxCopyToDisplay(gDALGlobals->gfx.window, r, r->x, r->y);
}

/********************************* EOF ***********************************/
