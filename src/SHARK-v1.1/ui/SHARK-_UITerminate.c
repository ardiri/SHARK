/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_UITerminate.c
 */

#include "../SHARK-prv.h"

void
_UITerminate()
{
  GLOBALS_ACCESS;

  // if menu is active, need to kill the window
  if (g->ui.menu.win != NULL) _GfxDeleteWindow(g->ui.menu.win);
  g->ui.menu.win = NULL;
}

/********************************* EOF ***********************************/
