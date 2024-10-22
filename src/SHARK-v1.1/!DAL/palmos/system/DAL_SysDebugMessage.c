/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SysDebugMessage.c
 */

#include "../DAL-prv.h"

void
DAL_SysDebugMessage(uint8 *msg, boolean dialog)
{
  // do we need to show a dialog message?
  if (dialog)
    FrmCustomAlert(DebugAlert, msg, NULL, NULL);
}

/********************************* EOF ***********************************/
