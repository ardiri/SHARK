/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxInitialize.c
 */

#include "../SHARK-prv.h"

boolean
_GfxInitialize()
{
#include "../SHARK-gfx-palette.inc"

  boolean init_ok;
  int     i;
  GLOBALS_ACCESS;

  // assume everything is ok
  init_ok = true;

  // create the 8bpp background buffer 
#if (defined(__PALMOS__) && defined(_M_68K))
  g->gfx.window = _GfxCreateWindow(g->device.display.width, g->device.display.height);
#else
  if (g->device.display.depth != 8)
  {
    // if the depth is not 8bpp, we will have do some conversion (so, allocate)
    g->gfx.window = _GfxCreateWindow(g->device.display.width, g->device.display.height);
  }
  else
  {
    // if the depth is 8bpp, we can avoid a secondary buffer
    g->gfx.window = _GfxCreateWindowFromBuffer(g->device.display.width, g->device.display.height, g->device.display.bits);
  }
#endif
  init_ok = (g->gfx.window != NULL); if (!init_ok) goto GFX_INIT_ABORT;

  // allocate pallete resources
  g->gfx.palette     =  (uint8 *)_MemPtrNew((PALETTE_SIZE * 3), true);
  g->gfx.paletteDiff = (uint32 *)_MemPtrNew((PALETTE_SIZE * sizeof(uint32)), true);
  init_ok = ((g->gfx.palette != NULL) && (g->gfx.paletteDiff != NULL)); if (!init_ok) goto GFX_INIT_ABORT;

  // color lookup tables
  g->gfx._direct    = NULL;
  g->gfx._indexSize = 0;
  g->gfx._indexed   = NULL;

  switch (g->device.display.depth)
  {
    case 1:
    case 2:
    case 4:
         g->gfx._indexSize = 8 / g->device.display.depth;
         g->gfx._indexed   = (uint8 *)_MemPtrNew((uint16)(256 * sizeof(uint8) * g->gfx._indexSize), true);
         break;

    case 8:
         // its a simple 1:1 copy, basic!
         break;

    case 12:
    case 16:
         g->gfx._direct = (uint16 *)_MemPtrNew(256 * sizeof(uint16), true);
         break;

    default:
         _SysDebugMessage("GfxInitialize() - unsupported graphic display depth", false);
         break;
  }
  init_ok = ((g->device.display.depth == 8) || 
             (boolean)((g->gfx._direct != NULL) ^ (g->gfx._indexed != NULL))); if (!init_ok) goto GFX_INIT_ABORT;

  // blitter callback functions
  switch (g->device.display.depth)
  {
    case  1: g->gfx.fnCopyRegion = _GfxCopyToDisplay_1bpp;  break;
    case  2: g->gfx.fnCopyRegion = _GfxCopyToDisplay_2bpp;  break;
    case  4: g->gfx.fnCopyRegion = _GfxCopyToDisplay_4bpp;  break;
#if (defined(__PALMOS__) && defined(_M_68K))
    case  8: g->gfx.fnCopyRegion = _GfxCopyToDisplay_8bpp;  break;
#else
    case  8: g->gfx.fnCopyRegion = NULL;                    break;  // not required
#endif
    case 12: g->gfx.fnCopyRegion = _GfxCopyToDisplay_12bpp; break;
    case 16: g->gfx.fnCopyRegion = _GfxCopyToDisplay_16bpp; break;
    default: break;
  }

  // initialize the "font" resources 
  g->gfx.font.winFonts   = (gfx_window **)_MemPtrNew(sizeof(_gfx_window *) * MAX_FONT, true);
  g->gfx.font.fontHeight = (coord *)_MemPtrNew(sizeof(coord) * MAX_FONT, true);
  g->gfx.font.fontWidth  = (uint8 **)_MemPtrNew(sizeof(uint8 *) * MAX_FONT, true);
  g->gfx.font.fontOffset = (coord **)_MemPtrNew(sizeof(coord *) * MAX_FONT, true);
  init_ok = ((g->gfx.font.winFonts   != NULL) && 
             (g->gfx.font.fontHeight != NULL) && 
             (g->gfx.font.fontWidth  != NULL) &&
             (g->gfx.font.fontOffset != NULL)); if (!init_ok) goto GFX_INIT_ABORT;

  for (i=0; i<MAX_FONT; i++)
  {
    _MemSemaphore(true);
    g->gfx.font.winFonts[i]   = NULL;
    g->gfx.font.fontWidth[i]  = NULL;
    g->gfx.font.fontOffset[i] = NULL;
    _MemSemaphore(false);
  }

  // palette 
  _GfxSetPalette(sys_palette);
  _GfxSetDrawWindow(g->gfx.window);

  // everything ok?
  goto GFX_INIT_SUCCESS;

GFX_INIT_ABORT:

  if (g->gfx.font.fontOffset  != NULL) _MemPtrFree(g->gfx.font.fontOffset);
  if (g->gfx.font.fontWidth   != NULL) _MemPtrFree(g->gfx.font.fontWidth);
  if (g->gfx.font.fontHeight  != NULL) _MemPtrFree(g->gfx.font.fontHeight);
  if (g->gfx.font.winFonts    != NULL) _MemPtrFree(g->gfx.font.winFonts);
  if (g->gfx._indexed         != NULL) _MemPtrFree(g->gfx._indexed);
  if (g->gfx._direct          != NULL) _MemPtrFree(g->gfx._direct);
  if (g->gfx.paletteDiff      != NULL) _MemPtrFree(g->gfx.paletteDiff);
  if (g->gfx.palette          != NULL) _MemPtrFree(g->gfx.palette);
  if (g->gfx.window           != NULL) _GfxDeleteWindow(g->gfx.window);
  g->gfx.font.fontOffset  = NULL;
  g->gfx.font.fontWidth   = NULL;
  g->gfx.font.fontHeight  = NULL;
  g->gfx.font.winFonts    = NULL;
  g->gfx._indexed         = NULL;
  g->gfx._direct          = NULL;
  g->gfx.paletteDiff      = NULL;
  g->gfx.palette          = NULL;
  g->gfx.window           = NULL;

GFX_INIT_SUCCESS:

  return init_ok;
}

/********************************* EOF ***********************************/
