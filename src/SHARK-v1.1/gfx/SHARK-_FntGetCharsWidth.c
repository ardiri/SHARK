/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FntGetCharsWidth.c
 */

#include "../SHARK-prv.h"

uint16
_FntGetCharsWidth(uint8 *str, uint16 len)
{
  uint16 i, result;

  // default return value
  result = 0;

  if (str != NULL)
  {
    for (i=0; i<len; i++)
      result += _FntGetCharWidth(str[i]);
  }

  return result;
}

/********************************* EOF ***********************************/
