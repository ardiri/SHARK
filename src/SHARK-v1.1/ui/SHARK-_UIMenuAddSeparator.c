/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_UIMenuAddSeparator.c
 */

#include "../SHARK-prv.h"

void
_UIMenuAddSeparator()
{
  _UIMenuAddItem("-", _key_unknown, NULL);
}

/********************************* EOF ***********************************/
