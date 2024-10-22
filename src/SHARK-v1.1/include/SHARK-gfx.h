/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-gfx.h
 */

#ifndef SHARK_GFX_H
#define SHARK_GFX_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Constants / Structures
 *
 *************************************************************************/

typedef uint8 font;
typedef uint8 color;

#define font_undefined 255

typedef enum
{
  gfxPaint = 0,      // x = y
  gfxMask,           // x = x & ~y
  gfxInvert,         // x = x ^ y
  gfxOverlay         // x = x | y
} draw_operation;

typedef struct gfx_window gfx_window;

/*************************************************************************
 *
 * Application Programmers Interface
 *
 *************************************************************************/

// device specific
// none

// device independent
extern void        _GfxSetState(boolean enabled);

extern gfx_window *_GfxCreateWindow(coord width, coord height);
extern gfx_window *_GfxCreateWindowFromBuffer(coord width, coord height, uint8 *bits);
extern gfx_window *_GfxCreateWindowFromBufferDisposable(coord width, coord height, uint8 *bits);
extern void        _GfxDeleteWindow(gfx_window *win);

extern void        _GfxDisplayInfo(coord *width, coord *height);
extern void        _GfxGetWindowInfo(gfx_window *win, uint8 **bits, coord *width, coord *height);

extern void        _GfxSetDrawWindow(gfx_window *window);
extern gfx_window *_GfxGetDrawWindow();

extern void        _GfxGetPalette(uint8 *rgb);
extern void        _GfxSetPalette(uint8 *rgb);
extern color       _GfxGetPaletteIndex(uint8 r, uint8 g, uint8 b);

extern void        _GfxClearWindow();
extern void        _GfxFillRegion(rectangle *rect, color c);
extern void        _GfxInvertRegion(rectangle *rect);
extern void        _GfxCopyRegion(gfx_window *srcWin, gfx_window *dstWin, rectangle *rect, 
                                  coord x, coord y, draw_operation mode);

extern void        _GfxSetClip(rectangle *rect);
extern void        _GfxGetClip(rectangle *rect);
extern void        _GfxResetClip();

extern void        _GfxSetPixel(coord x, coord y, color c);
extern color       _GfxGetPixel(coord x, coord y);
extern void        _GfxDrawLine(coord x1, coord y1, coord x2, coord y2, color c);
extern void        _GfxDrawRectangle(rectangle *rect, color c);

extern void        _GfxRepaint();
extern void        _GfxRepaintRegion(rectangle *rect);
extern void        _GfxCopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y);

extern void        _GfxDrawString(uint8 *string, uint16 len, coord x, coord y, draw_operation mode);

extern void        _FntDefineFont(font id, uint8 *fontData);
extern void        _FntSetFont(font id);
extern font        _FntGetFont();
extern void        _FntReleaseFont(font id);

extern uint16      _FntGetCharsWidth(uint8 *str, uint16 len);
extern uint16      _FntGetCharWidth(uint8 chr);
extern uint16      _FntGetFontHeight();
extern uint16      _FntGetWordWrap(uint8 *string, uint16 maxPixels);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
