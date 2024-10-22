/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-resource.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define RES_GLOBALS           g_res
#define GLOBALS_UNINITIALIZED (RES_GLOBALS == NULL)
#define GLOBALS_DEFINITION    ResGlobals *g_res = NULL

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-resource.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
DAL_ResourceInitialize()
{
  boolean init_ok;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(RES_GLOBALS, 0, sizeof(ResGlobals));

  return init_ok;
}

static void *
DAL_ResourceLoad(resource id)
{
  return NULL;
}

static uint32
DAL_ResourceSize(resource id)
{
  return 0;
}

void 
DAL_ResourceTerminate()
{
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef RES_GLOBALS

/********************************* EOF ***********************************/
