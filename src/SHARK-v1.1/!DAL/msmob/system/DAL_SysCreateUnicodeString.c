/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SysCreateUnicodeString.c
 */

#include "../DAL-prv.h"

uint16 *
DAL_SysCreateUnicodeString(uint8 *str)
{
  uint16 *result;
  int     i;
  
  result = (uint16 *)_MemPtrNew((uint16)((_StrLen(str)+1) << 1), false);
  if (result != NULL)
  {
    i = 0;
    while (i < _StrLen(str))
      result[i] = str[i++];
    result[i] = 0;
  }
 
  return result;
}

/********************************* EOF ***********************************/
