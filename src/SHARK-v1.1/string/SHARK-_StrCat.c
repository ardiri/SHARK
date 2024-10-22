/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrCat.c
 */

#include "../SHARK-prv.h"

void
_StrCat(uint8 *s, uint8 *sX)
{
  // pre-condition (cannot have null pointer)
  if ((s == NULL) || (sX == NULL)) return;

  _MemMove((void *)(s + _StrLen(s)), (void *)sX, (uint16)(_StrLen(sX) + 1));
}

/********************************* EOF ***********************************/
