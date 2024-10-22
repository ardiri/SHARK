/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SysDebugMessage.c
 */

#include "../SHARK-prv.h"

void
_SysDebugMessage(uint8 *msg, boolean dialog)
{
  DAL_SysDebugMessage(msg, dialog);
}

/********************************* EOF ***********************************/
