/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_InpInitialize.c
 */

#include "../DAL-prv.h"

boolean
DAL_InpInitialize()
{
#ifdef INP_HANDLE_KEY_UP_DN_MASK
  MSmob_PlatformInfo *platform;
#endif
  GLOBALS_ACCESS;

  // setup default values
  g->input.keyMask        = 0;
  g->input.report_numeric = false;

#ifdef INP_HANDLE_KEY_UP_DN_MASK
  // access device information block
  platform = (MSmob_PlatformInfo *)g->platform;
  if (platform != NULL)
    platform->input.appKeyState = 0;
#endif

  return true;
}

/********************************* EOF ***********************************/
