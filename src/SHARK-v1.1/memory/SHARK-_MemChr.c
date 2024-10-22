/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_MemChr.c
 */

#include "../SHARK-prv.h"

void *
_MemChr(void *p, uint8 chr, uint16 count)
{
  void  *pos;
  uint8 *x;
  int    i;

  // default return value
  pos = NULL;

  // pre-condition (cannot have null pointer)
  if ((p != NULL) && (count != 0))
  {
    // use temporary variables for processing
    x = (uint8 *)p;
  
    // lets scan the memory buffer
    i = count; 
    do
    {
      if (*x == chr) { pos = (void *)x; break; }
      x++;
    } while (--i);
  }

  return pos;
}

/********************************* EOF ***********************************/
