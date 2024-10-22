/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxCopyRegion.c
 */

#include "../SHARK-prv.h"

void
_GfxCopyRegion(gfx_window *srcWin, gfx_window *dstWin, rectangle *rect, coord x, coord y, draw_operation mode)
{
  coord        x1,  y1,  x2,  y2;
  coord        _x1, _y1, _x2, _y2;
  int          i, j;
  uint8       *srcP, *dstP;
  int32        dst_delta, src_delta;
  uint16       cnt;
  _gfx_window *_srcWin, *_dstWin;
  GLOBALS_ACCESS;

  _srcWin = (_gfx_window *)srcWin;
  _dstWin = (_gfx_window *)dstWin;

  // entry conditions, NULL window or bits doesn't make sense
  if (((_srcWin == NULL) || (_srcWin->bits == NULL)) ||
      ((_dstWin == NULL) || (_dstWin->bits == NULL))) return;

  // whats a "valid" box area for this operation? - source window
  x1 = MAX(0, rect->x);
  y1 = MAX(0, rect->y);
  x2 = MIN((coord)_srcWin->width,  rect->x + rect->width);
  y2 = MIN((coord)_srcWin->height, rect->y + rect->height);

  // whats a "valid" box area for this operation? - destination window
  if (x < 0) { x1 = x1 - x; x = 0; }
  if (y < 0) { y1 = y1 - y; y = 0; }
  if ((x + (x2 - x1)) > (coord)_dstWin->width)  { x2 = x1 + (_dstWin->width  - x); }
  if ((y + (y2 - y1)) > (coord)_dstWin->height) { y2 = y1 + (_dstWin->height - y); }

  // active 'draw' window - clipping!
  if ((_gfx_window *)g->gfx.activeWindow == _dstWin)
  {
    _x1 = MAX(g->gfx.clip.x, x);
    _y1 = MAX(g->gfx.clip.y, y);
    if (_x1 != x) { x1 += (_x1 - x); x = _x1; }
    if (_y1 != y) { y1 += (_y1 - y); y = _y1; }

    _x2 = MIN(g->gfx.clip.x + g->gfx.clip.width,  x + (x2-x1));
    _y2 = MIN(g->gfx.clip.y + g->gfx.clip.height, y + (y2-y1));
    if (_x2 != (x + (x2-x1))) { x2 += (_x2 - (x + (x2-x1))); }
    if (_y2 != (y + (y2-y1))) { y2 += (_y2 - (y + (y2-y1))); }
  }

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    // determine the "pointer" values..
    srcP = (uint8 *)(_srcWin->bits) + (((uint32)y1 * _srcWin->width) + x1);
    dstP = (uint8 *)(_dstWin->bits) + (((uint32)y  * _dstWin->width) + x);
    cnt  = (x2-x1);

    src_delta = (_srcWin->width - cnt);
    dst_delta = (_dstWin->width - cnt);

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

/********************************* EOF ***********************************/
