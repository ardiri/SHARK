/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-prv.h
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#ifndef _DAL_PRV_
#define _DAL_PRV_

#include "../DAL.h"

/*************************************************************************
 *
 *    Include Files
 *
 *************************************************************************/

#ifndef NATIVE

// system includes
#include <stdarg.h>
#include <PalmOS.h>
#include <ExpansionMgr.h>
#include <VFSMgr.h>

// device includes
#include "device/PalmHDD.h"
#include "device/HanderaVGA.h"
#include "device/SonyHR.h"
#include "device/PalmChars.h"
#include "device/HanderaChars.h"
#include "device/SonyChars.h"
#include "device/GamePad.h"              // 3rd party game controller
#include "device/SonySilkLib.h"
#include "device/HanderaSilk.h"
#include "device/PceNativeCall.h"

#define sysFtrNumProcessorVZ      0x00030000
#define sysFtrNumProcessorSuperVZ 0x00040000

#else

#include "device/PceNativeCall.h"
#include "DAL-armlet-palmos.h"

#endif

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 *    Application Constants
 *
 *************************************************************************/

#define palmos1   sysMakeROMVersion(1,0,0,sysROMStageDevelopment,0)
#define palmos2   sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0)
#define palmos3   sysMakeROMVersion(3,0,0,sysROMStageDevelopment,0)
#define palmos3_1 sysMakeROMVersion(3,1,0,sysROMStageDevelopment,0)
#define palmos3_2 sysMakeROMVersion(3,2,0,sysROMStageDevelopment,0)
#define palmos3_3 sysMakeROMVersion(3,3,0,sysROMStageDevelopment,0)
#define palmos3_5 sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0)
#define palmos4   sysMakeROMVersion(4,0,0,sysROMStageDevelopment,0)
#define palmos5   sysMakeROMVersion(5,0,0,sysROMStageDevelopment,0)

#define DAL_CACHE     "DAL_cache"

#define PALETTE_SIZE  256
#define QUEUE_SIZE    128

#define MAX_MEM       128

#define MAX_FONT      128
#define MAX_FONT_CHAR 256

// helper macros
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// device identification (if we need to do anything special)
#define DEVICE_PALM         1
#define DEVICE_PALM_SONY    2
#define DEVICE_PALM_HANDERA 3
#define DEVICE_PALM_HDD     4

/*************************************************************************
 *
 *     GateWay to DAL from 'abstract' application framework
 *
 *************************************************************************/

#define BRIDGE_GLOBALS_INITIALIZE         0
#define BRIDGE_GLOBALS_TERMINATE          1
#define BRIDGE_APPLICATION_INITIALIZE     2
#define BRIDGE_APPLICATION_GET_UNIQUE_ID  3
#define BRIDGE_APPLICATION_GET_NAME       4
#define BRIDGE_APPLICATION_GET_VERSION    5
#define BRIDGE_APPLICATION_GET_PREFS      6
#define BRIDGE_APPLICATION_GET_PREFS_SIZE 7
#define BRIDGE_APPLICATION_HANDLE_EVENT   8
#define BRIDGE_APPLICATION_TERMINATE      9
#define BRIDGE_SYS_GET_EVENT              10
#define BRIDGE_SYS_EVENT_AVAILABLE        11
#define BRIDGE_PEN_HANDLE_EVENT           12
#define BRIDGE_KEY_HANDLE_EVENT           13
#define BRIDGE_GFX_REPAINT                14

/*************************************************************************
 *
 *    Application Structures
 *
 *************************************************************************/

typedef struct
{
  uint32         evtTimeOut;             // event handling time out controller
  uint32         ticksPerFrame;
  uint32         ticksPerFrameE;

  uint32         ticksPerSecond;
  uint32         timerPoint;
#ifdef PALM_TIMING_CHECK
  int16          frameCount;
  uint32         timerReference;
#else
  uint32         timerLastFrameUpdate;
  int32          timerDiff;
#endif

  uint32         device_id;              // which device?

} XXX_GlobalsType;

typedef struct
{
  uint32         evtTimeOut;             // event handling time out controller
  uint32         ticksPerFrame;
  uint32         ticksPerFrameE;
  uint32         ticksPerSecond;
  uint32         device_id;              // which device?

// everything below here is "private" to the DAL implementation 
  void            *emulStateP;
  Call68KFuncType *call68KFuncP;

  struct
  {
    void         *app_globals;           // application globals

    uint32       fps;                    // desired frames/second (x10 factor)
    uint32       seed;                   // random number seed

    uint32       q_index;
    uint32       q_max;
    event       *eventQ;                 // event queue functionality
  } sys;

  struct
  {
    uint32       dbID;    // LocalID
    uint32       card;
    void        *dbRef;   // DmOpenRef   // database references for DAL cache

    uint32       count;
    void        *info;                   // memory 'management' chunk data
  } mem;

  struct
  {
    uint32       width;                  // width of display
    uint32       height;                 // height of display
    uint32       titleHeight;            // height of the title bar
    uint32       depth;                  // depth of display
    uint32       density;                // density [palmos5+]

    gfx_window  *window;                 // primary display

    gfx_window  *activeWindow;           // active draw window
    rectangle    clip;                   // clipping area

    uint32       dsa;                    // can we be 100% sure of DSA (direct screen access)

    uint8       *_4bpp[2];               // grayscale mapping values (for blitter)
    uint16      *_16bpp;                 // 16bpp mapping values (for blitter)
    uint8       *palette;  
    uint32      *paletteDiff;

    uint32       activeFont;             // active font
    void        *font;                   // font specific information

    void        *info;                   // screen specific information

    void        (*fnCopyRegion)(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y);
  } gfx;

  struct
  {
    uint32       keyMask;                // the keys not translated as events
    void        *info;                   // input/output specific information
  } io;
// everything above here is "private" to the DAL implementation 

} DAL_GlobalsType;

/*************************************************************************
 *
 *    Application Functions
 *
 *************************************************************************/

#ifndef NATIVE

extern UInt32  PilotMain(UInt16, MemPtr, UInt16);
extern void    InitApplication(void);
extern void    EventLoop(void);
extern void    EndApplication(void);

#else

#include "DAL-armlet.h"
#include "DAL-armlet-palmos.h"

#endif

/*************************************************************************
 *
 *    Global Variables
 *
 *************************************************************************/

// none

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
