/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrHToA.c
 */

#include "../SHARK-prv.h"

void
_StrHToA(uint8 *s, uint16 value)
{
  uint8  *x;
  uint16  div, res;
  boolean  padding;

  // pre-condition (cannot have null pointer)
  if (s == NULL) return;

  // use temporary variables for processing
  x       = (uint8 *)s;
  div     = 4096;
  padding = true;

  // number text generation
  *x++ = '0';
  *x++ = 'x';

  while (div > 0)
  {
    res = (value / div) & 0x0f; // mod 16

    if (res != 0) 
    {
      if (res < 10) *x++ = '0' + res;
      else          *x++ = 'a' + (res-10);
      padding = true;
    }
    else if (padding) *x++ = '0';

    div = div >> 4; // div 16;
  }
  *x++ = 0;
}

/********************************* EOF ***********************************/
