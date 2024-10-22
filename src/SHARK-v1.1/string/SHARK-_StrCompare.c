/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrCompare.c
 */

#include "../SHARK-prv.h"

int16
_StrCompare(uint8 *s1, uint8 *s2)
{
  return (uint16)_StrNCompare(s1, s2, _StrLen(s2));
}

/********************************* EOF ***********************************/
