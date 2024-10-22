/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxGetClip.c
 */

#include "../SHARK-prv.h"

void
_GfxGetClip(rectangle *rect)
{
  GLOBALS_ACCESS;

  rect->x      = g->gfx.clip.x;
  rect->y      = g->gfx.clip.y;
  rect->width  = g->gfx.clip.width;
  rect->height = g->gfx.clip.height;
}

/********************************* EOF ***********************************/
