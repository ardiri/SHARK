/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-library.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define LIB_GLOBALS           g_lib
#define GLOBALS_UNINITIALIZED (LIB_GLOBALS == NULL)
#define GLOBALS_DEFINITION    LibGlobals *g_lib = NULL

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-library.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
DAL_LibInitialize()
{
  boolean init_ok;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(LIB_GLOBALS, 0, sizeof(LibGlobals));

  return init_ok;
}

void 
DAL_LibTerminate()
{
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef LIB_GLOBALS

/********************************* EOF ***********************************/
