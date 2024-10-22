/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_PenState.c
 */

#include "../SHARK-prv.h"

void
_PenState(coord *penX, coord *penY, boolean *penDown)
{
  if (_PenAvailable())
    DAL_PenState(penX, penY, penDown);
  else
  {
    if (penDown != NULL) *penDown = false;
    if (penX != NULL)    *penX = (coord)0;
    if (penY != NULL)    *penY = (coord)0;
  }
}

/********************************* EOF ***********************************/
