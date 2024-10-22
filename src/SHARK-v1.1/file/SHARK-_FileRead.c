/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FileRead.c
 */

#include "../SHARK-prv.h"

void
_FileRead(file *f, void *buf, uint16 objSize, uint16 numObject, uint16 *objRead)
{
  DAL_FileRead(f, buf, objSize, numObject, objRead);
}

/********************************* EOF ***********************************/
