/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)series60/DAL-system.inc
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
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  gDALGlobals->sys.eventQ = (event *)malloc(QUEUE_SIZE * sizeof(event));
  memset(gDALGlobals->sys.eventQ, 0, QUEUE_SIZE * sizeof(event));

  _SysSetFPS(0);
  _SysFlushEventQueue();
}

void
_SysTerminate()
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  free(gDALGlobals->sys.eventQ);
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
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  gDALGlobals->sys.fps = fps;

  if (fps == 0)
    gDALGlobals->evtTimeOut     = (TInt32)-1; // no animations
  else
    gDALGlobals->evtTimeOut     = (TInt32)(MICRO_SECONDx10 / fps);

  // if the thread is running, terminate, and, start new if needed
  gDALGlobals->appThread->CancelTimerRequest();
  if (gDALGlobals->evtTimeOut != -1)
    gDALGlobals->appThread->After(TTimeIntervalMicroSeconds32(gDALGlobals->evtTimeOut));
}

uint32
_SysGetFPS()
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  return gDALGlobals->sys.fps;
}

void
_SysEnqueueEvent(event *e)
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  // save event in queue at 'max' index
  memmove(&gDALGlobals->sys.eventQ[gDALGlobals->sys.q_max], e, sizeof(event));

  // move index along, or wrap around if needed
  gDALGlobals->sys.q_max++;
  if (gDALGlobals->sys.q_max >= QUEUE_SIZE) gDALGlobals->sys.q_max = 0;
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
  TTimeIntervalSeconds interval;
  TTime                currentTime, baseTime;

  _LIT(KTTime01Jan1970,"19700101:000000:000000");
  baseTime.Set(KTTime01Jan1970);

  // number of seconds since 01 Jan, 1970?
  currentTime.HomeTime();
  currentTime.SecondsFrom(baseTime, interval);

  return (uint32)interval.Int();
}

void
_SysDelay(uint32 millisec)
{
  TRequestStatus timerStatus;
  RTimer         timer;

  timer.CreateLocal();
  timer.After(timerStatus,TTimeIntervalMicroSeconds32(millisec * 1000));
  User::WaitForRequest(timerStatus);
  timer.Close();
}

uint16
_SysRandom(uint32 seed)
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  if (seed != 0) gDALGlobals->sys.seed = seed;
  gDALGlobals->sys.seed = (0x015A4E35L * gDALGlobals->sys.seed) + 1;
  return (uint16)(gDALGlobals->sys.seed >> 16);
}

void
_SysSetGlobals(void *gbls)
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  gDALGlobals->app_globals = gbls;
}

void *
_SysGetGlobals()
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  return (void *)gDALGlobals->app_globals;
}

void 
_SysDebugMessage(uint8 *msg)
{
#ifdef __WINS__
  TPtrC8 buf = new TPtrC8(msg);
  User::InfoPrint(buf);
  delete buf;
#endif
}

//
// private functions
//

static boolean
_SysEventAvailable()
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  return (boolean)(gDALGlobals->sys.q_index != gDALGlobals->sys.q_max);
}

static void
_SysGetEvent(event *e)
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  // get event from queue at current index
  memmove(e, &gDALGlobals->sys.eventQ[gDALGlobals->sys.q_index], sizeof(event));

  // move index along, or wrap around if needed
  gDALGlobals->sys.q_index++;
  if (gDALGlobals->sys.q_index >= QUEUE_SIZE) gDALGlobals->sys.q_index = 0;
}

static void
_SysFlushEventQueue()
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  gDALGlobals->sys.q_index = 0;
  gDALGlobals->sys.q_max   = 0;
}

/********************************* EOF ***********************************/