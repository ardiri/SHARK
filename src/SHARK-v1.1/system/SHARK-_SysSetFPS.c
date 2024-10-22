/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SysSetFPS.c
 */

#include "../SHARK-prv.h"

void
_SysSetFPS(uint32 fps)
{
  GLOBALS_ACCESS;

  // we will NOT allow the developer to change the FPS once set
  if (g->system.fps_set) return;

  g->system.fps     = fps;
  g->system.fps_set = true;
  DAL_SysSetFPS(fps);
}

/********************************* EOF ***********************************/
