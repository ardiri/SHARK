/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SysDelay.c
 */

#include "../DAL-prv.h"

void
DAL_SysDelay(uint32 millisec)
{
  SysTaskDelay((millisec * SysTicksPerSecond()) / 1000);
}

/********************************* EOF ***********************************/
