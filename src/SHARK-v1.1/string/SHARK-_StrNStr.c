/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrNStr.c
 */

#include "../SHARK-prv.h"

uint8 *
_StrNStr(uint8 *s, uint8 *tok, uint16 count)
{
  uint8 *pos;
  uint8 *x;
  int    i;

  // default return value
  pos = NULL;

  // pre-condition (cannot have null pointer)
  if (s != NULL)
  {
    // use temporary variables for processing
    x = (uint8 *)s;
  
    // lets scan the memory buffer
    i = count; 
    do
    {
      if (_StrCompare(x, tok) == 0) { pos = x; break; }
      x++;
    } while (--i);
  }

  return pos;
}

/********************************* EOF ***********************************/
