/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FileCreate.c
 */

#include "../SHARK-prv.h"

file *
_FileCreate(uint8 *fileName)
{
  return (file *)DAL_FileCreate(fileName);
}

/********************************* EOF ***********************************/
