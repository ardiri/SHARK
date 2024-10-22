/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-globals.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 * Platform Implementation Structures
 *
 *************************************************************************/

// none

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

static void
GlobalsInitialize()
{
  // initialize the modules
  _SysInitialize();
  _MemInitialize();
  _IOInitialize();
  _StrInitialize();
  _GfxInitialize();
  _AudioInitialize();
  _FileInitialize();

  // platform specific initialization (load fonts etc)
  PlatformInitialize();
}

static void
GlobalsTerminate()
{
  // platform specific shutdown (release fonts etc)
  PlatformTerminate();

  // shutdown the modules
  _FileTerminate();
  _AudioTerminate();
  _GfxTerminate();
  _StrTerminate();
  _IOTerminate();
  _MemTerminate();
  _SysTerminate();
}

/********************************* EOF ***********************************/

