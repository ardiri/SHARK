/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_MemPtrNewX.c
 */

#include "../SHARK-prv.h"

void *
_MemPtrNewX(uint32 size, boolean storage)
{
  return (void *)DAL_MemPtrNewX(size, storage);
}

/********************************* EOF ***********************************/
