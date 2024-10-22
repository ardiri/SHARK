/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrCopy.c
 */

#include "../SHARK-prv.h"

void
_StrCopy(uint8 *s, uint8 *sX)
{
  // pre-condition (cannot have null pointer)
  if ((s == NULL) || (sX == NULL)) return;

  _MemMove((void *)s, (void *)sX, (uint16)(_StrLen(sX) + 1));
}

/********************************* EOF ***********************************/
