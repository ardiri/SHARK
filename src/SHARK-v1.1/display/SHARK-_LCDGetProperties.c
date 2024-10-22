/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_LCDGetProperties.c
 */

#include "../SHARK-prv.h"

void
_LCDGetProperties(void **bits, coord *width, coord *height, uint8 *depth)
{
  DAL_LCDGetProperties(bits, width, height, depth);
}

/********************************* EOF ***********************************/
