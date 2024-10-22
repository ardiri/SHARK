/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-PlatformResourceExists.c
 */

#include "../SHARK-prv.h"

boolean
PlatformResourceExists(uint16 resNum)
{
  return (PlatformGetResourceSize(resNum) != 0);
}

/********************************* EOF ***********************************/
