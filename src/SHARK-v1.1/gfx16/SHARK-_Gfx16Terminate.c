/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_Gfx16Terminate.c
 */

#include "../SHARK-prv.h"

void
_Gfx16Terminate()
{
  GLOBALS_ACCESS;

  // if a 16bpp buffer was allocated, free it now
  if (g->gfx16.bits != NULL) _MemPtrFree(g->gfx16.bits);
  g->gfx16.bits = NULL;
}

/********************************* EOF ***********************************/
