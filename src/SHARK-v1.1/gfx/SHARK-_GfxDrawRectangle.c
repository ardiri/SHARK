/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxDrawRectangle.c
 */

#include "../SHARK-prv.h"

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

/********************************* EOF ***********************************/
