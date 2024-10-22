/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrNCaselessCompare.c
 */

#include "../SHARK-prv.h"

int16
_StrNCaselessCompare(uint8 *s1, uint8 *s2, uint16 count)
{
  uint8 *x, *y;
  int16  diff;
  int    i;

  // default return value
  diff = 0;

  // pre-condition (cannot have null pointer)
  if ((s1 != NULL) && (s2 != NULL) && (count > 0))
  {
    // use temporary variables for processing
    x = (uint8 *)s1;
    y = (uint8 *)s2;
  
    count = MIN(count, _StrLen(s1));
    count = MIN(count, _StrLen(s2)); // ensure within bounds
  
    // copy buffer as appropriate
    i = count; 
    do
    {
      diff = (*x++ | 0x20) - (*y++ | 0x20);
    } while ((--i) && (diff == 0));
  }

  return diff;
}

/********************************* EOF ***********************************/
