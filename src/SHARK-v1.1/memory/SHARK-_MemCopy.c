/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_MemCopy.c
 */

#include "../SHARK-prv.h"

void
_MemCopy(void *pDst, void *pSrc, uint16 count)
{
  _MemMove(pDst, pSrc, count);
}

/********************************* EOF ***********************************/
