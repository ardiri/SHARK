/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxSetClip.c
 */

#include "../SHARK-prv.h"

void
_GfxSetClip(rectangle *rect)
{
  GLOBALS_ACCESS;

  g->gfx.clip.x      = rect->x;
  g->gfx.clip.y      = rect->y;
  g->gfx.clip.width  = rect->width;
  g->gfx.clip.height = rect->height;
}

/********************************* EOF ***********************************/
