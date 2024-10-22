/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FindFile.c
 */

#include "../SHARK-prv.h"

file *
_FindFile(uint8 *fileName)
{
  return (file *)DAL_FindFile(fileName);
}

/********************************* EOF ***********************************/
