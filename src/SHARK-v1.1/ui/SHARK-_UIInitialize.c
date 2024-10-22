/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_UIInitialize.c
 */

#include "../SHARK-prv.h"

boolean
_UIInitialize()
{
  GLOBALS_ACCESS;

  // define the default font id (we hope it'll exist)
  g->ui.menu.font_id = font_undefined;

  // reset the menu structures
  _UIMenuReset();

  return true;
}

/********************************* EOF ***********************************/
