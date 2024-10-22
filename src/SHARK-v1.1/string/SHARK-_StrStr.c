/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrStr.c
 */

#include "../SHARK-prv.h"

uint8 *
_StrStr(uint8 *s, uint8 *tok)
{
  return (uint8 *)_StrNStr(s, tok, _StrLen(s));
}

/********************************* EOF ***********************************/
