/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FntDefineFont.c
 */

#include "../SHARK-prv.h"

void
_FntDefineFont(font id, uint8 *fontData)
{
  uint16       i, ofs;
  uint32       size;
  coord        width, height;
  uint8       *ptr, *ptrBuffer;
  _gfx_window *win;
  GLOBALS_ACCESS;

  // lets make sure it aint defined.
  if (g->gfx.font.winFonts[id] == NULL)
  {
    _MemSemaphore(true);

    width  = (fontData[0] << 8) | fontData[1];
    height = (fontData[2] << 8) | fontData[3];
    size   = (uint32)width * (uint32)height;

    // create the "window"
    g->gfx.font.winFonts[id] = _GfxCreateWindow(width, height);

    // we have to generate the 'window' based on the buffer we had stored
    ptr       = (uint8 *)&fontData[MAX_FONT_CHAR+4];
    win       = (_gfx_window *)g->gfx.font.winFonts[id];
    ptrBuffer = win->bits;
    size = size >> 3;

    for (i=0; i<size; i++)
    {
      *ptrBuffer++ = (*ptr & 0x80) ? 0xff : 0x00;
      *ptrBuffer++ = (*ptr & 0x40) ? 0xff : 0x00;
      *ptrBuffer++ = (*ptr & 0x20) ? 0xff : 0x00;
      *ptrBuffer++ = (*ptr & 0x10) ? 0xff : 0x00;
      *ptrBuffer++ = (*ptr & 0x08) ? 0xff : 0x00;
      *ptrBuffer++ = (*ptr & 0x04) ? 0xff : 0x00;
      *ptrBuffer++ = (*ptr & 0x02) ? 0xff : 0x00;
      *ptrBuffer++ = (*ptr & 0x01) ? 0xff : 0x00;
      ptr++;
    }

    // adjust the "data" stuff
    g->gfx.font.fontHeight[id] = height;
    g->gfx.font.fontWidth[id]  = (uint8 *)&fontData[4];
    g->gfx.font.fontOffset[id] = (coord *)_MemPtrNew((sizeof(coord) * MAX_FONT_CHAR), true);

    // calculate the "offsets" for the copying
    ofs = 0;
    for (i=0; i<MAX_FONT_CHAR; i++)
    {
      g->gfx.font.fontOffset[id][i] = ofs;
      ofs += g->gfx.font.fontWidth[id][i];
    }

    _MemSemaphore(false);
  }
}

/********************************* EOF ***********************************/
