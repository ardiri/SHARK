/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrNChr.c
 */

#include "../SHARK-prv.h"

uint8 *
_StrNChr(uint8 *s, uint8 chr, uint16 count)
{
  return (uint8 *)_MemChr((void *)s, chr, (uint16)(MIN(count, _StrLen(s))));
}

/********************************* EOF ***********************************/
