/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-PlatformSavePreference.c
 */

#include "../SHARK-prv.h"

void
PlatformSavePreference(uint16 index, void *buf, uint16 size)
{
  DAL_PlatformSavePreference(index, buf, size);
}

/********************************* EOF ***********************************/
