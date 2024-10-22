/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)windows/DAL-prv.h
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#ifndef _DAL_PRV_
#define _DAL_PRV_

/*************************************************************************
 *
 *    Platform Check
 *
 *************************************************************************/

#ifdef _WIN32_WCE
#if (_WIN32_WCE < 300)
#error "SHARK only supports Pocket PC (WinCE 3.0) or better."
#endif
#endif

// #define GFX_12BPP_CUSTOM 1   // experimental 12bpp blitter support

/*************************************************************************
 *
 *    Include Files
 *
 *************************************************************************/

// system includes
#include <stdarg.h>
#include <stdio.h>

#include "stdafx.h"

#include <commctrl.h>
#ifdef _WIN32_WCE
#include <aygshell.h>
#include <sipapi.h>
#include <Projects.h>
#else
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <mmsystem.h>
#endif

#include "../DAL.h"
#include "application_resource.h"

/*************************************************************************
 *
 *    Application Constants
 *
 *************************************************************************/

#define MAX_LOADSTRING          100

#ifndef _WIN32_WCE
#define USE_HIGH_FREQ_TIMER
#endif

#define EVT_WAIT_FOREVER        0xFFFFFFFF

#define DEFAULT_TICKS_PER_SEC   1000
#define DEFAULT_DEVICE_DEPTH    16
#define DEFAULT_TIMER_ID        1

#define DEFAULT_PREF_FILENAME   "preferences.sav"

#define FILENAME_SIZE           256

#define PALETTE_SIZE            256
#define QUEUE_SIZE              128

#define MAX_MEM                 128

#define MAX_FONT                128
#define MAX_FONT_CHAR           256

// helper macros
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// screen constants
#define TITLE_SIZE              26

/*************************************************************************
 *
 *    Application Structures
 *
 *************************************************************************/

typedef struct
{
  void         *app_globals;            // application globals

  HWND          LCDWnd;                 // LCD window handle
  HDC           LCDScreen;              // LCD device context
  HINSTANCE     hInst;                  // global application instance

#ifdef _WIN32_WCE
  SHACTIVATEINFO  s_sai;                // hande to active application
#endif
  UINT          timerID;                // global timer
  HACCEL        hAccelTable;            // accelerator table

  UINT          evtTimeOut;             // event handling time out controller
  UINT          timerDiff;
  UINT          ticksPerFrame;
  UINT          ticksPerSecond;
  UINT          timerPointA;
  UINT          timerPointB;

  struct
  {
    uint32      fps;                    // desired frames/second (x10 factor)
    uint32      active;                 // is application active? (ie: has focus)
    uint32      seed;                   // random number seed

    uint16      q_index;
    uint16      q_max;
    event      *eventQ;                 // event queue functionality
  } sys;

  struct
  {
    int32       count;
    void       *info;                   // memory 'management' chunk data
  } mem;

  struct
  {
    uint32      width;                  // width of display
    uint32      height;                 // height of display
    uint32      depth;                  // depth of display
#ifndef _WIN32_WCE
    float       stretch_x;              // blitting stretching x-axis (if applicable)
    float       stretch_y;              // blitting stretching y-axis (if applicable)
#endif

    gfx_window *window;                 // primary display

    gfx_window *activeWindow;           // active draw window
    rectangle   clip;                   // clipping area

    uint16     *_16bpp;                 // 16bpp mapping values (for blitter)
    RGBQUAD    *palette;
    uint32     *paletteDiff;

    font        activeFont;             // active font
    void       *font;                   // font specific information

    void       *info;                   // screen specific information

    void        (*fnCopyRegion)(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y);
  } gfx;

  struct
  {
    uint32      keyMask;                // the keys not translated as events
    void       *info;                   // input/output specific information
  } io;

  struct
  {
    void       *info;                   // audio specific information
  } audio;

} DAL_GlobalsType;

/*************************************************************************
 *
 *    Application Functions
 *
 *************************************************************************/

extern int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
extern BOOL       InitApplication(HINSTANCE hInstance, int nCmdShow);
extern int        EventLoop(void);
extern void       EndApplication(void);

/*************************************************************************
 *
 *    Module Functions
 *
 *************************************************************************/

// none

/*************************************************************************
 *
 *    Global Variables
 *
 *************************************************************************/

// none

#endif

/********************************* EOF ***********************************/
