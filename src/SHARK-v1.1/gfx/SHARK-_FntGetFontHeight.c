/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FntGetFontHeight.c
 */

#include "../SHARK-prv.h"

uint16
_FntGetFontHeight()
{
  uint16 result;
  GLOBALS_ACCESS;

  // default return value
  result = 0;

  if (g->gfx.font.winFonts[g->gfx.activeFont] != NULL)
    result = g->gfx.font.fontHeight[g->gfx.activeFont];

  return result;
}

/********************************* EOF ***********************************/
