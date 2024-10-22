/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxCopyToDisplay_2bpp.c
 */

#include "../SHARK-prv.h"

void
_GfxCopyToDisplay_2bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
  void        *bits;
  coord        width, height;
  uint8        depth;
  uint32       size;
  uint8       *pS;
  uint8       *pD;
  uint8       *p1, *p2, *p3, *p4;
  uint16       hi1, hi2, lo1, lo2;
  _gfx_window *_win;
  GLOBALS_ACCESS;

  _win = (_gfx_window *)win;

  // entry condition, cannot have a NULL window
  if (_win == NULL) return;

  pS = (uint8 *)_win->bits;
  _LCDGetProperties(&bits, &width, &height, &depth);  

  p1   = (uint8 *)g->gfx._indexed;
  p2   = (uint8 *)(p1+1); p3   = (uint8 *)(p1+2); p4   = (uint8 *)(p1+3);
 
  size = ((uint32)width * (uint32)height) >> 2;
  pD   = (uint8 *)bits;
  do
  {
    hi1 = (uint16)*pS++ << 2; hi2 = (uint16)*pS++ << 2;
    lo1 = (uint16)*pS++ << 2; lo2 = (uint16)*pS++ << 2;
 
    *pD++ = *(p1 + hi1) | *(p2 + hi2) | *(p3 + lo1) | *(p4 + lo2);
  } while (--size);
}

/********************************* EOF ***********************************/
