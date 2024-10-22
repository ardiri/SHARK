/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SysGetTicks.c
 */

#include "../SHARK-prv.h"

uint32
_SysGetTicks()
{
  return (uint32)DAL_SysGetTicks();
}

/********************************* EOF ***********************************/
