/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrNCopy.c
 */

#include "../SHARK-prv.h"

void
_StrNCopy(uint8 *s, uint8 *sX, uint16 count)
{
  _MemMove((void *)s, (void *)sX, (uint16)(MIN(count, _StrLen(sX)) + 1));
}

/********************************* EOF ***********************************/
