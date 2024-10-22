/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxCopyToDisplay_4bpp.c
 */

#include "../SHARK-prv.h"

void
_GfxCopyToDisplay_4bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  void        *bits;
  coord        width, height;
  uint8        depth;
//uint32       size;
  uint8       *pS;
  uint8       *pD;
  uint8       *p1, *p2;
  uint16       hi, lo;
  int          cnt, cnt2, pix;
  int32        win_delta, scr_delta;
  coord        x1, y1, x2, y2;
  _gfx_window *_win;
  GLOBALS_ACCESS;

  _win = (_gfx_window *)win;

  // entry condition, cannot have a NULL window
  if (_win == NULL) return;

  pS = (uint8 *)_win->bits;
  _LCDGetProperties(&bits, &width, &height, &depth);  

  p1   = (uint8 *)g->gfx._indexed;
  p2   = (uint8 *)(p1+1);
  pD   = (uint8 *)bits;

/**
 ** FULL BUFFER CONVERSION
 **

  size = ((uint32)width * (uint32)height) >> 1;
  do
  {
    hi = (uint16)*pS++ << 1;
    lo = (uint16)*pS++ << 1;

    *pD++ = *(p1 + hi) | *(p2 + lo);
  } while (--size);
 
 **/

  // whats a "valid" box area for this operation?
  x1 = MAX(0, rect->x);
  y1 = MAX(0, rect->y);
  x2 = MIN(_win->width,  rect->x + rect->width);
  y2 = MIN(_win->height, rect->y + rect->height);

  // whats a "valid" box area for this operation? - destination window
  if (scr_x < 0) { x1 = x1 - scr_x; scr_x = 0; }
  if (scr_y < 0) { y1 = y1 - scr_y; scr_y = 0; }
  if ((scr_x + (x2 - x1)) > width)  { x2 = x1 + (width  - scr_x); }
  if ((scr_y + (y2 - y1)) > height) { y2 = y1 + (height - scr_y); }

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    pS += (((uint32)y1     * _win->width) + x1);
    pD += ((((uint32)scr_y * width)       + scr_x) >> 1);      // NOTE: 4bpp
  
    cnt = y2-y1;
    pix = x2-x1;

    win_delta =  _win->width - pix;
    scr_delta = (width       - pix) >> 1;
    if ((pix & 0x01) && !(x1 & 0x01)) scr_delta++;

    // do each line...
    do
    {
      cnt2 = pix;

      // prefix pixel
      if (x1 & 0x01)
      {
        lo = (uint16)*pS++ << 1;
        *pD = (*pD & 0xf0) | *(p2 + lo); pD++;
        cnt2--;
      }

      // pixels aligned on byte boundary
      cnt2 >>= 1;
      if (cnt2)
      {
        do
        {
          hi = (uint16)*pS++ << 1;
          lo = (uint16)*pS++ << 1;
      
          *pD++ = *(p1 + hi) | *(p2 + lo);
        }
        while (--cnt2);
      }

      // postfix pixel
      if (x2 & 0x01)
      {
        hi = (uint16)*pS++ << 1;
        *pD = *(p1 + hi) | (*pD & 0xf0);
      }

      pS += win_delta;
      pD += scr_delta;
    }
    while (--cnt);
  }
}

/********************************* EOF ***********************************/
