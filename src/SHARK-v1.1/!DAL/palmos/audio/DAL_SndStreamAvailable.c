/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SndStreamAvailable.c
 */

#include "../DAL-prv.h"

boolean
DAL_SndStreamAvailable()
{
  PALM_PlatformInfo *platform;
  boolean result;
  GLOBALS_ACCESS;

  // default return value
  result = false;

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
    result = platform->audio.streaming;

  return result;
}

/********************************* EOF ***********************************/
