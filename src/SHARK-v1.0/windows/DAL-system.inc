/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)windows/DAL-system.inc
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
static boolean  _SysEventAvailable();
static void     _SysGetEvent(event *);
static void     _SysFlushEventQueue();

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

void
_SysInitialize()
{
  gDALGlobals.sys.eventQ = (event *)malloc(QUEUE_SIZE * sizeof(event));
  memset(gDALGlobals.sys.eventQ, 0, QUEUE_SIZE * sizeof(event));

  _SysSetFPS(0);
  _SysFlushEventQueue();
}

void
_SysTerminate()
{
  free(gDALGlobals.sys.eventQ);
  gDALGlobals.sys.eventQ = NULL;
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

void
_SysSetFPS(uint32 fps)
{
  gDALGlobals.sys.fps = fps;

  if (fps == 0)
  {
    gDALGlobals.evtTimeOut     = EVT_WAIT_FOREVER;           // no animations
    KillTimer(gDALGlobals.LCDWnd, gDALGlobals.timerID);
  }
  else
  {
    gDALGlobals.evtTimeOut     = 1;
    gDALGlobals.ticksPerFrame  = (DEFAULT_TICKS_PER_SEC * 10) / fps;
    gDALGlobals.timerID        = SetTimer(gDALGlobals.LCDWnd, DEFAULT_TIMER_ID, gDALGlobals.ticksPerFrame, NULL);
  }
}

uint32
_SysGetFPS()
{
  return gDALGlobals.sys.fps;
}

void
_SysEnqueueEvent(event *e)
{
  // save event in queue at 'max' index
  memmove(&gDALGlobals.sys.eventQ[gDALGlobals.sys.q_max], e, sizeof(event));

  // move index along, or wrap around if needed
  gDALGlobals.sys.q_max++;
  if (gDALGlobals.sys.q_max >= QUEUE_SIZE) gDALGlobals.sys.q_max = 0;
}

void _SysEnqueueQuitEvent()
{
  event e;

  memset(&e, 0, sizeof(event));
  e.eType = _appStopEvent;
  _SysEnqueueEvent(&e);
}

uint32
_SysGetTime()
{
  return (uint32)time(NULL);
/*
  // TODO/FIX LATER:
  // this only returns the number of seconds since the system was last rebooted.
  // not the number of seconds since jan 1, 1970 (as it really should)

#ifdef _WIN32_WCE
  return (uint32)(GetTickCount() / 1000);
#else
#ifdef USE_HIGH_FREQ_TIMER
  LARGE_INTEGER curTick;
  LONGLONG tick;

  QueryPerformanceCounter(&curTick);
  tick = ((LONGLONG)(curTick.HighPart) << 32) + (LONGLONG)(curTick.LowPart);
  return (uint32)(tick / 1000);
#else
  return (uint32)(GetTickCount() / 1000);
#endif
#endif
*/
}

void
_SysDelay(uint32 millisec)
{
  Sleep(millisec);
}

uint16
_SysRandom(uint32 seed)
{
  if (seed != 0) gDALGlobals.sys.seed = seed;
  gDALGlobals.sys.seed = (0x015A4E35L * gDALGlobals.sys.seed) + 1;
  return (uint16)(gDALGlobals.sys.seed >> 16);
}

void
_SysSetGlobals(void *gbls)
{
  gDALGlobals.app_globals = gbls;
}

void *
_SysGetGlobals()
{
  return (void *)gDALGlobals.app_globals;
}

void
_SysDebugMessage(uint8 *msg)
{
#ifdef _WIN32_WCE
  uint16 *buf = (uint16 *)_MemPtrNew((uint16)((_StrLen(msg)+1) << 1), false);
  int     i;

  i = 0;
  while (i < _StrLen(msg))
    buf[i] = msg[i++];
  buf[i] = 0;

  MessageBox(gDALGlobals.LCDWnd, (const uint16 *)buf, TEXT("Information"), MB_OK);
  _MemPtrFree(buf);
#else
  MessageBox(gDALGlobals.LCDWnd, (const uint8 *)msg, TEXT("Information"), MB_OK);
#endif
}

//
// private functions
//

static boolean
_SysEventAvailable()
{
  return (boolean)(gDALGlobals.sys.q_index != gDALGlobals.sys.q_max);
}

static void
_SysGetEvent(event *e)
{
  // get event from queue at current index
  memmove(e, &gDALGlobals.sys.eventQ[gDALGlobals.sys.q_index], sizeof(event));

  // move index along, or wrap around if needed
  gDALGlobals.sys.q_index++;
  if (gDALGlobals.sys.q_index >= QUEUE_SIZE) gDALGlobals.sys.q_index = 0;
}

static void
_SysFlushEventQueue()
{
  gDALGlobals.sys.q_index = 0;
  gDALGlobals.sys.q_max   = 0;
}

/********************************* EOF ***********************************/
