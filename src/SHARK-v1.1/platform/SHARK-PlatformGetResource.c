/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-PlatformGetResource.c
 */

#include "../SHARK-prv.h"

void *
PlatformGetResource(uint16 resNum)
{
  return (void *)DAL_PlatformGetResource(resNum);
}

/********************************* EOF ***********************************/
