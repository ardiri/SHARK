/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrSet.c
 */

#include "../SHARK-prv.h"

void
_StrSet(uint8 *s, uint8 chr)
{
  // pre-condition (cannot have null pointer)
  if (s == NULL) return;

  _MemSet((void *)s, _StrLen(s), chr);
}

/********************************* EOF ***********************************/
