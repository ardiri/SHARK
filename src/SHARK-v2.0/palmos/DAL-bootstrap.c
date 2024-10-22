/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-bootstrap.c
 */

// 
// NOTE: code executed as 68000 code, which launches DAL-bootstrap-ARM.c 
// 

// -----------------------------------------------------------------------
// system includes 
// -----------------------------------------------------------------------

#include <PalmOS.h>

// palmos 5: PACE
typedef UInt32 Call68KFuncType(const void *, UInt32, const void *, UInt32);
typedef UInt32 NativeFuncType(const void *, void *, Call68KFuncType *);

#define sysTrapPceNativeCall                0xA45A
UInt32 PceNativeCall(NativeFuncType *nativeFuncP, void *userDataP)
  SYS_TRAP(sysTrapPceNativeCall);

#define __byte_swap32(n) \
        (((((UInt32) n) << 24) & 0xFF000000) |  \
         ((((UInt32) n) <<  8) & 0x00FF0000) |  \
         ((((UInt32) n) >>  8) & 0x0000FF00) |  \
         ((((UInt32) n) >> 24) & 0x000000FF) )

#define __write_byte32(addr, value) \
        (((UInt8 *)(addr))[3] = (UInt8)((UInt32)(value) >> 24), \
         ((UInt8 *)(addr))[2] = (UInt8)((UInt32)(value) >> 16), \
         ((UInt8 *)(addr))[1] = (UInt8)((UInt32)(value) >>  8), \
         ((UInt8 *)(addr))[0] = (UInt8)((UInt32)(value)) )

// -----------------------------------------------------------------------

#include "resources/resource.h"

static UInt32 armlet;
static UInt32 GOT;

#define DEBUG(msg) FrmCustomAlert(DebugAlert, msg, NULL, NULL)

/*************************************************************************
 *
 * Platform Depending Application Framework
 *
 *************************************************************************/

Boolean
PACE_LoadArmResource() 
{
  MemHandle memHandle;
  UInt32    i, offset, count, csize, gsize, xtra;
  UInt32   *table;

  gsize = 0;
  csize = 0;
  table = NULL;

  // LOAD: GOT size
  memHandle = DmGetResource('armg', 0);
  if (memHandle != NULL)
  {
    gsize = MemHandleSize(memHandle);
    table = (UInt32 *)MemPtrNew(gsize);
    if (table == NULL) goto MEMORY_FAILURE;
    MemMove(table, MemHandleLock(memHandle), gsize);
    MemHandleUnlock(memHandle);
    DmReleaseResource(memHandle);
  }

  // LOAD: ARM code - how many/total size?
  count = 0;
  while (DmGetResource('armc', count)) count++;
  for (i=0; i<count; i++)
  {
    csize += MemHandleSize(DmGetResource('armc', i));
  }
  xtra = (4 - (csize & 3)) & 3;  // ensure GOT is dword aligned

  // lets allocate memory for the new armlet
  FtrPtrFree('_mw_', 0);
   FtrPtrNew('_mw_', 0, (UInt32)(csize + xtra + gsize), (void **)&armlet);
  if ((void *)armlet == NULL) goto MEMORY_FAILURE;

  // LOAD: ARM code - concatenate all armcXXXX.bin resources
  offset = 0;
  for (i=0; i<count; i++)
  {
    memHandle = DmGetResource('armc', i);
    DmWrite((void *)armlet, offset, 
            MemHandleLock(memHandle), MemHandleSize(memHandle));
    MemHandleUnlock(memHandle);

    offset += MemHandleSize(memHandle);
  }
  offset += xtra;    

  // we need this value for later
  GOT = armlet + offset;

  // LOAD: ARM code - fix GOT references, append to armlet memory buffer.
  if (table != NULL)
  {
    count = gsize >> 2;
    for (i=0; i<count; i++)
    {
      __write_byte32(&table[i], __byte_swap32(table[i]) + armlet);
    }
    DmWrite((void *)armlet, offset, table, gsize);

    // release all our resources
    MemPtrFree(table);
  }

  // everything is OK!
  return true;

MEMORY_FAILURE:

  // we couldn't allocate the memory, sorry
  DEBUG("Insufficient Memory Available");
  return false;
}

void
PACE_UnloadArmResource() 
{
  // clean up on exit
  FtrPtrFree('_mw_', 0);
}

/*************************************************************************
 *
 * Main Entry Point
 *
 *************************************************************************/

UInt32
PilotMain(UInt16 cmd, MemPtr cmdPB, UInt16 launchFlags)
{
  UInt32  romVersion, displayDepths, result;
  UInt16  card;
  LocalID dbID;

  // what type of launch was this?
  switch (cmd)
  {
    case sysAppLaunchCmdDisplayAlarm:
         SysCurAppDatabase(&card, &dbID);
         SysUIAppSwitch(card, dbID, sysAppLaunchCmdNormalLaunch, NULL);
         break;

    case sysAppLaunchCmdNormalLaunch:

         // get the rom version from the device
         FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

         // determine some screen stuff?
         displayDepths = 0x00000001;
         if (romVersion >= sysMakeROMVersion(3,0,0,sysROMStageDevelopment,0))
           WinScreenMode(winScreenModeGetSupportedDepths, 
                         NULL, NULL, &displayDepths, NULL);

         // LIMITATION: we must have color and OS 5.x only
         if (((displayDepths & 0x80) == 0) ||
            (romVersion >= sysMakeROMVersion(6,0,0,sysROMStageDevelopment,0)) ||
            (romVersion <  sysMakeROMVersion(5,0,0,sysROMStageDevelopment,0)))
         {
           DEBUG("device incompatible\n- Palm OS\256 5.x\n- 16bit color display");
           if (romVersion < sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0))
             AppLaunchWithCommand(sysFileCDefaultApp,
                                  sysAppLaunchCmdNormalLaunch, NULL);
         }

         // we are running on a Palm OS 5.x based device with 16bit color! yay
         else
         { 
           result = errNone;
           if (PACE_LoadArmResource())
             result = PceNativeCall((void *)armlet, (void *)GOT);
           PACE_UnloadArmResource();
           
           switch (result)
           {
             case 0x10000000: DEBUG("device incompatible\n- check requirements"); break;
             case 0x20000000: DEBUG("insufficient memory\n- check requirements"); break;
             default: break;
           }
         }
         break;

    default:
         break;
  }

  return errNone;
}

/********************************* EOF ***********************************/
