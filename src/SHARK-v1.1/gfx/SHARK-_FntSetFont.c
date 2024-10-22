/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FntSetFont.c
 */

#include "../SHARK-prv.h"

void
_FntSetFont(font id)
{
  GLOBALS_ACCESS;

  g->gfx.activeFont = id;
}

/********************************* EOF ***********************************/
