/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxGetDrawWindow.c
 */

#include "../SHARK-prv.h"

gfx_window *
_GfxGetDrawWindow()
{
  GLOBALS_ACCESS;

  return g->gfx.activeWindow;
}

/********************************* EOF ***********************************/
