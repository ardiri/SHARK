/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-gfx-prv.h
 */

#ifndef SHARK_GFX_PRV_H
#define SHARK_GFX_PRV_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Module Constants/Structures
 *
 *************************************************************************/

typedef struct _gfx_window
{
  coord   width;
  coord   height;
  void   *bits;
  
  boolean allocated;
} _gfx_window;

#define PALETTE_SIZE     256
#define MAX_FONT         128
#define MAX_FONT_CHAR    256

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

extern boolean     _GfxInitialize();
extern void        _GfxTerminate();

/*************************************************************************
 *
 * SHARK Internal Use API's
 *
 *************************************************************************/

extern void        _GfxDrawChar(uint8, coord, coord, draw_operation);

extern void        _GfxCopyToDisplay_1bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y);
extern void        _GfxCopyToDisplay_2bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y);
extern void        _GfxCopyToDisplay_4bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y);
extern void        _GfxCopyToDisplay_8bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y);
extern void        _GfxCopyToDisplay_12bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y);
extern void        _GfxCopyToDisplay_16bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
