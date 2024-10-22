/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrNCat.c
 */

#include "../SHARK-prv.h"

void
_StrNCat(uint8 *s, uint8 *sX, uint16 count)
{
  _MemMove((void *)(s + _StrLen(s)), (void *)sX, (uint16)(MIN(count, _StrLen(sX)) + 1));
}

/********************************* EOF ***********************************/
