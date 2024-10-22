/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-startup.c
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 * PACE Entry Point
 *
 *************************************************************************/

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPB, UInt16 launchFlags);

uint32    
EntryPoint(const void *emustate, void *user, Call68KFuncType *call68K)
{
  uint32 result;

  // default return value
  result = 0;

  // we need to initialize a few things
  GOT = (void *)user;

  // initalize global variables
  g = (GlobalsType *)malloc(sizeof(GlobalsType));
  if (g != NULL)
  {
    memset(g, 0, sizeof(GlobalsType));

    // we may need these if we have to use PACE for something
    g -> PACE._68k = (void *)call68K;
    g -> PACE._emu = (void *)emustate;

    // run the application - direct link to previous application code
    result = PilotMain(sysAppLaunchCmdNormalLaunch, (void *)NULL, 0);

    // clean up globals variables
    free(g); 
  }

  return result;    
}

/********************************* EOF ***********************************/
