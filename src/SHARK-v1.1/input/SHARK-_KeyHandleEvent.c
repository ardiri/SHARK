/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_KeyHandleEvent.c
 */

#include "../SHARK-prv.h"

boolean
_KeyHandleEvent(uint16 key, event *_e)
{
  return (boolean)DAL_KeyHandleEvent(key, _e);
}

/********************************* EOF ***********************************/
