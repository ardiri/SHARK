/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxInvertRegion.c
 */

#include "../SHARK-prv.h"

void
_GfxInvertRegion(rectangle *rect)
{
  _gfx_window *win;
  coord        x1, y1, x2, y2;
  uint8       *ptr;
  int          i, j, cnt;
  GLOBALS_ACCESS;

  // entry condition, cannot have a NULL active window
  if (g->gfx.activeWindow == NULL) return;
  win = (_gfx_window *)g->gfx.activeWindow;

  // whats a "valid" box area for this operation?
  x1 = MAX(0, rect->x);
  y1 = MAX(0, rect->y);
  x2 = MIN(win->width,  rect->x + rect->width);
  y2 = MIN(win->height, rect->y + rect->height);
  
  // active 'draw' window - clipping!
  x1 = MAX(g->gfx.clip.x, x1);
  y1 = MAX(g->gfx.clip.y, y1);
  x2 = MIN(g->gfx.clip.x + g->gfx.clip.width,  x1 + (x2-x1));
  y2 = MIN(g->gfx.clip.y + g->gfx.clip.height, y1 + (y2-y1));

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    // determine the "pointer" value, and how many bytes to blit
    ptr = (uint8 *)(win->bits) + (((uint32)y1 * win->width) + x1);
    cnt = (x2 - x1);

    // do each line...
    for (j=y1; j<y2; j++)
    {
      // do the inner blit
      i = cnt;
      do
      {
        *ptr = ~*ptr; ptr++;
      }
      while (--i);

      ptr += (win->width - cnt);
    }
  }
}

/********************************* EOF ***********************************/
