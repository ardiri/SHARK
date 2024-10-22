/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-PlatformLoadPreference.c
 */

#include "../SHARK-prv.h"

boolean
PlatformLoadPreference(uint16 index, void *buf, uint16 size)
{
  return DAL_PlatformLoadPreference(index, buf, size);
}

/********************************* EOF ***********************************/
