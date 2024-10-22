/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrOToA.c
 */

#include "../SHARK-prv.h"

void 
_StrOToA(uint8 *s, uint16 value)
{
  uint8  *x;
  uint16  div, res;
  boolean  padding;

  // pre-condition (cannot have null pointer)
  if (s == NULL) return;

  // use temporary variables for processing
  x       = (uint8 *)s;
  div     = 32768;
  padding = false;

  // number text generation
  *x++ = '0';

  while (div > 0)
  {
    res = (value / div) & 0x07; // mod 8

    if (res != 0) { *x++ = '0' + res; padding = true; }
    else if (padding) *x++ = '0';

    div = div >> 3; // div 8
  }
  *x++ = 0;
}

/********************************* EOF ***********************************/
