/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxDrawLine.c
 */

#include "../SHARK-prv.h"

void
_GfxDrawLine(coord x1, coord y1, coord x2, coord y2, color c)
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

  _GfxSetPixel((coord)px, (coord)py, (uint8)c);

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
      _GfxSetPixel((coord)px, (coord)py, (uint8)c);
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
      _GfxSetPixel((coord)px, (coord)py, (uint8)c);
    }
    while (--i);
  }
}

/********************************* EOF ***********************************/
