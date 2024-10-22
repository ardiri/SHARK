/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-ARM-platform_loader.c
 */

#include "DAL-ARM-platform_loader.h"

static UInt32 armlet;
static UInt32 GOT;

/*************************************************************************
 *
 * MMC Insert/Removal + DIA + sleep Handling
 *
 *************************************************************************/

Err notifyCardMounted(SysNotifyParamType *notifyParamsP)  { notifyParamsP->handled |= vfsHandledUIAppSwitch; return 0; }
Err notifyCardRemoval(SysNotifyParamType *notifyParamsP)  {                                                  return 0; }
Err notifyDIAResized(SysNotifyParamType *notifyParamsP)   { EventType e = {}; e.eType = winDisplayChangedEvent; EvtAddEventToQueue(&e); return 0; }

/*************************************************************************
 *
 * Platform Depending Application Framework
 *
 *************************************************************************/

Boolean
DAL_LoadArmResource() 
{
  MemHandle memHandle;
  UInt32    i, offset, count, csize, gsize, xtra;
  UInt32   *table;
  Boolean   result;

  // LOAD: GOT size
  memHandle = DmGetResource('armg', 0);
  gsize = MemHandleSize(memHandle);
  table = (UInt32 *)MemPtrNew(gsize);
  if (table == NULL) goto MEMORY_FAILURE;
  MemMove(table, MemHandleLock(memHandle), gsize);
  MemHandleUnlock(memHandle);
  DmReleaseResource(memHandle);

  // LOAD: ARM code - how many/total size?
  count = 0;
  while (DmGetResource('armc', count)) count++;
  csize = 0;
  for (i=0; i<count; i++)
    csize += MemHandleSize(DmGetResource('armc', i));
  xtra = (4 - (csize & 3)) & 3;  // ensure GOT is dword aligned

  // lets allocate memory for the new armlet
  FtrPtrFree('_mw_', 0);
  FtrPtrNew ('_mw_', 0, (UInt32)(csize + xtra + gsize), (void **)&armlet);
  if (armlet == NULL) goto MEMORY_FAILURE;

  // LOAD: ARM code - concatenate all armcXXXX.bin resources
  offset = 0;
  for (i=0; i<count; i++)
  {
    memHandle = DmGetResource('armc', i);
    DmWrite((void *)armlet, offset, MemHandleLock(memHandle), MemHandleSize(memHandle));
    MemHandleUnlock(memHandle);

    offset += MemHandleSize(memHandle);
  }
  offset += xtra;    

  // we need this value for later
  GOT = armlet + offset;

  // LOAD: ARM code - fix GOT references, append to armlet memory buffer.
  count = gsize >> 2;
  for (i=0; i<count; i++)
    __write_byte32(&table[i], __byte_swap32(table[i]) + armlet);
  DmWrite((void *)armlet, offset, table, gsize);

  // release all our resources
  MemPtrFree(table);

  // everything is OK!
  return true;

MEMORY_FAILURE:

  FrmCustomAlert(DebugAlert, "INSUFFICIENT MEMORY AVAILABLE", NULL, NULL);
  return false;
}

void
DAL_UnloadArmResource() 
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
  UInt32  romVersion;
  UInt32  displayDepths;
  UInt32  version;
  LocalID dbID;
  UInt16  card;

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
           WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &displayDepths, NULL);

         // LIMITATION #1: this code is not optimized for new PalmOS Cobalt devices 
         if ((romVersion >= sysMakeROMVersion(6,0,0,sysROMStageDevelopment,0)) && !(displayDepths & 0x88))
         {
           FrmCustomAlert(OSIncompatibleAlert, "This application has not been optimized for your Palm OS\256 handheld, install the appropriate version for your device.", NULL, NULL);
         }
         else

         // LIMITATION #2: eek - prior to OS 5.0? this code is optimized for ARM
         if (romVersion < sysMakeROMVersion(5,0,0,sysROMStageDevelopment,0))
         {
           FrmCustomAlert(OSIncompatibleAlert, "Palm OS\256 version 5.0 or greater is required to run this application.", NULL, NULL);
           if (romVersion < sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0))
             AppLaunchWithCommand(sysFileCDefaultApp,
                                  sysAppLaunchCmdNormalLaunch, NULL);
         }
         else

         // we passed all the requirements - lets do it!
         { 
           // notification registration for external media card removal
           if (FtrGet(sysFtrCreator, sysFtrNumNotifyMgrVersion, &version) == errNone)
           {
             SysCurAppDatabase(&card, &dbID);

             // mmc insert/removal notification
             SysNotifyRegister(card, dbID, sysNotifyVolumeMountedEvent,
                               notifyCardMounted, sysNotifyNormalPriority, NULL);
             SysNotifyRegister(card, dbID, sysNotifyCardRemovedEvent,
                               notifyCardRemoval, sysNotifyNormalPriority, NULL);

             // DIA support
             SysNotifyRegister(card, dbID, sysNotifyDisplayResizedEvent, 
                               notifyDIAResized, sysNotifyNormalPriority, NULL);
           }

           // lets switch to 100% ARM mode :)
           if (DAL_LoadArmResource())
             PceNativeCall((void *)armlet, (void *)GOT);
           DAL_UnloadArmResource();

           // notification registration for external media card removal
           if (FtrGet(sysFtrCreator, sysFtrNumNotifyMgrVersion, &version) == errNone)
           {
             SysCurAppDatabase(&card, &dbID);

             // mmc insert/removal notification
             SysNotifyUnregister(card, dbID, sysNotifyVolumeMountedEvent,  sysNotifyNormalPriority);
             SysNotifyUnregister(card, dbID, sysNotifyCardRemovedEvent,    sysNotifyNormalPriority);
             SysNotifyUnregister(card, dbID, sysNotifyDisplayResizedEvent, sysNotifyNormalPriority);
           }
         }
         break;

    default:
         break;
  }

  return errNone;
}

/********************************* EOF ***********************************/
