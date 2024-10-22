/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrCaselessCompare.c
 */

#include "../SHARK-prv.h"

int16
_StrCaselessCompare(uint8 *s1, uint8 *s2)
{
  return (uint16)_StrNCaselessCompare(s1, s2, _StrLen(s2));
}

/********************************* EOF ***********************************/
