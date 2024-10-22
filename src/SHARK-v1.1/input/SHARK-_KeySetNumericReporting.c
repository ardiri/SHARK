/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_KeySetNumericReporting.c
 */

#include "../SHARK-prv.h"

void
_KeySetNumericReporting(boolean report)
{
  GLOBALS_ACCESS;
  
  // save the key mask value
  g->input.report_numeric = report;
}

/********************************* EOF ***********************************/
