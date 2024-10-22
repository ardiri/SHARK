/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_Gfx16Initialize.c
 */

#include "../SHARK-prv.h"

boolean
_Gfx16Initialize()
{
  GLOBALS_ACCESS;

  // no 16bpp buffer is to be allocated yet (may not be needed)
  g->gfx16.bits = NULL;

  return true;
}

/********************************* EOF ***********************************/
