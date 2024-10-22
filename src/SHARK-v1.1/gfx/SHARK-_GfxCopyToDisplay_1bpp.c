/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxCopyToDisplay_1bpp.c
 */

#include "../SHARK-prv.h"

void
_GfxCopyToDisplay_1bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  void        *bits;
  coord        width, height;
  uint8        depth;
  uint32       size;
  uint8       *pS;
  uint8       *pD;
  uint8       *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8;
  uint16       hi1, hi2, hi3, hi4, lo1, lo2, lo3, lo4;
  _gfx_window *_win;
  GLOBALS_ACCESS;

  _win = (_gfx_window *)win;

  // entry condition, cannot have a NULL window
  if (_win == NULL) return;

  pS = (uint8 *)_win->bits;
  _LCDGetProperties(&bits, &width, &height, &depth);  

  p1   = (uint8 *)g->gfx._indexed;                p2   = (uint8 *)(p1+1); 
  p3   = (uint8 *)(p1+2); p4   = (uint8 *)(p1+3); p5   = (uint8 *)(p1+4);
  p6   = (uint8 *)(p1+5); p7   = (uint8 *)(p1+6); p8   = (uint8 *)(p1+7);
 
  size = ((uint32)width * (uint32)height) >> 3;
  pD   = (uint8 *)bits;
  do
  {
    hi1 = (uint16)*pS++ << 3; hi2 = (uint16)*pS++ << 3; 
    hi3 = (uint16)*pS++ << 3; hi4 = (uint16)*pS++ << 3;
    lo1 = (uint16)*pS++ << 3; lo2 = (uint16)*pS++ << 3;
    lo3 = (uint16)*pS++ << 3; lo4 = (uint16)*pS++ << 3;
 
    *pD++ = *(p1 + hi1) | *(p2 + hi2) | *(p3 + hi3) | *(p4 + hi4) | 
            *(p5 + lo1) | *(p6 + lo2) | *(p7 + lo3) | *(p8 + lo4);
  } while (--size);
}

/********************************* EOF ***********************************/
