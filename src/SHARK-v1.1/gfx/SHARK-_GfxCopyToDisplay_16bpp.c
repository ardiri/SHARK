/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxCopyToDisplay_16bpp.c
 */

#include "../SHARK-prv.h"

void
_GfxCopyToDisplay_16bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  void        *bits;
  coord        width, height;
  uint8        depth;
//uint32       size;
  uint8       *pS;
  uint16      *pD;
  uint16      *pal;
  int          cnt, pix, i;
  int32        win_delta, scr_delta;
#if !(defined(_M_IX86) || defined(_M_68K))
  uint32       win1, win2;
#endif
  coord        x1, y1, x2, y2;
  _gfx_window *_win;
  GLOBALS_ACCESS;

  _win = (_gfx_window *)win;

  // entry condition, cannot have a NULL window
  if (_win == NULL) return;

  pS = (uint8 *)_win->bits;
  _LCDGetProperties(&bits, &width, &height, &depth);  

  pal  = g->gfx._direct;
  pD   = (uint16 *)bits;

/**
 ** FULL BUFFER CONVERSION
 **

  size = (uint32)width * (uint32)height;
  do
  {
    *pD++ = *(pal + *pS++);
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
      i = pix;
  
#if (defined(_M_IX86) || defined(_M_68K))
      do
      {
        *pD++ = *(pal + *pS++);
      } while (--i);
#else
      // both pD and pS can be aligned on dword boundaries
      if (((uint32)(pD) & 0x3) == ((uint32)(pS) & 0x3))
      {
        win2 = (4 - ((uint32)(pS))) & 0x3;
        if (win2)
        {
          i -= win2;
          do
          {
            win1 = *pS++;
            *pD++ = *(pal + win1);
          }
          while (--win2);
        }  

        // chunks of 8 writes (faster)
        win2 = i & 0x7;
        if (i >>= 3)
        {
          do 
          {
    	      win1 = ((uint32 *)(pS))[0];
            ((uint32 *)(pD))[0] = ((uint32)*(uint16 *)((uint8 *)(pal) + ((win1 >> 7) & 0x1fe)) << 16)  +
              *(uint16 *)((uint8 *)(pal) + ((win1 & 0xff) << 1));
            ((uint32 *)(pD))[1] = ((uint32)*(uint16 *)((uint8 *)(pal) + ((win1 >> 23) & 0x1fe)) << 16) +
              *(uint16 *)((uint8 *)(pal) + ((win1 >> 15) & 0x1fe));
    	      win1 = ((uint32 *)(pS))[1];
            pS += 8;
            ((uint32 *)(pD))[2] = ((uint32)*(uint16 *)((uint8 *)(pal) + ((win1 >> 7) & 0x1fe))  << 16) +
              *(uint16 *)((uint8 *)(pal) + ((win1 & 0xff) << 1));
            ((uint32 *)(pD))[3] = ((uint32)*(uint16 *)((uint8 *)(pal) + ((win1 >> 23) & 0x1fe)) << 16) +
              *(uint16 *)((uint8 *)(pal) + ((win1 >> 15) & 0x1fe));
            pD += 8;
          }
          while (--i);
        }
    
        if (win2)
        {
          do
          {
            win1 = *pS++;
            *pD++ = *(pal + win1);
          }
          while (--win2);
        }
      }

      // nope, have to do it the hard way
      else
      {
        do
        {
          *pD++ = *(pal + *pS++);
        } while (--i);
      }
#endif
  
      pS += win_delta;
      pD += scr_delta;
    }
    while (--cnt);
  }
}

/********************************* EOF ***********************************/
