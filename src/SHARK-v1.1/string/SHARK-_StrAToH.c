/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrAToH.c
 */

#include "../SHARK-prv.h"

uint16
_StrAToH(uint8 *s)
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
  
    // skip over the '0x' if it is there
    if ((*x == '0') && (*(x+1) == 'x')) x += 2;
  
    // process the number parsing
    while (((*x >= '0') && (*x <= '9')) || (((*x | 0x20) >= 'a') && ((*x | 0x20) <= 'f')))
    {
      if ((*x >= '0') && (*x <= '9')) 
        calc = (value << 4) + (*x++ - '0'); // * 16
      else
        calc = (value << 4) + (((*x++ | 0x20) - 'a') + 10); // * 16
      
      if (calc < 65535) value = (uint16)calc;
      else break;
    }
  }

  return (uint16)value;
}

/********************************* EOF ***********************************/
