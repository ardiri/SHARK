/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-prv.h
 */

#ifndef SHARK_PRV_H
#define SHARK_PRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SHARK.h"
#include "!DAL/DAL.h"

/*************************************************************************
 *
 * Platform API's
 *
 *************************************************************************/

#include    "audio/SHARK-audio-prv.h"
#include  "display/SHARK-display-prv.h"
#include     "file/SHARK-file-prv.h"
#include      "gfx/SHARK-gfx-prv.h"
#include    "gfx16/SHARK-gfx16-prv.h"
#include    "input/SHARK-input-prv.h"
#include   "memory/SHARK-memory-prv.h"
#include   "string/SHARK-string-prv.h"
#include   "system/SHARK-system-prv.h"
#include       "ui/SHARK-ui-prv.h"

#include "platform/SHARK-platform-prv.h"

/*************************************************************************
 *
 * Application Constants/Structures
 *
 *************************************************************************/

typedef struct MemChunkInfo
{
  void           *x;
  boolean         storage:1;
  uint32          size   :31;

  void           *platform;
} MemChunkInfo;

typedef struct
{
  struct
  {
    void         *app_globals;          // application globals

    boolean       fps_set;  
    uint32        fps;                  // desired frames/second (x10 factor)
    uint32        seed;                 // random number seed
  
    struct
    {
      uint16      max;
      uint16      index;
      event      *queue;                // event queue functionality
    } events;

  } system;

  struct
  {
    struct
    {
      uint16      width;
      uint16      height;
      void       *bits;
      void       *palette;
      uint8       depth;
      uint8       padding[3];
    } display;                          // display parameters
  
    struct
    {
      boolean     stylus;
      boolean     analog;
      uint8       padding[2];
    } input;                            // input parameters

    void         *platform;             // device::platform information (XXX_DeviceInfo)
  } device;

  struct
  {
    void         *platform;             // device::audio information (XXX_AudioInfo)
  } audio;

  struct
  {
    boolean       enabled;              // is _GfxXXX screen acccess enabled?

    gfx_window   *window;               // primary display
    gfx_window   *activeWindow;         // active draw window
    rectangle     clip;                 // clipping area

    uint8        *palette;
    uint32       *paletteDiff;

    uint16        _indexSize;
    uint8        *_indexed;             // indexed color mapping { 8bpp --> 1,2,4 }
    uint16       *_direct;              // direct color mapping  { 8bpp --> 12,16 }

    font          activeFont;           // active font
    struct
    {
      gfx_window **winFonts;
      coord       *fontHeight;
      uint8      **fontWidth;
      coord      **fontOffset;
    } font;                             // font specific information

    void        (*fnCopyRegion)(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y);
  } gfx;

  struct
  {
    void         *bits;
  } gfx16;

  struct
  {
    struct
    {
      boolean     visible;              // is the menu visible?
      boolean     special;              // special menu type?
      rectangle   rect;
      gfx_window *win;

      font        font_id;              // the font to draw the menu text
      coord       spacing;

      int16       index;
      int16       count;
      struct
      {
        rectangle rect;
        uint8     label[32];
        uint8     key_code;             // the text and key codes for buttons on screen
      } button[2];
      struct
      {
        uint8     label[32];
        uint8     key_code;             // the text and key codes for each menu item
        struct
        { 
          uint8   count;
          uint8   index, prev;
          uint8   labels[64];
        } nested;
      } item[MAX_MENUS];
    } menu;
  } ui;

  struct
  {
    uint32        keyMask;              // key event filtering mask
    boolean       report_numeric;
    uint8         padding[3];
  } input;

  struct
  {
    MemChunkInfo *info;
    uint16        count;
    uint16        max;
    boolean       max_set;
    uint8         padding[3];           // memory chunk management

    void         *platform;             // device::memory information (XXX_MemInfo)
  } memory;

  void           *platform;             // device information (XXX_PlatformInfo)
  void           *platform_special[2];

} DAL_Globals;

/*************************************************************************
 *
 * Globals Access
 *
 *************************************************************************/

extern DAL_Globals *DAL_GetGlobals();

#define GLOBALS_ACCESS \
  DAL_Globals *g = (DAL_Globals *)DAL_GetGlobals()

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
