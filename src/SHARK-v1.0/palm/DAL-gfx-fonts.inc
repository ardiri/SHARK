/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-fonts.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

// local functions
static void  _GfxDrawChar(uint8, coord, coord, draw_operation) __DAL__;

void
_GfxDrawString(uint8 *string, uint16 len, coord x, coord y, draw_operation mode)
{
  int i;

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

void
_FntDefineFont(font id, uint8 *fontData)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _FontInfo *fontP;
  uint16     i, ofs;
  uint32     size;
  coord      width, height;
  uint8     *ptr, *ptrBuffer;

  fontP = (_FontInfo *)gDALGlobals->gfx.font;

  // lets make sure it aint defined.
  if (fontP->winFonts[id] == NULL)
  {
    width  = (fontData[0] << 8) | fontData[1];
    height = (fontData[2] << 8) | fontData[3];
    size   = (uint32)width * (uint32)height;

    // create the "window"
    fontP->winFonts[id]         = (_PalmWindow *)MemPtrNew(sizeof(_PalmWindow));
    fontP->winFonts[id]->width  = width;
    fontP->winFonts[id]->height = height;
    fontP->fontHacked[id]       = (size > 64000);
    if (fontP->fontHacked[id])
      fontP->winFonts[id]->bits = MemChunkNew(0, size, 0x1200); // memNewChunkFlagNonMovable 0x0200
                                                                // memNewChunkFlagAllowLarge 0x1000
    else
      fontP->winFonts[id]->bits = _MemPtrNew(size, true);  // on the storage heap (if possible)

    // we have to generate the 'window' based on the buffer we had stored
    ptr       = (uint8 *)&fontData[MAX_FONT_CHAR+4];
    ptrBuffer = fontP->winFonts[id]->bits;
    size = size >> 3;

    _MemSemaphore(true);
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
    _MemSemaphore(false);

    // adjust the "data" stuff
    fontP->fontHeight[id] = height;
    fontP->fontWidth[id]  = (uint8 *)&fontData[4];
    fontP->fontOffset[id] = (coord *)MemPtrNew(sizeof(coord) * MAX_FONT_CHAR);

    // calculate the "offsets" for the copying
    ofs = 0;
    for (i=0; i<MAX_FONT_CHAR; i++)
    {
      fontP->fontOffset[id][i] = ofs;
      ofs += fontP->fontWidth[id][i];
    }
  }
}

void
_FntSetFont(font id)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  gDALGlobals->gfx.activeFont = id;
}

font
_FntGetFont()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  return gDALGlobals->gfx.activeFont;
}

void
_FntReleaseFont(font id)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _FontInfo *fontP;

  fontP = (_FontInfo *)gDALGlobals->gfx.font;

  // lets make sure it aint defined.
  if (fontP->winFonts[id] != NULL)
  {
    if (fontP->fontHacked[id])
      MemPtrFree(fontP->winFonts[id]->bits);
    else
      _MemPtrFree(fontP->winFonts[id]->bits); // remember, could be on storage heap
    MemPtrFree(fontP->winFonts[id]);
    MemPtrFree(fontP->fontOffset[id]);
    fontP->fontWidth[id] = NULL;
  }
  fontP->winFonts[id] = NULL;
}

uint16
_FntGetCharsWidth(uint8 *str, uint16 len)
{
  uint16 i, result = 0;

  for (i=0; i<len; i++)
    result += _FntGetCharWidth(str[i]);

  return result;
}

uint16
_FntGetCharWidth(uint8 ch)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _FontInfo *fontP;
  uint16    result = 0;

  fontP = (_FontInfo *)gDALGlobals->gfx.font;
  result = fontP->fontWidth[gDALGlobals->gfx.activeFont][ch];

  return result;
}

uint16
_FntGetFontHeight()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _FontInfo *fontP;
  fontP = (_FontInfo *)gDALGlobals->gfx.font;

  return fontP->fontHeight[gDALGlobals->gfx.activeFont];
}

uint16
_FntGetWordWrap(uint8 *string, uint16 maxPixels)
{
  coord   x, pos, brkpos;
  boolean done;

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

  return pos;
}

//
// private functions
//

static void
_GfxDrawChar(uint8 c, coord x, coord y, draw_operation mode)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _FontInfo  *fontP;
  gfx_window *winSrc, *winDst;
  rectangle   rect;

  fontP = (_FontInfo *)gDALGlobals->gfx.font;

  // configure
  winSrc = (gfx_window *)fontP->winFonts[gDALGlobals->gfx.activeFont];
  winDst = gDALGlobals->gfx.activeWindow;

  rect.x      = fontP->fontOffset[gDALGlobals->gfx.activeFont][c];
  rect.y      = 0;
  rect.width  = fontP->fontWidth[gDALGlobals->gfx.activeFont][c];
  rect.height = fontP->fontHeight[gDALGlobals->gfx.activeFont];

  // copy over the character bitmap!
  _GfxCopyRegion(winSrc, winDst, &rect, x, y, mode);
}

/********************************* EOF ***********************************/
