/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrAToD.c
 */

#include "../SHARK-prv.h"

uint16
_StrAToD(uint8 *s)
{
  uint8  *x;
  uint16  value;
  uint32  calc;

  // default return value
  value = 0; 

  // pre-condition (cannot have null pointer)
  if (s != NULL)
  {
    // use temporary variables for processing
    x = (uint8 *)s;

    // process the number parsing
    while ((*x >= '0') && (*x <= '9'))
    {
      calc = ((uint32)value * 10) + (*x++ - '0');
      if (calc < 65535) value = (uint16)calc;
      else break;
    }
  }

  return (uint16)value;
}

/********************************* EOF ***********************************/
