/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)DAL.h
 *
 * -- Aaron Ardiri  (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Chip Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#ifndef __DAL_H__
#define __DAL_H__

//
// DAL configuration
//

#include "DAL-configure.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Architecture Data Types
//

// common constants
#define false 0
#define true  (!false)
#ifndef NULL
#define NULL  (void *)0
#endif

// generic "integer" - best for loop counters, generics
typedef unsigned int       uint;

// specific sized integers
typedef char               int8;
typedef unsigned char      uint8;
typedef short              int16;
typedef unsigned short     uint16;
typedef long               int32;
typedef unsigned long      uint32;

// miscellaneous
typedef unsigned char      boolean;
typedef short              coord;

//
// User Input Management
//

#define _key_base               1
#define _key_unknown            _key_base;
#define _key_action             _key_base+1
#define _key_scrollup           _key_base+2
#define _key_scrolldown         _key_base+3
#define _key_scrollselect       _key_base+4
#define _key_padup              _key_base+5
#define _key_paddown            _key_base+6
#define _key_padleft            _key_base+7
#define _key_padright           _key_base+8
#define _key_padselect          _key_base+9
#define _key_option1            _key_base+10
#define _key_option2            _key_base+11
#define _key_pause              _key_base+12

#define KeyAction                (1L << (_key_action))
#define KeyScrollUp              (1L << (_key_scrollup))
#define KeyScrollDown            (1L << (_key_scrolldown))
#define KeyScrollSelect          (1L << (_key_scrollselect))
#define KeyPadUp                 (1L << (_key_padup))
#define KeyPadDown               (1L << (_key_paddown))
#define KeyPadLeft               (1L << (_key_padleft))
#define KeyPadRight              (1L << (_key_padright))
#define KeyPadSelect             (1L << (_key_padselect))
#define KeyOption1               (1L << (_key_option1))
#define KeyOption2               (1L << (_key_option2))
#define KeyPause                 (1L << (_key_pause))

#define KeyAllMask               0xFFFFFFFF

extern void        _KeySetMask(uint32 mask)                                                         __DAL__;
extern uint32      _KeyGetMask()                                                                    __DAL__;
extern uint32      _KeyCurrentState()                                                               __DAL__;

extern boolean     _PenAvailable()                                                                  __DAL__;
extern void        _PenState(coord *penX, coord *penY, boolean *penDown)                            __DAL__;
extern boolean     _PenDisplayToWindow(coord *penX, coord *penY)                                    __DAL__;

//
// Storage Management
//

typedef enum { file_beginning = 0, file_current, file_end } file_position;
typedef struct file file;

extern file       *_FindFile(uint8 *fileName)                                                       __DAL__;

extern boolean     _FileOpen(file *f)                                                               __DAL__;
extern void        _FileSeek(file *f, uint32 ofs, file_position position)                           __DAL__;
extern uint32      _FileTell(file *f)                                                               __DAL__;
extern boolean     _FileEOF(file *f)                                                                __DAL__;
extern void        _FileRead(file *f, void *buf, uint16 objSize, uint16 numObject, uint16 *objRead) __DAL__;
extern uint16      _FileWrite(file *f, void *buf, uint16 objSize, uint16 numObject, int16 *err)     __DAL__;
extern void        _FileClose(file *f)                                                              __DAL__;

//
// Graphics Management
//

#define GFX_WINDOW_BPP 8

typedef uint8 font;
typedef uint8 color;

typedef enum
{
  gfxPaint = 0,      // x = y
  gfxMask,           // x = x & ~y
  gfxInvert,         // x = x ^ y
  gfxOverlay         // x = x | y
} draw_operation;

typedef struct rectangle
{
  coord x;
  coord y;
  coord width;
  coord height;
} rectangle;

typedef struct gfx_window gfx_window;

extern gfx_window *_GfxCreateWindow(coord width, coord height)                                      __DAL__;
extern gfx_window *_GfxCreateWindowFromBuffer(coord width, coord height, uint8 *bits)               __DAL__;
extern void        _GfxDeleteWindow(gfx_window *win)                                                __DAL__;

extern void        _GfxCopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)    __DAL__;

// gfx library routines
extern void        _GfxDisplayInfo(coord *width, coord *height)                                     __DAL__;

extern void        _GfxSetDrawWindow(gfx_window *window)                                            __DAL__;
extern gfx_window *_GfxGetDrawWindow()                                                              __DAL__;

extern void        _GfxGetPalette(uint8 *rgb)                                                       __DAL__;
extern void        _GfxSetPalette(uint8 *rgb)                                                       __DAL__;
extern color       _GfxGetPaletteIndex(uint8 r, uint8 g, uint8 b)                                   __DAL__;

extern void        _GfxGetWindowInfo(gfx_window *win, uint8 **bits, coord *width, coord *height)    __DAL__;
extern void        _GfxClearWindow()                                                                __DAL__;
extern void        _GfxFillRegion(rectangle *rect, color c)                                         __DAL__;
extern void        _GfxCopyRegion(gfx_window *srcWin, gfx_window *dstWin, rectangle *rect, 
                                  coord x, coord y, draw_operation mode)                            __DAL__;

extern void        _GfxSetClip(rectangle *rect);
extern void        _GfxGetClip(rectangle *rect);
extern void        _GfxResetClip();

extern void        _GfxSetPixel(coord x, coord y, color c)                                          __DAL__;
extern color       _GfxGetPixel(coord x, coord y)                                                   __DAL__;
extern void        _GfxDrawLine(coord x1, coord y1, coord x2, coord y2, color c)                    __DAL__;
extern void        _GfxDrawRectangle(rectangle *rect, color c)                                      __DAL__;

extern void        _GfxRepaint()                                                                    __DAL__;
extern void        _GfxRepaintRegion(rectangle *rect)                                               __DAL__;

// gfx font library routines
extern void        _GfxDrawString(uint8 *string, uint16 len, coord x, coord y, draw_operation mode) __DAL__;

extern void        _FntDefineFont(font id, uint8 *fontData)                                         __DAL__;
extern void        _FntSetFont(font id)                                                             __DAL__;
extern font        _FntGetFont()                                                                    __DAL__;
extern void        _FntReleaseFont(font id)                                                         __DAL__;

extern uint16      _FntGetCharsWidth(uint8 *str, uint16 len)                                        __DAL__;
extern uint16      _FntGetCharWidth(uint8 chr)                                                      __DAL__;
extern uint16      _FntGetFontHeight()                                                              __DAL__;
extern uint16      _FntGetWordWrap(uint8 *string, uint16 maxPixels)                                 __DAL__;

//
// Audio Management
//

extern void        _PlaySound(uint32 freq, uint16 duration, uint16 volume)                          __DAL__;

//
// Utility Library
//

extern void       *_MemPtrNew(uint16 size, boolean storage)                                         __DAL__;
extern void        _MemSet(void *ptr, uint16 count, uint8 value)                                    __DAL__;
extern void        _MemMove(void *ptrDst, void *ptrSrc, uint16 count)                               __DAL__;
extern void        _MemPtrFree(void *ptr)                                                           __DAL__;
extern uint16      _MemPtrSize(void *ptr)                                                           __DAL__;
extern void        _MemSemaphore(boolean active)                                                    __DAL__;

extern void        _StrCopy(uint8 *strDst, uint8 *strSrc)                                           __DAL__;
extern uint16      _StrLen(uint8 *str)                                                              __DAL__;
extern void        _StrCat(uint8 *str, uint8 *strCat)                                               __DAL__;
extern uint16      _StrAToI(uint8 *str)                                                             __DAL__;
extern void        _StrIToA(uint8 *str, int16 value)                                                __DAL__;

//
// System interface
//

#define _sysFirstEvent 0x7000

typedef enum
{
  _nilEvent      = _sysFirstEvent,
  _appStartEvent,
  _appStopEvent,
  _penEvent,
  _keyEvent,
  _libFirstEvent = 0x7100,
  _usrFirstEvent = 0x7800,
  _sysLastEvent  = 0x7fff
} sys_event;

struct _GenericEvent { uint16 padding[4]; };
struct _PenEvent     { coord  x; coord y; };
struct _KeyEvent     { uint8  chr;        };

typedef struct event
{
   int16     eType;
   boolean   penDown;
   coord     screenX;
   coord     screenY;

   union
   {
     struct _GenericEvent generic;
     struct _PenEvent     pen;
     struct _KeyEvent     key;
   } data;

} event;

extern void        _SysSetFPS(uint32 fps)                                                           __DAL__;
extern uint32      _SysGetFPS()                                                                     __DAL__;

extern void        _SysEnqueueEvent(event *e)                                                       __DAL__;
extern void        _SysEnqueueQuitEvent()                                                           __DAL__;

extern uint32      _SysGetTime()                                                                    __DAL__;
extern void        _SysDelay(uint32 millisec)                                                       __DAL__;
extern uint16      _SysRandom(uint32 seed)                                                          __DAL__;

extern void        _SysSetGlobals(void *gbls)                                                       __DAL__;
extern void       *_SysGetGlobals()                                                                 __DAL__;
extern void        _SysDebugMessage(uint8 *msg)                                                     __DAL__;

//
// Application Defined Routines
//

extern void        PlatformInitialize()                                                             __DAL__;
extern uint16      PlatformGetResourceSize(uint16 resNum)                                           __DAL__;
extern void       *PlatformGetResource(uint16 resNum)                                               __DAL__;
extern void        PlatformReleaseResource(void *ptr)                                               __DAL__;
extern void        PlatformTerminate()                                                              __DAL__;

extern uint32      ApplicationGetUniqueID();
extern uint8      *ApplicationGetName();
extern uint32      ApplicationGetVersion();

extern void       *ApplicationGetPreferences();
extern uint16      ApplicationGetPreferencesSize();

extern boolean     ApplicationInitialize();
extern boolean     ApplicationHandleEvent(event *e);
extern void        ApplicationTerminate();

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
