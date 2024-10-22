/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrIToA.c
 */

#include "../SHARK-prv.h"

void 
_StrIToA(uint8 *s, int16 value)
{
  uint8   *x;
  int16    div, res;
  boolean  padding;

  // pre-condition (cannot have null pointer)
  if (s == NULL) return;

  // use temporary variables for processing
  x       = (uint8 *)s;
  div     = 10000;
  padding = false;

  // number text generation
  if (value == 0) { *x++ = '0'; }
  if (value <  0) { *x++ = '-'; value = -value; }

  while (div > 0)
  {
    res = (value / div) % 10;

    if (res != 0) { *x++ = '0' + res; padding = true; }
    else if (padding) *x++ = '0';

    div = div / 10;
  }
  *x++ = 0;
}

/********************************* EOF ***********************************/