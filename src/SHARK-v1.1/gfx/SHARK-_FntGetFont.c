/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FntGetFont.c
 */

#include "../SHARK-prv.h"

font
_FntGetFont()
{
  GLOBALS_ACCESS;

  return g->gfx.activeFont;
}

/********************************* EOF ***********************************/
