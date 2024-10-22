/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_MemCompare.c
 */

#include "../SHARK-prv.h"

uint16
_MemCompare(void *p1, void *p2, uint16 count)
{
  uint8  *x, *y;
  uint16  diff;
  int     i;

  // default return value
  diff = 0;

  // pre-condition (cannot have null pointer)
  if ((p1 != NULL) && (p2 != NULL) && (count != 0))
  {
    // use temporary variables for processing
    x    = (uint8 *)p1;
    y    = (uint8 *)p2;
  
    // compare memory buffer as appropriate
    i = count; 
    do
    {
      if (*x == *y) { x++; y++; }
      else
      {
        diff = *x - *y;
        break;
      }
    } while (--i);
  }

  return diff;
}

/********************************* EOF ***********************************/
