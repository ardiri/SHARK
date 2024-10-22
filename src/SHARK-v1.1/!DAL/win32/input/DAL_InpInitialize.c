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
  GLOBALS_ACCESS;

  // LOGGING EVENT
  LOG_ENTRY(">> InpInitialize");

  // setup default values
  g->input.keyMask        = 0;
  g->input.report_numeric = false;

  return true;
}

/********************************* EOF ***********************************/
