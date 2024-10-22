/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SysGetTicks.c
 */

#include "../DAL-prv.h"

uint32
DAL_SysGetTicks()
{
  return (uint32)(DAL_SysTimeLong() / (SECS_TO_FT_MULT / 100));
}

/********************************* EOF ***********************************/
