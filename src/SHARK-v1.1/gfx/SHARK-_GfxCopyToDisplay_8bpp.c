/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxCopyToDisplay_8bpp.c
 */

#include "../SHARK-prv.h"

void
_GfxCopyToDisplay_8bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  void        *bits;
  coord        width, height;
  uint8        depth;
//uint32       size;
  uint8       *pS;
  uint8       *pD;
  int          cnt, cnt2, pix;
  int32        win_delta, scr_delta;
  coord        x1, y1, x2, y2;
  _gfx_window *_win;

  _win = (_gfx_window *)win;

  // entry condition, cannot have a NULL window
  if (_win == NULL) return;

  pS = (uint8 *)_win->bits;
  _LCDGetProperties(&bits, &width, &height, &depth);  

  pD   = (uint8 *)bits;

/**
 ** FULL BUFFER CONVERSION
 **

  size = (uint32)width * (uint32)height;
  do
  {
    *pD++ = *pS++;
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
    pS += (((uint32)y1    * _win->width) + x1);
    pD += (((uint32)scr_y * width)       + scr_x);
  
    cnt = y2-y1;
    pix = x2-x1;

    win_delta = _win->width - pix;
    scr_delta = width       - pix;

    // do each line...
    do
    {
      cnt2 = pix;

      // copy bytes if we're dword unaligned
      if ((uint8)((uint32)(pD)) & 0x01)
      {
        *pD++ = *pS++;
        cnt2--;
      }

      // if we're even aligned, copy dwords
      if (((uint8)((uint32)(pS)) & 0x01) == 0)
      {
        int16 dwords = cnt2 >> 2;
        cnt2 &= 0x03;

        if (dwords)
        {
          int16 cnt3;

          cnt3 = (dwords & 0x03);
          if (cnt3)
          {
            dwords -= cnt3;
            do
            {
              *((uint32 *)(pD))++ = *((uint32 *)(pS))++;
            }
            while (--cnt3);
          }

          if (dwords >>= 2)
          {
            do
            {
              *((uint32 *)(pD))++ = *((uint32 *)(pS))++;
              *((uint32 *)(pD))++ = *((uint32 *)(pS))++;
              *((uint32 *)(pD))++ = *((uint32 *)(pS))++;
              *((uint32 *)(pD))++ = *((uint32 *)(pS))++;
            }
            while (--dwords);
          }
        }
      }

      // copy remaining bytes
      if (cnt2)
      {
        do
        {
          *pD++ = *pS++;
        }
        while (--cnt2);
      }

      pS += win_delta;
      pD += scr_delta;
    }
    while (--cnt);
  }
}

/********************************* EOF ***********************************/
