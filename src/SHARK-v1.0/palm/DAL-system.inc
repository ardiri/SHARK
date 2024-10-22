/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-system.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 * Platform Implementation Structures
 *
 *************************************************************************/

// local functions
static void     _SysInitialize()      __DAL__;
static boolean  _SysEventAvailable()  __DAL__;
static void     _SysGetEvent(event *) __DAL__;
static void     _SysFlushEventQueue() __DAL__;
static void     _SysTerminate()       __DAL__;

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

void
_SysInitialize()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif

  gDALGlobals->sys.eventQ = (event *)MemPtrNew(sizeof(event) * QUEUE_SIZE);
  MemSet(gDALGlobals->sys.eventQ, sizeof(event) * QUEUE_SIZE, 0);

  _SysSetFPS(0);
  _SysFlushEventQueue();
}

void
_SysTerminate()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif

  MemPtrFree(gDALGlobals->sys.eventQ);
  gDALGlobals->sys.eventQ = NULL;
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

void
_SysSetFPS(uint32 fps)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif

  gDALGlobals->sys.fps = fps;

  if (fps == 0)
    gDALGlobals->evtTimeOut     = evtWaitForever;            // no animations
  else
  {
    gDALGlobals->evtTimeOut     = 1;
    gDALGlobals->ticksPerSecond = SysTicksPerSecond();
    gDALGlobals->ticksPerFrame  = (gDALGlobals->ticksPerSecond * 10) / fps;
    gDALGlobals->ticksPerFrameE = (gDALGlobals->ticksPerSecond % (fps / 10));
    if (gDALGlobals->ticksPerFrameE != 0)
      gDALGlobals->ticksPerFrameE = ((fps / 10) - gDALGlobals->ticksPerFrameE);
  }
}

uint32
_SysGetFPS()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif

  return gDALGlobals->sys.fps;
}

void
_SysEnqueueEvent(event *e)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif

  // save event in queue at 'max' index
  MemMove(&gDALGlobals->sys.eventQ[gDALGlobals->sys.q_max], e, sizeof(event));

  // move index along, or wrap around if needed
  gDALGlobals->sys.q_max++;
  if (gDALGlobals->sys.q_max >= QUEUE_SIZE) gDALGlobals->sys.q_max = 0;
}

void _SysEnqueueQuitEvent()
{
  event e;

  MemSet(&e, sizeof(event), 0);
  e.eType = _appStopEvent;
  _SysEnqueueEvent(&e);
}

uint32
_SysGetTime()
{
  return (uint32)(TimGetSeconds() - 0x7C25B080); // 0x7C25B080 = 1970 - 1904
}

void
_SysDelay(uint32 millisec)
{
  SysTaskDelay((millisec * SysTicksPerSecond()) / 1000);
}

uint16
_SysRandom(uint32 seed)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif

  if (seed != 0) gDALGlobals->sys.seed = seed;
  gDALGlobals->sys.seed = (0x015A4E35L * gDALGlobals->sys.seed) + 1;
  return (uint16)(gDALGlobals->sys.seed >> 16);
}

void
_SysSetGlobals(void *gbls)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif

  gDALGlobals->sys.app_globals = gbls;
}

void *
_SysGetGlobals()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif

  return (void *)gDALGlobals->sys.app_globals;
}

void
_SysDebugMessage(uint8 *msg)
{
  SysFatalAlert(msg);
}

//
// private functions
//

static boolean
_SysEventAvailable()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif

  return (boolean)(gDALGlobals->sys.q_index != gDALGlobals->sys.q_max);
}

static void
_SysGetEvent(event *e)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif

  // get event from queue at current index
  MemMove(e, &gDALGlobals->sys.eventQ[gDALGlobals->sys.q_index], sizeof(event));

  // move index along, or wrap around if needed
  gDALGlobals->sys.q_index++;
  if (gDALGlobals->sys.q_index >= QUEUE_SIZE) gDALGlobals->sys.q_index = 0;
}

static void
_SysFlushEventQueue()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif

  gDALGlobals->sys.q_index = 0;
  gDALGlobals->sys.q_max   = 0;
}

/********************************* EOF ***********************************/

