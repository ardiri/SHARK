/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FileEOF.c
 */

#include "../SHARK-prv.h"

boolean
_FileEOF(file *f)
{
  return (boolean)DAL_FileEOF(f);
}

/********************************* EOF ***********************************/
