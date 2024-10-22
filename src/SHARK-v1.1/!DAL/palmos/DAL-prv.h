/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-prv.h
 */

#include "../DAL.h"

#ifndef DAL_DEVICE_PRV_H
#define DAL_DEVICE_PRV_H

/*************************************************************************
 *
 * NOTES
 *
 *************************************************************************

MEM_FTR_MEM_STORAGE
storage memory allocated on the feature memory heap instead of as database
resources - the Tungsten|T cannot allocate > 65505 bytes in a resource 
database, hence restricting the available memory to allocate.

 *************************************************************************/

/*************************************************************************
 *
 * Conditional Compilation
 *
 *************************************************************************/

   #define GFX_FULL_SCREEN      // run in as highest resolution possible

// #define MEM_ALLOW_STORAGE    // allow the use of storage memory
// #define MEM_FTR_MEM_STORAGE  // use feature memory for storage heap

   #define INP_SUPPORT_POWER    // support power button on treo/ace
   #define AUD_ONLY_REQUIRED    // allocate audio memory as required

// #define DEV_FIX_PORTRAIT_T3  // force portait mode on T|T3 (faster)
   #define DEV_DYNAMIC_RESIZE   // allow dynamic resizing on palmos (T|T3)

/*************************************************************************
 *
 * Globals Access Optimizations
 *
 *************************************************************************/

#ifdef _M_68K
#undef GLOBALS_ACCESS
#define GLOBALS_ACCESS 

extern DAL_Globals *g;
#endif

#ifdef _M_ARM
#undef GLOBALS_ACCESS
#define GLOBALS_ACCESS register DAL_Globals *g asm ("r8");
#endif

/*************************************************************************
 *
 * Header Defines
 *
 *************************************************************************/

// system includes
#ifdef _M_68K
#include "DAL-68K-platform.h"
#endif
#ifdef _M_ARM
#include "DAL-ARM-platform.h"
#endif

#include "resource.h"

#include    "audio/DAL-audio-prv.h"
#include  "display/DAL-display-prv.h"
#include     "file/DAL-file-prv.h"
#include    "input/DAL-input-prv.h"
#include   "memory/DAL-memory-prv.h"
#include "platform/DAL-platform-prv.h"
#include   "system/DAL-system-prv.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Conditional Compilation
 *
 *************************************************************************/

// none

/*************************************************************************
 *
 * Application Constants/Structures/Typedefs
 *
 *************************************************************************/

#define DEVICE_GENERIC     0
#define DEVICE_HANDERA     1
#define DEVICE_SONY        2
#define DEVICE_PALM_HDD    3

#define DENSITY_SINGLE     0
#define DENSITY_ONEANDHALF 1
#define DENSITY_DOUBLE     2

typedef struct PALM_AudioInfo
{
  snd_stream       id;
  snd_rate         rate;
  snd_format       format;
  snd_type         type;                  // stream information

  uint16           size;
  void            *buffer;                // audio buffers (developer writes to)

  SndStreamRef     streamRef;
  boolean          active; 
  _SndStreamBufferCallback callback; 
  void            *user;                  // streaming audio callbacks

} PALM_AudioInfo;

typedef struct PALM_PlatformInfo
{
  struct
  {
    uint16         type;
    uint16         density;

    struct
    {
      boolean      available;               // is there a silk screen to worry about?
      boolean      maximized;               // may need to restore on exit
    } silk;

    uint16         libRef[4];               // any 3rd party library references (if needed)

  } device;

#ifdef _M_ARM
  struct
  {
    BitmapType    *bmp;
    BitmapTypeV3  *bmpV3;
    WinHandle      win;
  } gfx;
#endif

  struct
  {
    int32          evtTimeOut;              // event handling time out controller
    uint32         ticksPerFrame;
    uint32         ticksPerFrameE;
  
    uint32         ticksPerSecond;
    uint32         timerPoint;
    uint32         timerLastFrameUpdate;
    int32          timerDiff;
  } system;

  struct
  {
    boolean        dpad;
    boolean        tapwave;
#ifdef INP_SUPPORT_POWER
    boolean        power;
#endif
  } input;

  struct
  {
    uint16         volume;
    boolean        streaming;               // is there streaming audio?
  } audio;

  struct
  {
    LocalID        dbID;
    void          *dbRef;                   // database references for DAL cache
  } memory;

  struct
  {
    LocalID        dbID;
    void          *dbRef;                   // database references for platform preferences
  } preferences;

} PALM_PlatformInfo;

#define PALM_HDD_PINS_NEW (boolean)(platform->device.libRef[0])

/*************************************************************************
 *
 * Platform Abstractions
 *
 *************************************************************************/

// these macro's define system specific API calls (memory related) that
// are required for application management - and, > 64k limits of SHARK.

#define SYSTEM_MemPtrNew(x)  MemChunkNew(0, x, memNewChunkFlagNonMovable | memNewChunkFlagAllowLarge)
#define SYSTEM_MemPtrFree    MemPtrFree
#define SYSTEM_MemSet(p,v,c) MemSet(p,v,c)

/*************************************************************************
 *
 * Application Functions
 *
 *************************************************************************/

// device abstraction layer
extern boolean DAL_Initialize(void);
extern void    DAL_Configuration(void);
extern void    DAL_Terminate(void);
extern boolean GlobalsInitialize(void);
extern void    GlobalsRepaint(void);
extern void    GlobalsTerminate(void);

// device application layer
extern UInt32  PilotMain(UInt16 cmd, MemPtr cmdPB, UInt16 launchFlags);
extern Boolean MainFormEventHandler(EventType *event);

extern void    InitApplication(void);
extern void    EventLoop(void);
extern void    EndApplication(void);

extern boolean DAL_ApplicationHandleEvent(event *e);
extern boolean DAL_ApplicationValidateDRM();

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
