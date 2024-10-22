/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-PlatformGetResourceSize.c
 */

#include "../SHARK-prv.h"

uint16
PlatformGetResourceSize(uint16 resNum)
{
  return (uint16)DAL_PlatformGetResourceSize(resNum);
}

/********************************* EOF ***********************************/
