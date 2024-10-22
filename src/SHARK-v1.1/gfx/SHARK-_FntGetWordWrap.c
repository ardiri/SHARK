/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FntGetWordWrap.c
 */

#include "../SHARK-prv.h"

uint16
_FntGetWordWrap(uint8 *string, uint16 maxPixels)
{
  coord   x, pos, brkpos;
  boolean done;

  // default return value
  pos = 0;

  if (string != NULL)
  {
    // initialize
    x = pos = brkpos = 0;
    done = (string[pos] == '\0');
  
    // process
    while (!done)
    {
      uint8 chr = string[pos++];
  
      // what character do we have?
      switch (chr)
      {
        case ' ':
        case '\t':
        case '\n':
             if (x > maxPixels)
             {
               pos  = (brkpos != 0) ? brkpos : pos;
               while (string[pos] == ' ') pos++;
               done = true;
             }
             else
               brkpos = pos;
  
             x += _FntGetCharWidth(chr);
             break;
  
        default:
             x += _FntGetCharWidth(chr);
             if (x > maxPixels)
             {
               pos  = (brkpos != 0) ? brkpos : pos - 1;
               done = true;
             }
             break;
      }
  
      // are we done?
      done |= (string[pos] == '\0');
      done |= (string[pos] == '\n');
    }
  }

  return pos;
}

/********************************* EOF ***********************************/
