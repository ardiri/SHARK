/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_MemMove.c
 */

#include "../SHARK-prv.h"

void
_MemMove(void *pDst, void *pSrc, uint16 count)
{
  uint8 *x, *y;
  int    i;

  // pre-condition (cannot have null pointer)
  if ((pDst == NULL) || (pSrc == NULL) || (count == 0)) return;

  // use temporary variables for processing
  x = (uint8 *)pDst;
  y = (uint8 *)pSrc;

  // copy buffer as appropriate
  i = count; 
  do
  {
    *x++ = *y++;
  } while (--i);
}

/********************************* EOF ***********************************/
