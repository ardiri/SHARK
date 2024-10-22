/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_PlaySound.c
 */

#include "../SHARK-prv.h"

void
_PlaySound(uint32 freq, uint16 duration, uint16 volume)
{
  DAL_PlaySound(freq, duration, volume);
}

/********************************* EOF ***********************************/
