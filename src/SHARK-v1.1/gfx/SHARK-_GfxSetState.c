/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxSetState.c
 */

#include "../SHARK-prv.h"

void
_GfxSetState(boolean enabled)
{
  GLOBALS_ACCESS;

  // record the state
  g->gfx.enabled = enabled;

  // is _Gfx screen access enabled (ie: double buffering)
  if (g->gfx.enabled)
  {
    // allocate new backscreen buffer if it doesn't exist yet
    if (g->gfx.window == NULL)
      g->gfx.window = _GfxCreateWindow(g->device.display.width, g->device.display.height);
  }

  // is _Gfx screen access disabled (ie: only offscreen 8bpp renderering)
  else
  {
    // clean up window (release it back to heap)
    if (g->gfx.window != NULL) _GfxDeleteWindow(g->gfx.window);
    g->gfx.window  = NULL;
  }

  // need to flag this for any routines that depend on an active window
  g->gfx.activeWindow = g->gfx.window;
}

/********************************* EOF ***********************************/
