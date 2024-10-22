/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformGetUniqueID.c
 */

#include "../DAL-prv.h"

void
DAL_PlatformGetUniqueID(uint8 *str)
{
  // palmos hotsync username
  DlkGetSyncInfo(NULL, NULL, NULL, str, NULL, NULL);
}

/********************************* EOF ***********************************/
