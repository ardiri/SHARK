/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrAToI.c
 */

#include "../SHARK-prv.h"

int16
_StrAToI(uint8 *s)
{
  uint8  *x;
  int16   value;
  int32   calc;
  boolean negative;

  // default return value
  value = 0; 

  // pre-condition (cannot have null pointer)
  if (s != NULL)
  {
    // use temporary variables for processing
    negative = false;
    x        = (uint8 *)s;
  
    // process the number parsing
    if ((*x) == '-') { negative = true; x++; }
  
    while ((*x >= '0') && (*x <= '9'))
    {
      calc = ((uint32)value * 10) + (*x++ - '0');
      if ((calc > -32768) && (calc < 32767)) value = (uint16)calc;
      else break;
    }
  }

  return (int16)value;
}

/********************************* EOF ***********************************/
