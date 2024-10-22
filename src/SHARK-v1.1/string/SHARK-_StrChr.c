/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrChr.c
 */

#include "../SHARK-prv.h"

uint8 *
_StrChr(uint8 *s, uint8 chr)
{
  return (uint8 *)_MemChr((void *)s, chr, _StrLen(s));
}

/********************************* EOF ***********************************/
