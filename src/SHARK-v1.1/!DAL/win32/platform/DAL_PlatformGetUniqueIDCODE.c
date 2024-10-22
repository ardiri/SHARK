/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformGetUniqueIDCODE.c
 */

#include "../DAL-prv.h"

uint16
DAL_PlatformGetUniqueIDCODE()
{
  uint16 code;
  uint8  unique_str[64];
  int    i;

  // starting hash = based on application unique id
  code = ((uint16)(~ApplicationGetUniqueID() & 0xFFFF) ^ (uint16)(ApplicationGetUniqueID() >> 16));

  // lets throw in the ASCII values of the unique string
  PlatformGetUniqueID(unique_str);
  for (i=0; i<_StrLen(unique_str); i++) code += unique_str[i];

  // PLATFORM SPECIFIC ADDITIONS:: (if any)

  return code;
}

/********************************* EOF ***********************************/
