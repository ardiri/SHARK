/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_MemPtrNew.c
 */

#include "../SHARK-prv.h"

void *
_MemPtrNew(uint16 size, boolean storage)
{
  return (void *)DAL_MemPtrNew(size, storage);
}

/********************************* EOF ***********************************/
