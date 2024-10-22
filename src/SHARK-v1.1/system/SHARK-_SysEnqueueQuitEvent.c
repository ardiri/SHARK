/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SysEnqueueQuitEvent.c
 */

#include "../SHARK-prv.h"

void
_SysEnqueueQuitEvent()
{
  event e;

  _MemSet(&e, sizeof(event), 0);
  e.eType = _appStopEvent;
  _SysEnqueueEvent(&e);
}

/********************************* EOF ***********************************/
