/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FileTell.c
 */

#include "../SHARK-prv.h"

uint32
_FileTell(file *f)
{
  return (uint32)DAL_FileTell(f);
}

/********************************* EOF ***********************************/
