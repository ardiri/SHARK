/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FileWrite.c
 */

#include "../SHARK-prv.h"

uint16
_FileWrite(file *f, void *buf, uint16 objSize, uint16 numObject, int16 *err)
{
  return (uint16)DAL_FileWrite(f, buf, objSize, numObject, err);
}

/********************************* EOF ***********************************/
