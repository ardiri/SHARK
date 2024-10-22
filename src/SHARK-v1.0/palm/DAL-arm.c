/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-arm.c
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"
#include "application_resource.h"
#include "DAL-armlet.h"

/*************************************************************************
 *
 *     Global Variables
 *
 *************************************************************************/

static XXX_GlobalsType  gXXXGlobals;
static DAL_GlobalsType  _DALGlobals;
static DAL_GlobalsType *gDALGlobals;

/*************************************************************************
 *
 *     DAL Functions
 *
 *************************************************************************/

// these are done by armlet - not defined here!

/*************************************************************************
 *
 *     GateWay to DAL from 'abstract' application framework
 *
 *************************************************************************/

// armlet code and GOT table
static uint8   armCodeCount;
static void   *armCode;
static uint32 *gotTable;

static void
DALBridgeInitialize()
{
  MemHandle memHandle;
  uint32    i, cnt, offset;
  uint32    gotSize, armSize, xtra;

  // LOAD: GOT table
  memHandle = DmGetResource('armg', 0); 
  gotSize   = MemHandleSize(memHandle);
  gotTable  = (uint32 *)MemPtrNew(gotSize);
  MemMove(gotTable, MemHandleLock(memHandle), gotSize);
  MemHandleUnlock(memHandle);
  DmReleaseResource(memHandle);

  // LOAD: ARM code - how many/total size?
  armCodeCount = 0;
  armSize      = 0;
  while (DmGetResource('armc', armCodeCount)) armCodeCount++;
  for (i=0; i<armCodeCount; i++)
    armSize += MemHandleSize(DmGetResource('armc', i));

  // LOAD: ARM code - allocate memory buffer (remember GOT)
  xtra = 4 - (armSize & 3);
  FtrPtrNew('_mw_', 0, armSize + gotSize + xtra, &armCode);

  // LOAD: ARM code - concatenate all armcXXXX.bin resources
  offset = 0;
  for (i=0; i<armCodeCount; i++)
  {
    memHandle = DmGetResource('armc', i);
    DmWrite(armCode, offset, MemHandleLock(memHandle), MemHandleSize(memHandle));
    MemHandleUnlock(memHandle);

    offset += MemHandleSize(memHandle);
  }
  offset += xtra;    

  // LOAD: ARM code - fix GOT references, append to armlet memory buffer.
  cnt = gotSize >> 2;
  for (i=0; i<cnt; i++)
    __write_byte32(&gotTable[i], __byte_swap32(gotTable[i]) + (uint32)armCode);
  DmWrite(armCode, offset, gotTable, gotSize);

  // we can free the local memory now, and assign the gotTable ptr correctly :)
  MemPtrFree(gotTable);
  gotTable = (uint32 *)((uint8 *)armCode + offset);
}

static uint32 
DALBridge(UInt16 id, ...)
{
  ParameterType param;
  va_list       arglist;

  // whats the command?
  param.gDALGlobals = gDALGlobals;
  param.gDALGOT     = gotTable;
  param.command     = id;
  param.result      = 0;

  // prepare parameters for armlet callback
  va_start(arglist, id);
  switch (param.command)
  {
    case BRIDGE_APPLICATION_HANDLE_EVENT:
    case BRIDGE_SYS_GET_EVENT:
         param.param[0] = va_arg(arglist, event *);
         break;

    case BRIDGE_PEN_HANDLE_EVENT:
         param.param[0] = va_arg(arglist, Boolean *);
         param.param[1] = va_arg(arglist, Coord *);
         param.param[2] = va_arg(arglist, Coord *);
         param.param[3] = va_arg(arglist, event *);
         break;

    case BRIDGE_KEY_HANDLE_EVENT:
         param.param[0] = va_arg(arglist, UInt16 *);
         param.param[1] = va_arg(arglist, UInt16 *);
         param.param[2] = va_arg(arglist, UInt16 *);
         param.param[3] = va_arg(arglist, event *);
         break;

    default: 
         break;
  }
  va_end(arglist);

  // invoke armlet 
  PceNativeCall(armCode, &param);

  return (param.result);
}

static void
DALBridgeTerminate()
{
  // free allocated memory for the armlet
  FtrPtrFree('_mw_', 0);
}

/*************************************************************************
 *
 *     The Common DAL Functions (application framework = 68k)
 *
 *************************************************************************/

#include "DAL-common.inc"

/********************************* EOF ***********************************/
