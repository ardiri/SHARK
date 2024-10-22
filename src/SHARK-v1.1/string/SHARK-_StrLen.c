/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_StrLen.c
 */

#include "../SHARK-prv.h"

uint16
_StrLen(uint8 *s)
{
  uint8  *x;
  uint16  len;

  // default return value
  len = 0;

  // pre-condition (cannot have null pointer)
  if (s != NULL)
  {
    // use temporary variables for processing
    x = (uint8 *)s;
  
    // look for that \0 character
    while (*x++ != 0) len++;
  }

  return len;
}

/********************************* EOF ***********************************/
