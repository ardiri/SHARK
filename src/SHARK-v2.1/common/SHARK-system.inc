/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-system.c
 */

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

// add a appStopEvent to the event queue; allowing exit
void
_SysEnqueueQuitEvent()
{
  event e;

  memset(&e, 0, sizeof(event));
  e.eType = _appStopEvent;
  _SysEnqueueEvent(&e);
}

/********************************* EOF ***********************************/
