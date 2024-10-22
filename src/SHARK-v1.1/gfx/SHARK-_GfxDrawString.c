/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxDrawString.c
 */

#include "../SHARK-prv.h"

void
_GfxDrawString(uint8 *string, uint16 len, coord x, coord y, draw_operation mode)
{
  int i;

  // entry conditions
  if (string == NULL) return;

  // draw each character in the string, one after another
  for (i=0; i<len; i++)
  {
    // dont draw \n chars :)
    if (string[i] != '\n')
    {
      _GfxDrawChar(string[i], x, y, mode);
      x += _FntGetCharWidth(string[i]);
    }
  }
}

/********************************* EOF ***********************************/
