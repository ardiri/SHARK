/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SysGetTime.c
 */

#include "../DAL-prv.h"

uint32
DAL_SysGetTime()
{
  return (uint32)(DAL_SysTimeLong() / SECS_TO_FT_MULT);
}

/********************************* EOF ***********************************/
