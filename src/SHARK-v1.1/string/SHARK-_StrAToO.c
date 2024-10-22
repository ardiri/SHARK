/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrAToO.c
 */

#include "../SHARK-prv.h"

uint16
_StrAToO(uint8 *s)
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
  
    // skip over the '0' if it is there
    if (*x == '0') x++;
  
    // process the number parsing
    while ((*x >= '0') && (*x <= '7'))
    {
      calc = (value << 3) + (*x++ - '0');  // * 8
      if (calc < 65535) value = (uint16)calc;
      else break;
    }
  }

  return (uint16)value;
}

/********************************* EOF ***********************************/
