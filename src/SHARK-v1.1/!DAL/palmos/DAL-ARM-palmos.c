/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-ARM-palmos.c
 */

#include "DAL-ARM-platform_loader.h"
#include "DAL-prv.h"

/*************************************************************************
 *
 *     PACE callback helper macro's
 * 
 *************************************************************************/

typedef struct EmulStateType 
{
  UInt32 instr;
  UInt32 regD[8];
  UInt32 regA[8];
  UInt32 regPC;
} EmulStateType;

#define PACE_PARAMS_INIT()            \
  GLOBALS_ACCESS;                     \
  UInt8 params[] = {
  
#define PACE_PARAMS_ADD8(param)       \
  (  (UInt8)(param)        & 0xff), 0,

#define PACE_PARAMS_ADD16(param)      \
  (((UInt16)(param) >> 8)  & 0xff),   \
  ( (UInt16)(param)        & 0xff),

#define PACE_PARAMS_ADD32(param)      \
  (((UInt32)(param) >> 24) & 0xff),   \
  (((UInt32)(param) >> 16) & 0xff),   \
  (((UInt32)(param) >> 8)  & 0xff),   \
  ( (UInt32)(param)        & 0xff),

#define PACE_PARAMS_DONE()            \
  };

#define PACE_EXEC(trap, flag)                                                 \
  ((Call68KFuncType *)g->platform_special[1])((void *)g->platform_special[0], \
                            PceNativeTrapNo(trap), &params, sizeof(params) | (flag));

#define PACE_EXEC_SELECTOR(selector) \
  ((EmulStateType *)g->platform_special[0])->regD[2] = selector; 

/*************************************************************************
 *
 *     PACE callback implementation
 * 
 *************************************************************************/

//
// STANDARD HEADERS
//

// 
// system
//

MemPtr
MemChunkNew(UInt16 heapID, UInt32 size, UInt16 attr)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(heapID)
  PACE_PARAMS_ADD32(size)
  PACE_PARAMS_ADD16(attr)
  PACE_PARAMS_DONE()
  return (MemPtr)PACE_EXEC(sysTrapMemChunkNew, kPceNativeWantA0)
}

void
MemChunkFree(MemPtr p)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(p)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapMemChunkFree, 0)
}

MemPtr
MemPtrNew(UInt32 size)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(size)
  PACE_PARAMS_DONE()
  return (MemPtr)PACE_EXEC(sysTrapMemPtrNew, kPceNativeWantA0)
}

MemHandle 
MemPtrRecoverHandle(MemPtr p)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(p)
  PACE_PARAMS_DONE()
  return (MemHandle)PACE_EXEC(sysTrapMemPtrRecoverHandle, kPceNativeWantA0)
}

MemPtr
MemHandleLock(MemHandle h)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(h)
  PACE_PARAMS_DONE()
  return (MemPtr)PACE_EXEC(sysTrapMemHandleLock, kPceNativeWantA0)
}
            
void
MemHandleUnlock(MemHandle h)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(h)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapMemHandleUnlock, 0)
}

void
MemMove(void *dstPtr, void *srcPtr, Int32 numBytes)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(dstPtr)
  PACE_PARAMS_ADD32(srcPtr)
  PACE_PARAMS_ADD32(numBytes)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapMemMove, 0)
}

void
MemSet(void *ptr, UInt32 size, UInt8 val)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(ptr)
  PACE_PARAMS_ADD32(size)
  PACE_PARAMS_ADD8(val)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapMemSet, 0)
}

UInt32
MemHandleSize(MemHandle h)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(h)
  PACE_PARAMS_DONE()
  return (UInt32)PACE_EXEC(sysTrapMemHandleSize, 0)
}

Err
MemSemaphoreReserve(Boolean writeAccess)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD8(writeAccess)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapMemSemaphoreReserve, 0)
}

Err
MemSemaphoreRelease(Boolean writeAccess)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD8(writeAccess)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapMemSemaphoreRelease, 0)
}

Err       
SysTaskDelay(Int32 delay)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(delay)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapSysTaskDelay, 0)
}

Boolean
SysHandleEvent(EventType *e)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(e)
  PACE_PARAMS_DONE()
  return (Boolean)PACE_EXEC(sysTrapSysHandleEvent, 0)
}

Err
SysLibFind(UInt8 *nameP, UInt16 *refNumP)
{
  Err    result;
  UInt16 x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(nameP)
  PACE_PARAMS_ADD32(refNumP)
  PACE_PARAMS_DONE()
  result = (Err)PACE_EXEC(sysTrapSysLibFind, 0)

  // fixup return values (in pointer)
  if (refNumP) { x = __byte_swap16(*refNumP); *refNumP = x; }

  return result;
}

UInt32    
TimGetSeconds()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  return (UInt32)PACE_EXEC(sysTrapTimGetSeconds, 0)
}

UInt32    
TimGetTicks()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  return (UInt32)PACE_EXEC(sysTrapTimGetTicks, 0)
}

void
SysReset()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapSysReset, 0)
}

void
EvtAddEventToQueue(EventType *e)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(e)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapEvtAddEventToQueue, 0)
}

void
EvtGetEvent(EventType *e, Int32 timeout)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(e)
  PACE_PARAMS_ADD32(timeout)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapEvtGetEvent, 0)
}

Err
EvtFlushPenQueue()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapEvtFlushPenQueue, 0)
}

Err
FtrGet(UInt32 creator, UInt16 featureNum, UInt32 *valueP)
{
  Err    result;
  UInt32 x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(creator)
  PACE_PARAMS_ADD16(featureNum)
  PACE_PARAMS_ADD32(valueP)
  PACE_PARAMS_DONE()
  result = (Err)PACE_EXEC(sysTrapFtrGet, 0)

  // fixup return values (in pointer)
  if (valueP) { x = __byte_swap32(*valueP); *valueP = x; }

  return result;
}

Err
FtrSet(UInt32 creator, UInt16 featureNum, UInt32 newValue)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(creator)
  PACE_PARAMS_ADD16(featureNum)
  PACE_PARAMS_ADD32(newValue)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapFtrSet, 0)
}

Err
SysLibLoad(UInt32 libType, UInt32 libCreator, UInt16 *refNumP)
{
  Err    result;
  UInt16 x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(libType)
  PACE_PARAMS_ADD32(libCreator)
  PACE_PARAMS_ADD32(refNumP)
  PACE_PARAMS_DONE()
  result = (Err)PACE_EXEC(sysTrapSysLibLoad, 0)

  // fixup return values (in pointer)
  if (refNumP) { x = __byte_swap16(*refNumP); *refNumP = x; }

  return result;
}

Int32
PrefGetPreference(UInt8 choice)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD8(choice)
  PACE_PARAMS_DONE()
  return (UInt32)PACE_EXEC(sysTrapPrefGetPreference, 0)
}

Int16
PrefGetAppPreferences(UInt32 creator, UInt16 id, void *prefs, UInt16 *prefsSize, Boolean saved)
{
  Int16  result;
  UInt16 x;

  // fixup passing values (in pointer)
  if (prefsSize) { x = __byte_swap16(*prefsSize); *prefsSize = x; }

  {
    PACE_PARAMS_INIT()
    PACE_PARAMS_ADD32(creator)
    PACE_PARAMS_ADD16(id)
    PACE_PARAMS_ADD32(prefs)
    PACE_PARAMS_ADD32(prefsSize)
    PACE_PARAMS_ADD8(saved)
    PACE_PARAMS_DONE()
    result = (Int16)PACE_EXEC(sysTrapPrefGetAppPreferences, 0)
  }

  // fixup return values (in pointer)
  if (prefsSize) { x = __byte_swap16(*prefsSize); *prefsSize = x; }

  return result;
}

void
PrefSetAppPreferences(UInt32 creator, UInt16 id, Int16 version, void *prefs, UInt16 prefsSize, Boolean saved)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(creator)
  PACE_PARAMS_ADD16(id)
  PACE_PARAMS_ADD16(version)
  PACE_PARAMS_ADD32(prefs)
  PACE_PARAMS_ADD16(prefsSize)
  PACE_PARAMS_ADD8(saved)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapPrefSetAppPreferences, 0)
}

UInt16    
SysTicksPerSecond()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  return (UInt16)PACE_EXEC(sysTrapSysTicksPerSecond, 0)
}

Err
FtrPtrNew(UInt32 creator, UInt16 featureNum, UInt32 size, void **newPtrP)
{
  Err     result;
  uint32 *p, x;
  
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(creator)
  PACE_PARAMS_ADD16(featureNum)
  PACE_PARAMS_ADD32(size)
  PACE_PARAMS_ADD32(newPtrP)
  PACE_PARAMS_DONE()
  result = (Err)PACE_EXEC(sysTrapFtrPtrNew, 0)

  // fixup return values (in pointer)
  if (newPtrP) { p = (uint32 *)newPtrP; x = __byte_swap32(*p); *p = x; }

  return result;
}

Err
FtrPtrFree(UInt32 creator, UInt16 featureNum)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(creator)
  PACE_PARAMS_ADD16(featureNum)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapFtrPtrFree, 0)
}

Err
SysLibOpen(UInt16 refNum)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysLibTrapOpen, 0)
}

Err
SysLibClose(UInt16 refNum)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysLibTrapClose, 0)
}

//
// input
//

UInt32
KeyCurrentState()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  return (UInt32)PACE_EXEC(sysTrapKeyCurrentState, 0)
}

void    
EvtGetPen(Int16 *pScreenX, Int16 *pScreenY, Boolean *pPenDown)
{
  Int16   x;
  Boolean y;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(pScreenX)
  PACE_PARAMS_ADD32(pScreenY)
  PACE_PARAMS_ADD32(pPenDown)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapEvtGetPen, 0)

  // fixup return values (in pointer)
  if (pScreenX) { x = __byte_swap16(*pScreenX); *pScreenX = x; }
  if (pScreenY) { x = __byte_swap16(*pScreenY); *pScreenY = x; }
  if (pScreenY) { y = *pPenDown;                *pPenDown = y; }
}

// 
// user interface
//

FormType *
FrmInitForm(UInt16 frmID)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(frmID)
  PACE_PARAMS_DONE()
  return (FormType *)PACE_EXEC(sysTrapFrmInitForm, kPceNativeWantA0)
}

void
FrmDrawForm(FormType *frm)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(frm)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapFrmDrawForm, 0)
}

FormType *
FrmGetActiveForm()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  return (FormType *)PACE_EXEC(sysTrapFrmGetActiveForm, kPceNativeWantA0)
}

void
FrmSetActiveForm(FormType *frm)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(frm)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapFrmSetActiveForm, 0)
}

WinHandle
FrmGetWindowHandle(FormType *frm)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(frm)
  PACE_PARAMS_DONE()
  return (WinHandle)PACE_EXEC(sysTrapFrmGetWindowHandle, kPceNativeWantA0)
}

FormType *
FrmGetFormPtr(UInt16 frmID)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(frmID)
  PACE_PARAMS_DONE()
  return (FormType *)PACE_EXEC(sysTrapFrmGetFormPtr, kPceNativeWantA0)
}

UInt16
FrmCustomAlert(UInt16 altID, UInt8 *str1, UInt8 *str2, UInt8 *str3)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(altID)
  PACE_PARAMS_ADD32(str1)
  PACE_PARAMS_ADD32(str2)
  PACE_PARAMS_ADD32(str3)
  PACE_PARAMS_DONE()
  return (UInt16)PACE_EXEC(sysTrapFrmCustomAlert, 0)
}

void
FrmGotoForm(UInt16 frmID)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(frmID)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapFrmGotoForm, 0)
}

void
FrmUpdateForm(UInt16 frmID, UInt16 updateCode)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(frmID)
  PACE_PARAMS_ADD16(updateCode)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapFrmUpdateForm, 0)
}

void
FrmSetEventHandler(FormType *frm, FormEventHandlerType *frmHandler)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(frm)
  PACE_PARAMS_ADD32(frmHandler)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapFrmSetEventHandler, 0)
}

void
FrmCloseAllForms()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapFrmCloseAllForms, 0)
}

// 
// data manager
//

Err       
DmCreateDatabase(UInt16 cardNo, Int8 *nameP, UInt32 creator, UInt32 type, Boolean resource)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(cardNo)
  PACE_PARAMS_ADD32(nameP)
  PACE_PARAMS_ADD32(creator)
  PACE_PARAMS_ADD32(type)
  PACE_PARAMS_ADD8(resource)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapDmCreateDatabase, 0)
}

void
DmDeleteDatabase(UInt16 cardNo, LocalID dbID)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(cardNo)
  PACE_PARAMS_ADD32(dbID)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapDmDeleteDatabase, 0)
}

LocalID
DmFindDatabase(UInt16 cardNo, Int8 *nameP)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(cardNo)
  PACE_PARAMS_ADD32(nameP)
  PACE_PARAMS_DONE()
  return (LocalID)PACE_EXEC(sysTrapDmFindDatabase, 0)
}

DmOpenRef 
DmOpenDatabase(UInt16 cardNo, LocalID dbID, UInt16 mode)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(cardNo)
  PACE_PARAMS_ADD32(dbID)
  PACE_PARAMS_ADD16(mode)
  PACE_PARAMS_DONE()
  return (DmOpenRef)PACE_EXEC(sysTrapDmOpenDatabase, kPceNativeWantA0)
}

void
DmCloseDatabase(DmOpenRef dbP)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(dbP)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapDmCloseDatabase, 0)
}

MemHandle
DmGetResource(DmResType type, DmResID resID)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(type)
  PACE_PARAMS_ADD16(resID)
  PACE_PARAMS_DONE()
  return (MemHandle)PACE_EXEC(sysTrapDmGetResource, kPceNativeWantA0)
}

void
DmReleaseResource(MemHandle h)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(h)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapDmReleaseResource, 0)
}

MemHandle
DmGetResourceIndex(DmOpenRef dbRef, UInt16 index)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(dbRef)
  PACE_PARAMS_ADD16(index)
  PACE_PARAMS_DONE()
  return (MemHandle)PACE_EXEC(sysTrapDmGetResourceIndex, kPceNativeWantA0)
}

UInt16
DmNumResources(DmOpenRef dbRef)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(dbRef)
  PACE_PARAMS_DONE()
  return (UInt16)PACE_EXEC(sysTrapDmGetResourceIndex, 0)
}

UInt16    
DmFindResource(DmOpenRef dbP, DmResType resType, DmResID resID, MemHandle resH)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(dbP)
  PACE_PARAMS_ADD32(resType)
  PACE_PARAMS_ADD16(resID)
  PACE_PARAMS_ADD32(resH)
  PACE_PARAMS_DONE()
  return (uint16)PACE_EXEC(sysTrapDmFindResource, 0)
}

MemHandle 
DmNewResource(DmOpenRef dbP, DmResType resType, DmResID resID, UInt32 size)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(dbP)
  PACE_PARAMS_ADD32(resType)
  PACE_PARAMS_ADD16(resID)
  PACE_PARAMS_ADD32(size)
  PACE_PARAMS_DONE()
  return (MemHandle)PACE_EXEC(sysTrapDmNewResource, kPceNativeWantA0)
}

Err       
DmRemoveResource(DmOpenRef dbP, UInt16 index)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(dbP)
  PACE_PARAMS_ADD16(index)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapDmRemoveResource, 0)
}

Err
DmWrite(void *recordP, UInt32 offset, void *srcP, UInt32 bytes)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(recordP)
  PACE_PARAMS_ADD32(offset)
  PACE_PARAMS_ADD32(srcP)
  PACE_PARAMS_ADD32(bytes)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapDmWrite, 0)
}

//
// dlk manager
//

Err
DlkGetSyncInfo(UInt32 *succSyncDate, UInt32 *lastSyncDate, void *syncState, UInt8 *name, UInt8 *log, UInt32 *loglen)
{
  Err    result;
  UInt32 x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(succSyncDate)
  PACE_PARAMS_ADD32(lastSyncDate)
  PACE_PARAMS_ADD32(syncState)
  PACE_PARAMS_ADD32(name)
  PACE_PARAMS_ADD32(log)
  PACE_PARAMS_ADD32(loglen)
  PACE_PARAMS_DONE()
  result = (Err)PACE_EXEC(sysTrapDlkGetSyncInfo, 0)

  // fixup return values (in pointer)
  if (succSyncDate) { x = __byte_swap32(*succSyncDate); *succSyncDate = x; }
  if (lastSyncDate) { x = __byte_swap32(*lastSyncDate); *lastSyncDate = x; }
  if (loglen)       { x = __byte_swap32(*loglen);       *loglen       = x; }

  return result;
}

//
// file streaming
//

FileHand      
FileOpen(UInt16 cardNo, UInt8 *nameP, UInt32 type, UInt32 creator, UInt32 openMode, Err *errP)
{
  FileHand result;
  Err      x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(cardNo)
  PACE_PARAMS_ADD32(nameP)
  PACE_PARAMS_ADD32(type)
  PACE_PARAMS_ADD32(creator)
  PACE_PARAMS_ADD32(openMode)
  PACE_PARAMS_ADD32(errP)
  PACE_PARAMS_DONE()
  result = (FileHand)PACE_EXEC(sysTrapFileOpen, kPceNativeWantA0)

  // fixup return values (in pointer)
  if (errP) { x = __byte_swap16(*errP); *errP = x; }

  return result;
}

Err           
FileClose(FileHand stream)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(stream)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapFileClose, 0)
}

Int32         
FileReadLow(FileHand stream, void *baseP, Int32 offset, Boolean dataStoreBased, Int32 objSize, Int32 numObj, Err *errP)
{
  Int32 result;
  Err   x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(stream)
  PACE_PARAMS_ADD32(baseP)
  PACE_PARAMS_ADD32(offset)
  PACE_PARAMS_ADD8(dataStoreBased)
  PACE_PARAMS_ADD32(objSize)
  PACE_PARAMS_ADD32(numObj)
  PACE_PARAMS_ADD32(errP)
  PACE_PARAMS_DONE()
  result = (Int32)PACE_EXEC(sysTrapFileReadLow, 0)

  // fixup return values (in pointer)
  if (errP) { x = __byte_swap16(*errP); *errP = x; }

  return result;
}

Int32         
FileWrite(FileHand stream, void *dataP, Int32 objSize, Int32 numObj, Err *errP)
{
  Int32 result;
  Err   x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(stream)
  PACE_PARAMS_ADD32(dataP)
  PACE_PARAMS_ADD32(objSize)
  PACE_PARAMS_ADD32(numObj)
  PACE_PARAMS_ADD32(errP)
  PACE_PARAMS_DONE()
  result = (Int32)PACE_EXEC(sysTrapFileWrite, 0)

  // fixup return values (in pointer)
  if (errP) { x = __byte_swap16(*errP); *errP = x; }

  return result;
}

Err           
FileSeek(FileHand stream, Int32 offset, UInt8 origin)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(stream)
  PACE_PARAMS_ADD32(offset)
  PACE_PARAMS_ADD8(origin)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapFileSeek, 0)
}

Int32         
FileTell(FileHand stream, Int32 *fileSizeP, Err *errP)
{
  Int32 result;
  Err   x;
  Int32 y;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(stream)
  PACE_PARAMS_ADD32(fileSizeP)
  PACE_PARAMS_ADD32(errP)
  PACE_PARAMS_DONE()
  result = (Int32)PACE_EXEC(sysTrapFileTell, 0)

  // fixup return values (in pointer)
  if (errP)      { x = __byte_swap16(*errP);      *errP      = x; }
  if (fileSizeP) { y = __byte_swap32(*fileSizeP); *fileSizeP = y; }

  return result;
}

Err           
FileControl(UInt8 op, FileHand stream, void *valueP, Int32 *valueLenP)
{
  Err   result;
  Int32 x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD8(op)
  PACE_PARAMS_ADD32(stream)
  PACE_PARAMS_ADD32(valueP)
  PACE_PARAMS_ADD32(valueLenP)
  PACE_PARAMS_DONE()
  result = (Err)PACE_EXEC(sysTrapFileControl, 0)

  // fixup return values (in pointer)
  if (valueLenP) { x = __byte_swap32(*valueLenP); *valueLenP = x; }

  return result;
}

//
// vfs manager
//

Err 
VFSFileOpen(UInt16 volRefNum, UInt8 *pathNameP, UInt16 openMode, FileRef *fileRefP)
{
  Err     result;
  FileRef x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(volRefNum)
  PACE_PARAMS_ADD32(pathNameP)
  PACE_PARAMS_ADD16(openMode)
  PACE_PARAMS_ADD32(fileRefP)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(vfsTrapFileOpen)
  result = PACE_EXEC(sysTrapFileSystemDispatch, 0)

  // fixup return values (in pointer)
  if (fileRefP) { x = __byte_swap32(*fileRefP); *fileRefP = x; }

  return result;
}

Err 
VFSFileClose(FileRef fileRef)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(fileRef)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(vfsTrapFileClose)
  return (Err)PACE_EXEC(sysTrapFileSystemDispatch, 0)
}

Err 
VFSFileRead(FileRef fileRef, UInt32 numBytes, void *bufP, UInt32 *numBytesReadP)
{
  Err    result;
  UInt32 x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(fileRef)
  PACE_PARAMS_ADD32(numBytes)
  PACE_PARAMS_ADD32(bufP)
  PACE_PARAMS_ADD32(numBytesReadP)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(vfsTrapFileRead)
  result = (Err)PACE_EXEC(sysTrapFileSystemDispatch, 0)

  // fixup return values (in pointer)
  if (numBytesReadP) { x = __byte_swap32(*numBytesReadP); *numBytesReadP = x; }

  return result;
}

Err 
VFSFileWrite(FileRef fileRef, UInt32 numBytes, void *dataP, UInt32 *numBytesWrittenP)
{
  Err    result;
  UInt32 x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(fileRef)
  PACE_PARAMS_ADD32(numBytes)
  PACE_PARAMS_ADD32(dataP)
  PACE_PARAMS_ADD32(numBytesWrittenP)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(vfsTrapFileWrite)
  result = (Err)PACE_EXEC(sysTrapFileSystemDispatch, 0)

  // fixup return values (in pointer)
  if (numBytesWrittenP) { x = __byte_swap32(*numBytesWrittenP); *numBytesWrittenP = x; }

  return result;
}

Err 
VFSFileSeek(FileRef fileRef, UInt16 origin, Int32 offset)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(fileRef)
  PACE_PARAMS_ADD16(origin)
  PACE_PARAMS_ADD32(offset)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(vfsTrapFileSeek)
  return (Err)PACE_EXEC(sysTrapFileSystemDispatch, 0)
}

Err 
VFSFileEOF(FileRef fileRef)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(fileRef)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(vfsTrapFileEOF)
  return (Err)PACE_EXEC(sysTrapFileSystemDispatch, 0)
}

Err 
VFSFileTell(FileRef fileRef, UInt32 *filePosP)
{
  Err    result;
  UInt32 x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(fileRef)
  PACE_PARAMS_ADD32(filePosP)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(vfsTrapFileTell)
  result = (Err)PACE_EXEC(sysTrapFileSystemDispatch, 0)

  // fixup return values (in pointer)
  if (filePosP) { x = __byte_swap32(*filePosP); *filePosP = x; }

  return result;
}

Err 
VFSVolumeEnumerate(UInt16 *volRefNumP, UInt32 *volIteratorP)
{
  Err    result;
  UInt32 x;
  UInt16 y;

  // fixup passing values (in pointer)
  if (volIteratorP)  { x = __byte_swap32(*volIteratorP); *volIteratorP = x; }  // IN/OUT

  {
    PACE_PARAMS_INIT()
    PACE_PARAMS_ADD32(volRefNumP)
    PACE_PARAMS_ADD32(volIteratorP)
    PACE_PARAMS_DONE()
    PACE_EXEC_SELECTOR(vfsTrapVolumeEnumerate)
    result = (Err)PACE_EXEC(sysTrapFileSystemDispatch, 0)
  }

  // fixup return values (in pointer)
  if (volRefNumP)   { y = __byte_swap16(*volRefNumP);   *volRefNumP   = y; }
  if (volIteratorP) { x = __byte_swap32(*volIteratorP); *volIteratorP = x; }

  return result;
}

Err
VFSVolumeInfo(UInt16 volRefNum, VolumeInfoType *volInfoP)
{
  Err    result;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(volRefNum)
  PACE_PARAMS_ADD32(volInfoP)
  PACE_PARAMS_DONE()

  // fixup passing values (in pointer)
  volInfoP->attributes    = __byte_swap32(volInfoP->attributes);
  volInfoP->fsType        = __byte_swap32(volInfoP->fsType);
  volInfoP->fsCreator     = __byte_swap32(volInfoP->fsCreator);
  volInfoP->mountClass    = __byte_swap32(volInfoP->mountClass);
  volInfoP->slotLibRefNum = __byte_swap16(volInfoP->slotLibRefNum);
  volInfoP->slotRefNum    = __byte_swap16(volInfoP->slotRefNum);
  volInfoP->mediaType     = __byte_swap32(volInfoP->mediaType);
  volInfoP->reserved      = __byte_swap32(volInfoP->reserved);

  PACE_EXEC_SELECTOR(vfsTrapVolumeInfo)
  result = (Err)PACE_EXEC(sysTrapFileSystemDispatch, 0)

  // fixup passing values (in pointer)
  volInfoP->attributes    = __byte_swap32(volInfoP->attributes);
  volInfoP->fsType        = __byte_swap32(volInfoP->fsType);
  volInfoP->fsCreator     = __byte_swap32(volInfoP->fsCreator);
  volInfoP->mountClass    = __byte_swap32(volInfoP->mountClass);
  volInfoP->slotLibRefNum = __byte_swap16(volInfoP->slotLibRefNum);
  volInfoP->slotRefNum    = __byte_swap16(volInfoP->slotRefNum);
  volInfoP->mediaType     = __byte_swap32(volInfoP->mediaType);
  volInfoP->reserved      = __byte_swap32(volInfoP->reserved);

  return result;
}

Err
VFSVolumeSize(UInt16 volRefNum, UInt32 *volumeUsedP, UInt32 *volumeTotalP)
{
  Err    result;
  UInt32 x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(volRefNum)
  PACE_PARAMS_ADD32(volumeUsedP)
  PACE_PARAMS_ADD32(volumeTotalP)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(vfsTrapVolumeSize)
  result = (Err)PACE_EXEC(sysTrapFileSystemDispatch, 0)

  // fixup return values (in pointer)
  if (volumeUsedP)  { x = __byte_swap16(*volumeUsedP);  *volumeUsedP  = x; }
  if (volumeTotalP) { x = __byte_swap32(*volumeTotalP); *volumeTotalP = x; }

  return result;
}

//
// sound manager
//

Err
SndDoCmd(void *channelP, SndCommandType *cmdP, Boolean noWait)
{
  Err   result;
  UInt8 snd[10];

  // sizeof(SndCommandType): ARM == 12, 68k == 10, need to fix
  snd[0] = cmdP->cmd;
  snd[1] = cmdP->reserved;
  __write_unaligned32(&snd[2], cmdP->param1);
  __write_unaligned16(&snd[6], cmdP->param2);
  __write_unaligned16(&snd[8], cmdP->param3);

  {
    PACE_PARAMS_INIT()
    PACE_PARAMS_ADD32(channelP)
    PACE_PARAMS_ADD32(snd)
    PACE_PARAMS_ADD8(noWait)
    PACE_PARAMS_DONE()
    result = (Err)PACE_EXEC(sysTrapSndDoCmd, 0)
  }

  return result;
}

Err
SndStreamCreate(SndStreamRef *channel, SndStreamMode mode, UInt32 samplerate, SndSampleType type, SndStreamWidth width, SndStreamBufferCallback *func, void *userdata, UInt32 buffsize, Boolean armNative)
{
  Err          result;
  SndStreamRef x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(channel)
  PACE_PARAMS_ADD8(mode)
  PACE_PARAMS_ADD32(samplerate)
  PACE_PARAMS_ADD16(type)
  PACE_PARAMS_ADD8(width)
  PACE_PARAMS_ADD32(func)
  PACE_PARAMS_ADD32(userdata)
  PACE_PARAMS_ADD32(buffsize)
  PACE_PARAMS_ADD8(armNative)
  PACE_PARAMS_DONE()
  result = (Err)PACE_EXEC(sysTrapSndStreamCreate, 0)

  // fixup return values (in pointer)
  if (channel) { x = __byte_swap32(*channel); *channel = x; }

  return result;
}

Err
SndStreamDelete(SndStreamRef channel)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(channel)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapSndStreamDelete, 0)
}

Err
SndStreamStart(SndStreamRef channel)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(channel)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapSndStreamStart, 0)
}

Err
SndStreamStop(SndStreamRef channel)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(channel)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapSndStreamStop, 0)
}

Err
SndStreamSetVolume(SndStreamRef channel, Int32 volume)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(channel)
  PACE_PARAMS_ADD32(volume)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapSndStreamSetVolume, 0)
}

// 
// window manager
//

void
WinDeleteWindow(WinHandle winHandle, Boolean eraseIt)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(winHandle)
  PACE_PARAMS_ADD8(eraseIt)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapWinDeleteWindow, 0)
}

WinHandle
WinSetDrawWindow(WinHandle winHandle)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(winHandle)
  PACE_PARAMS_DONE()
  return (WinHandle)PACE_EXEC(sysTrapWinSetDrawWindow, kPceNativeWantA0)
}

WinHandle
WinGetActiveWindow()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  return (WinHandle)PACE_EXEC(sysTrapWinGetActiveWindow, kPceNativeWantA0)
}

WinHandle
WinGetDisplayWindow()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  return (WinHandle)PACE_EXEC(sysTrapWinGetDisplayWindow, kPceNativeWantA0)
}

void
WinCopyRectangle(WinHandle srcWin, WinHandle dstWin, RectangleType *srcRect, Coord destX, Coord destY, UInt8 mode)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(srcWin)
  PACE_PARAMS_ADD32(dstWin)
  PACE_PARAMS_ADD32(srcRect)
  PACE_PARAMS_ADD16(destX)
  PACE_PARAMS_ADD16(destY)
  PACE_PARAMS_ADD8(mode)
  PACE_PARAMS_DONE()

  // fixup passing values (in pointer)
  srcRect->topLeft.x = __byte_swap16(srcRect->topLeft.x);
  srcRect->topLeft.y = __byte_swap16(srcRect->topLeft.y);
  srcRect->extent.x  = __byte_swap16(srcRect->extent.x);
  srcRect->extent.y  = __byte_swap16(srcRect->extent.y);

  PACE_EXEC(sysTrapWinCopyRectangle, 0)

  // fixup return values (in pointer)
  srcRect->topLeft.x = __byte_swap16(srcRect->topLeft.x);
  srcRect->topLeft.y = __byte_swap16(srcRect->topLeft.y);
  srcRect->extent.x  = __byte_swap16(srcRect->extent.x);
  srcRect->extent.y  = __byte_swap16(srcRect->extent.y);
}

void
WinSetBounds(WinHandle win, RectangleType *rect)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(win)
  PACE_PARAMS_ADD32(rect)
  PACE_PARAMS_DONE()

  // fixup passing values (in pointer)
  rect->topLeft.x = __byte_swap16(rect->topLeft.x);
  rect->topLeft.y = __byte_swap16(rect->topLeft.y);
  rect->extent.x  = __byte_swap16(rect->extent.x);
  rect->extent.y  = __byte_swap16(rect->extent.y);

  PACE_EXEC(sysTrapWinSetBounds, 0)

  // fixup return values (in pointer)
  rect->topLeft.x = __byte_swap16(rect->topLeft.x);
  rect->topLeft.y = __byte_swap16(rect->topLeft.y);
  rect->extent.x  = __byte_swap16(rect->extent.x);
  rect->extent.y  = __byte_swap16(rect->extent.y);
}

Err
WinScreenMode(UInt8 operation, UInt32 *widthP, UInt32 *heightP, UInt32 *depthP, Boolean *enableColorP)
{
  Err     result;
  UInt32  width, height, depth;
  UInt8   color;

  // fixup passing values (in pointers)
  width  = 0;
  height = 0;
  depth  = 0;
  color  = 0;
  if (widthP)       width  = __byte_swap32(*widthP);
  if (heightP)      height = __byte_swap32(*heightP);
  if (depthP)       depth  = __byte_swap32(*depthP);
  if (enableColorP) color  = *enableColorP;

  {
    PACE_PARAMS_INIT()  
    PACE_PARAMS_ADD8(operation)
    PACE_PARAMS_ADD32((width  == 0) ? NULL : &width)
    PACE_PARAMS_ADD32((height == 0) ? NULL : &height)
    PACE_PARAMS_ADD32((depth  == 0) ? NULL : &depth)
    PACE_PARAMS_ADD32((color  == 0) ? NULL : &color)
    PACE_PARAMS_DONE()
    result = (Err)PACE_EXEC(sysTrapWinScreenMode, 0)
  }

  // fixup return values (in pointer) // only when "getting" data
  if ((operation != winScreenModeSetToDefaults) && (operation != winScreenModeSet))
  {
    if (widthP)       *widthP       = __byte_swap32(&width);
    if (heightP)      *heightP      = __byte_swap32(&height);
    if (depthP)       *depthP       = __byte_swap32(&depth);
    if (enableColorP) *enableColorP = color;
  }

  return result;
}

void *
BmpGetBits(BitmapType *bitmapP)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(bitmapP)
  PACE_PARAMS_DONE()
  return (void *)PACE_EXEC(sysTrapBmpGetBits, kPceNativeWantA0)
}

void
WinPalette(UInt8 operation, Int16 startIndex, UInt16 paletteEntries, RGBColorType *tableP)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD8(operation)
  PACE_PARAMS_ADD16(startIndex)
  PACE_PARAMS_ADD16(paletteEntries)
  PACE_PARAMS_ADD32(tableP)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapWinPalette, 0)
}

BitmapType *
WinGetBitmap(WinHandle winHandle)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(winHandle)
  PACE_PARAMS_DONE()
  return (BitmapType *)PACE_EXEC(sysTrapWinGetBitmap, kPceNativeWantA0)
}

BitmapType * 
BmpCreate(Coord width, Coord height, UInt8 depth, ColorTableType *colortableP, UInt16 *error)
{
  BitmapType *result;
  UInt16      x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(width)
  PACE_PARAMS_ADD16(height)
  PACE_PARAMS_ADD8(depth)
  PACE_PARAMS_ADD32(colortableP)
  PACE_PARAMS_ADD32(error)
  PACE_PARAMS_DONE()
  result = (BitmapType *)PACE_EXEC(sysTrapBmpCreate, kPceNativeWantA0)

  // fixup return values (in pointer)
  if (error) { x = __byte_swap16(*error); *error = x; }

  return result;
}

Err
BmpDelete(BitmapType *bitmapP)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(bitmapP)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapBmpDelete, 0)
}

WinHandle
WinCreateBitmapWindow(BitmapType *bitmapP, UInt16 *error)
{
  WinHandle result;
  UInt16    x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(bitmapP)
  PACE_PARAMS_ADD32(error)
  PACE_PARAMS_DONE()
  result = (WinHandle)PACE_EXEC(sysTrapWinCreateBitmapWindow, kPceNativeWantA0)

  // fixup return values (in pointer)
  if (error) { x = __byte_swap16(*error); *error = x; }

  return result;
}

void
WinGetBounds(WinHandle win, RectangleType *rect)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(win)
  PACE_PARAMS_ADD32(rect)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapWinGetBounds, 0)

  // fixup return values (in pointer)
  rect->topLeft.x = __byte_swap16(rect->topLeft.x);
  rect->topLeft.y = __byte_swap16(rect->topLeft.y);
  rect->extent.x  = __byte_swap16(rect->extent.x);
  rect->extent.y  = __byte_swap16(rect->extent.y);
}

//
// HARDWARE EXTENTIONS:  HANDERA
//

Err
VgaSetScreenMode(VgaScreenModeType mode, VgaRotateModeType rotation)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD8(mode)
  PACE_PARAMS_ADD8(rotation)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(trgSetScreenMode)
  return (Err)PACE_EXEC(sysTrapHanderaSelector, 0)
}

Err
VgaFormModify(FormType *frmP, VgaFormModifyType type)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(frmP)
  PACE_PARAMS_ADD8(type)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(trgFrmModify)
  return (Err)PACE_EXEC(sysTrapHanderaSelector, 0)
}

void
SilkMaximizeWindow()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(trgDrawSilkWindow)
  PACE_EXEC(sysTrapHanderaSelector, 0)
}

void
SilkMinimizeWindow()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(trgEraseSilkWindow)
  PACE_EXEC(sysTrapHanderaSelector, 0)
}

Boolean
SilkWindowMaximized()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(trgSilkWindowShown)
  return (Boolean)PACE_EXEC(sysTrapHanderaSelector, 0)
}

//
// HARDWARE EXTENSIONS:  SONY
//

Err
HROpen(UInt16 refNum)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysLibTrapOpen, 0)
}

Err
HRClose(UInt16 refNum)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysLibTrapClose, 0)
}

void
HRWinCopyRectangle(UInt16 refNum, WinHandle srcWin, WinHandle dstWin, RectangleType *srcRect, Coord destX, Coord destY, UInt8 mode)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_ADD32(srcWin)
  PACE_PARAMS_ADD32(dstWin)
  PACE_PARAMS_ADD32(srcRect)
  PACE_PARAMS_ADD16(destX)
  PACE_PARAMS_ADD16(destY)
  PACE_PARAMS_ADD8(mode)
  PACE_PARAMS_DONE()

  // fixup passing values (in pointer)
  srcRect->topLeft.x = __byte_swap16(srcRect->topLeft.x);
  srcRect->topLeft.y = __byte_swap16(srcRect->topLeft.y);
  srcRect->extent.x  = __byte_swap16(srcRect->extent.x);
  srcRect->extent.y  = __byte_swap16(srcRect->extent.y);

  PACE_EXEC(HRTrapWinCopyRectangle, 0)

  // fixup return values (in pointer)
  srcRect->topLeft.x = __byte_swap16(srcRect->topLeft.x);
  srcRect->topLeft.y = __byte_swap16(srcRect->topLeft.y);
  srcRect->extent.x  = __byte_swap16(srcRect->extent.x);
  srcRect->extent.y  = __byte_swap16(srcRect->extent.y);
}

WinHandle
HRWinCreateBitmapWindow(UInt16 refNum, BitmapType *bitmapP, Err *error)
{
  WinHandle result;
  Err       x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_ADD32(bitmapP)
  PACE_PARAMS_ADD32(error)
  PACE_PARAMS_DONE()
  result = (WinHandle)PACE_EXEC(HRTrapWinCreateBitmapWindow, kPceNativeWantA0)

  // fixup return values (in pointer)
  if (error) { x = __byte_swap16(*error); *error = x; }

  return result;
}

void
HRWinGetDisplayExtent(UInt16 refNum, Coord *extentX, Coord *extentY)
{
  Coord x;

  PACE_PARAMS_INIT()  
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_ADD32(extentX)
  PACE_PARAMS_ADD32(extentY)
  PACE_PARAMS_DONE()
  PACE_EXEC(HRTrapWinGetDisplayExtent, 0)

  // fixup return values (in pointer)
  if (extentX) { x = __byte_swap16(*extentX); *extentX = x; }
  if (extentY) { x = __byte_swap16(*extentY); *extentY = x; }
}

Err
HRWinScreenMode(UInt16 refNum, WinScreenModeOperation operation, UInt32 *widthP, UInt32 *heightP, UInt32 *depthP, Boolean *enableColorP)
{
  Err     result;
  UInt32  width, height, depth;
  UInt8   color;

  // fixup passing values (in pointers)
  width  = 0;
  height = 0;
  depth  = 0;
  color  = 0;
  if (widthP)       width  = __byte_swap32(*widthP);
  if (heightP)      height = __byte_swap32(*heightP);
  if (depthP)       depth  = __byte_swap32(*depthP);
  if (enableColorP) color  = *enableColorP;

  {
    PACE_PARAMS_INIT()  
    PACE_PARAMS_ADD16(refNum)
    PACE_PARAMS_ADD8(operation)
    PACE_PARAMS_ADD32((width  == 0) ? NULL : &width)
    PACE_PARAMS_ADD32((height == 0) ? NULL : &height)
    PACE_PARAMS_ADD32((depth  == 0) ? NULL : &depth)
    PACE_PARAMS_ADD32((color  == 0) ? NULL : &color)
    PACE_PARAMS_DONE()
    result = (Err)PACE_EXEC(HRTrapWinScreenMode, 0)
  }

  // fixup return values (in pointer) // only when "getting" data
  if ((operation != winScreenModeSetToDefaults) && (operation != winScreenModeSet))
  {
    if (widthP)       *widthP       = __byte_swap32(&width);
    if (heightP)      *heightP      = __byte_swap32(&height);
    if (depthP)       *depthP       = __byte_swap32(&depth);
    if (enableColorP) *enableColorP = color;
  }

  return result;
}

BitmapType *
HRBmpCreate(UInt16 refNum, Coord width, Coord height, UInt8 depth, ColorTableType *colortableP, Err *error)
{
  BitmapType *result;
  Err         x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_ADD16(width)
  PACE_PARAMS_ADD16(height)
  PACE_PARAMS_ADD8(depth)
  PACE_PARAMS_ADD32(colortableP)
  PACE_PARAMS_ADD32(error)
  PACE_PARAMS_DONE()
  result = (BitmapType *)PACE_EXEC(HRTrapBmpCreate, kPceNativeWantA0)

  // fixup return values (in pointer)
  if (error) { x = __byte_swap16(*error); *error = x; }

  return result;
}

Err
SilkLibOpen(UInt16 refNum)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysLibTrapOpen, 0)
}

Err
SilkLibClose(UInt16 refNum)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysLibTrapClose, 0)
}

Err
SilkLibResizeDispWin(UInt16 refNum, UInt8 win)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_ADD8(win)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(silkLibTrapResizeDispWin, 0)
}

Err
SilkLibEnableResize(UInt16 refNum)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(silkLibTrapEnableResize, 0)
}

Err
SilkLibDisableResize(UInt16 refNum)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(silkLibTrapDisableResize, 0)
}

Err
VskSetState(UInt16 refNum, UInt16 stateType, UInt16 state)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_ADD16(stateType)
  PACE_PARAMS_ADD16(state)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(vskLibTrapSetState, 0)
}

//
// HARDWARE EXTENSIONS:  PALM HDD
//

BitmapTypeV3 *
BmpCreateBitmapV3(BitmapType *bitmapP, UInt16 density, void *bitsP, ColorTableType *colorTableP)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(bitmapP)
  PACE_PARAMS_ADD16(density)
  PACE_PARAMS_ADD32(bitsP)
  PACE_PARAMS_ADD32(colorTableP)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(HDSelectorBmpCreateBitmapV3)
  return (BitmapTypeV3 *)PACE_EXEC(sysTrapSysHighDensitySelector, kPceNativeWantA0)
}

UInt16        
WinSetCoordinateSystem(uint16 coordSys)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(coordSys)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(HDSelectorWinSetCoordinateSystem)
  return (UInt16)PACE_EXEC(sysTrapSysHighDensitySelector, 0)
}

Err
WinScreenGetAttribute(WinScreenAttrType selector, UInt32* attrP)
{
  Err    result;
  UInt32 x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD8(selector)
  PACE_PARAMS_ADD32(attrP)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(HDSelectorWinScreenGetAttribute)
  result = (Err)PACE_EXEC(sysTrapSysHighDensitySelector, 0)

  // fixup return values (in pointer)
  if (attrP) { x = __byte_swap32(*attrP); *attrP = x; }

  return result;
}

void
EvtGetPenNative(WinHandle winH, Int16* pScreenX, Int16* pScreenY, Boolean* pPenDown)
{
  Int16   x;
//Boolean y;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(winH)
  PACE_PARAMS_ADD32(pScreenX)
  PACE_PARAMS_ADD32(pScreenY)
  PACE_PARAMS_ADD32(pPenDown)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(HDSelectorEvtGetPenNative)
  PACE_EXEC(sysTrapSysHighDensitySelector, 0)

  // fixup return values (in pointer)
  if (pScreenX) { x = __byte_swap16(*pScreenX); *pScreenX = x; }
  if (pScreenY) { x = __byte_swap16(*pScreenY); *pScreenY = x; }
//if (pPenDown) { y = *pPenDown;                *pPenDown = y; }
}

Err
PINSetInputAreaState(UInt16 state)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(state)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinPINSetInputAreaState)
  return (Err)PACE_EXEC(sysTrapSysPinSelector, 0)
}

UInt16
PINGetInputAreaState()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinPINGetInputAreaState)
  return (uint16)PACE_EXEC(sysTrapSysPinSelector, 0)
}

Err
PINSetInputTriggerState(UInt16 state)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(state)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinPINSetInputTriggerState)
  return (Err)PACE_EXEC(sysTrapSysPinSelector, 0)
}

Err
FrmSetDIAPolicyAttr(FormType *formP, UInt16 diaPolicy)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(formP)
  PACE_PARAMS_ADD16(diaPolicy)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinFrmSetDIAPolicyAttr)
  return (Err)PACE_EXEC(sysTrapSysPinSelector, 0)
}

Err
StatHide()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinStatHide)
  return (Err)PACE_EXEC(sysTrapSysPinSelector, 0)
}

Err
StatShow()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinStatShow)
  return (Err)PACE_EXEC(sysTrapSysPinSelector, 0)
}

Err
StatGetAttribute(UInt16 selector, UInt32 *dataP)
{
  Err    result;
  UInt32 x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(selector)
  PACE_PARAMS_ADD32(dataP)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinStatGetAttribute)
  result = (Err)PACE_EXEC(sysTrapSysPinSelector, 0)

  // fixup return values (in pointer)
  if (dataP) { x = __byte_swap32(*dataP); *dataP = x; }

  return result;
}

Err
SysSetOrientation(UInt16 orientation)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(orientation)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinSysSetOrientation)
  return (Err)PACE_EXEC(sysTrapSysPinSelector, 0)
}

Err
SysSetOrientationTriggerState(UInt16 state)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(state)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinSysSetOrientationTriggerState)
  return (Err)PACE_EXEC(sysTrapSysPinSelector, 0)
}

/********************************* EOF ***********************************/
