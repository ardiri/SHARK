/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_MemPtrNew.c
 */

#include "../DAL-prv.h"

void *
DAL_MemPtrNew(uint16 size, boolean storage)
{
  return (void *)DAL_MemPtrNewX((uint32)size, storage);
}

/********************************* EOF ***********************************/
