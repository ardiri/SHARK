/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_KeySetMask.c
 */


#include "../SHARK-prv.h"

void
_KeySetMask(uint32 mask)
{
  GLOBALS_ACCESS;

  // save the key mask value
  g->input.keyMask = mask;
}

/********************************* EOF ***********************************/
