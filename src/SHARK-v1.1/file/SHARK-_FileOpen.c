/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FileOpen.c
 */

#include "../SHARK-prv.h"

boolean
_FileOpen(file *f)
{
  return (boolean)DAL_FileOpen(f);
}

/********************************* EOF ***********************************/
