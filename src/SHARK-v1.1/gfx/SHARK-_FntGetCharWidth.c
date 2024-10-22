/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FntGetCharWidth.c
 */

#include "../SHARK-prv.h"

uint16
_FntGetCharWidth(uint8 chr)
{
  uint16 result;
  GLOBALS_ACCESS;

  // default return value
  result = 0;

  if (g->gfx.font.winFonts[g->gfx.activeFont] != NULL)
    result = g->gfx.font.fontWidth[g->gfx.activeFont][chr];

  return result;
}

/********************************* EOF ***********************************/
