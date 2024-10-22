/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_MemSet.c
 */

#include "../SHARK-prv.h"

void
_MemSet(void *p, uint16 count, uint8 value)
{
  uint8 *x;
  int    i;

  // pre-condition (cannot have null pointer)
  if ((p == NULL) || (count == 0)) return;

  // use temporary variables for processing
  x = (uint8 *)p;

  // fill buffer with value
  i = count; 
  do
  {
    *x++ = value;
  } while (--i);
}

/********************************* EOF ***********************************/
