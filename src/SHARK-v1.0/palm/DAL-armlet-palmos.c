/*************************************************************************
 *
 * Copyright (c) 2002-2003 Mobile Wizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-armlet-palmos.c
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

/*************************************************************************
 *
 *     Include Files
 * 
 *************************************************************************/

#include "DAL-prv.h"

/*************************************************************************
 *
 *     PACE callback helper macro's
 * 
 *************************************************************************/

typedef struct EmulStateType 
{
  uint32 instr;
  uint32 regD[8];
  uint32 regA[8];
  uint32 regPC;
} EmulStateType;

#define PACE_PARAMS_INIT()           \
  register DAL_GlobalsType *gDALGlobals asm ("r8"); \
  uint8 params[] = {
  
#define PACE_PARAMS_ADD8(param)       \
  (  (uint8)(param)        & 0xff), 0,

#define PACE_PARAMS_ADD16(param)     \
  (((uint16)(param) >> 8)  & 0xff),   \
  ( (uint16)(param)        & 0xff),

#define PACE_PARAMS_ADD32(param)     \
  (((uint32)(param) >> 24) & 0xff),   \
  (((uint32)(param) >> 16) & 0xff),   \
  (((uint32)(param) >> 8)  & 0xff),   \
  ( (uint32)(param)        & 0xff),

#define PACE_PARAMS_DONE()           \
  };

#define PACE_EXEC(trap, flag)                                     \
  gDALGlobals->call68KFuncP((void *)gDALGlobals->emulStateP,      \
                            PceNativeTrapNo(trap), &params, sizeof(params) | (flag));

#define PACE_EXEC_SELECTOR(selector)                               \
  ((EmulStateType *)gDALGlobals->emulStateP)->regD[2] = selector; 

MemPtr
MemChunkNew(uint16 heapID, uint32 size, uint16 attr)
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
MemPtrNew(uint32 size)
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
MemMove(void *dstPtr, void *srcPtr, int32 numBytes)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(dstPtr)
  PACE_PARAMS_ADD32(srcPtr)
  PACE_PARAMS_ADD32(numBytes)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapMemMove, 0)
}

void
MemSet(void *ptr, uint32 size, uint8 val)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(ptr)
  PACE_PARAMS_ADD32(size)
  PACE_PARAMS_ADD8(val)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapMemSet, 0)
}

uint32
MemHandleSize(MemHandle h)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(h)
  PACE_PARAMS_DONE()
  return (uint32)PACE_EXEC(sysTrapMemHandleSize, 0)
}

Err       
DmCreateDatabase(uint16 cardNo, int8 *nameP, uint32 creator, uint32 type, Boolean resource)
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

LocalID
DmFindDatabase(uint16 cardNo, int8 *nameP)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(cardNo)
  PACE_PARAMS_ADD32(nameP)
  PACE_PARAMS_DONE()
  return (LocalID)PACE_EXEC(sysTrapDmFindDatabase, 0)
}

void
DmDeleteDatabase(uint16 cardNo, LocalID dbID)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(cardNo)
  PACE_PARAMS_ADD32(dbID)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapDmDeleteDatabase, 0)
}

DmOpenRef 
DmOpenDatabase(uint16 cardNo, LocalID dbID, uint16 mode)
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

uint16    
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
DmNewResource(DmOpenRef dbP, DmResType resType, DmResID resID, uint32 size)
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
DmRemoveResource(DmOpenRef dbP, uint16 index)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(dbP)
  PACE_PARAMS_ADD16(index)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapDmRemoveResource, 0)
}

Err
DmWrite(void *recordP, uint32 offset, void *srcP, uint32 bytes)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(recordP)
  PACE_PARAMS_ADD32(offset)
  PACE_PARAMS_ADD32(srcP)
  PACE_PARAMS_ADD32(bytes)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapDmWrite, 0)
}

void      
ErrDisplayFileLineMsg(int8 *file, uint16 line, int8 *msg)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(file)
  PACE_PARAMS_ADD16(line)
  PACE_PARAMS_ADD32(msg)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapErrDisplayFileLineMsg, 0)
}

Err       
SysTaskDelay(int32 delay)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(delay)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapSysTaskDelay, 0)
}

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
WinGetDisplayWindow()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  return (WinHandle)PACE_EXEC(sysTrapWinGetDisplayWindow, kPceNativeWantA0)
}

uint32    
TimGetSeconds()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  return (uint32)PACE_EXEC(sysTrapTimGetSeconds, 0)
}

void    
EvtGetPen(int16 *pScreenX, int16 *pScreenY, Boolean *pPenDown)
{
  int16   x;
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

uint16      
SysFatalAlert(int8 *msg)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(msg)
  PACE_PARAMS_DONE()
  return (uint16)PACE_EXEC(sysTrapSysFatalAlert, 0)
}

uint8 *
StrCopy(uint8 *strDst, uint8 *strSrc)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(strDst)
  PACE_PARAMS_ADD32(strSrc)
  PACE_PARAMS_DONE()
  return (uint8 *)PACE_EXEC(sysTrapStrCopy, kPceNativeWantA0)
}

uint8 *
StrCat(uint8 *str, uint8 *strCat)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(str)
  PACE_PARAMS_ADD32(strCat)
  PACE_PARAMS_DONE()
  return (uint8 *)PACE_EXEC(sysTrapStrCat, kPceNativeWantA0)
}

uint16
StrLen(uint8 *str)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(str)
  PACE_PARAMS_DONE()
  return (uint16)PACE_EXEC(sysTrapStrLen, 0)
}

uint8 *
StrIToA(uint8 *str, int32 value)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(str)
  PACE_PARAMS_ADD32(value)
  PACE_PARAMS_DONE()
  return (uint8 *)PACE_EXEC(sysTrapStrIToA, kPceNativeWantA0)
}

uint16
StrAToI(uint8 *str)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(str)
  PACE_PARAMS_DONE()
  return (uint16)PACE_EXEC(sysTrapStrAToI, 0)
}

void
WinCopyRectangle(WinHandle srcWin, WinHandle dstWin, RectangleType *srcRect, Coord destX, Coord destY, uint8 mode)
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

Err
SndDoCmd(void *channelP, SndCommandType *cmdP, Boolean noWait)
{
  Err   result;
  uint8 snd[10];

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
FtrGet(uint32 creator, uint16 featureNum, uint32 *valueP)
{
  Err    result;
  uint32 x;

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

uint32
KeyCurrentState()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  return (uint32)PACE_EXEC(sysTrapKeyCurrentState, 0)
}

uint32  
KeySetMask(uint32 keyMask)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(keyMask)
  PACE_PARAMS_DONE()
  return (uint32)PACE_EXEC(sysTrapKeySetMask, 0)
}

uint16    
SysTicksPerSecond()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  return (uint16)PACE_EXEC(sysTrapSysTicksPerSecond, 0)
}

FileHand      
FileOpen(uint16 cardNo, uint8 *nameP, uint32 type, uint32 creator, uint32 openMode, Err *errP)
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

int32         
FileReadLow(FileHand stream, void *baseP, int32 offset, Boolean dataStoreBased, int32 objSize, int32 numObj, Err *errP)
{
  int32 result;
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
  result = (int32)PACE_EXEC(sysTrapFileReadLow, 0)

  // fixup return values (in pointer)
  if (errP) { x = __byte_swap16(*errP); *errP = x; }

  return result;
}

int32         
FileWrite(FileHand stream, void *dataP, int32 objSize, int32 numObj, Err *errP)
{
  int32 result;
  Err   x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(stream)
  PACE_PARAMS_ADD32(dataP)
  PACE_PARAMS_ADD32(objSize)
  PACE_PARAMS_ADD32(numObj)
  PACE_PARAMS_ADD32(errP)
  PACE_PARAMS_DONE()
  result = (int32)PACE_EXEC(sysTrapFileWrite, 0)

  // fixup return values (in pointer)
  if (errP) { x = __byte_swap16(*errP); *errP = x; }

  return result;
}

Err           
FileSeek(FileHand stream, int32 offset, uint8 origin)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(stream)
  PACE_PARAMS_ADD32(offset)
  PACE_PARAMS_ADD8(origin)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapFileSeek, 0)
}

int32         
FileTell(FileHand stream, int32 *fileSizeP, Err *errP)
{
  int32 result;
  Err   x;
  int32 y;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(stream)
  PACE_PARAMS_ADD32(fileSizeP)
  PACE_PARAMS_ADD32(errP)
  PACE_PARAMS_DONE()
  result = (int32)PACE_EXEC(sysTrapFileTell, 0)

  // fixup return values (in pointer)
  if (errP)      { x = __byte_swap16(*errP);      *errP      = x; }
  if (fileSizeP) { y = __byte_swap32(*fileSizeP); *fileSizeP = y; }

  return result;
}

Err           
FileControl(uint8 op, FileHand stream, void *valueP, int32 *valueLenP)
{
  Err   result;
  int32 x;

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

Err
WinScreenMode(uint8 operation, uint32 *widthP, uint32 *heightP, uint32 *depthP, Boolean *enableColorP)
{
  Err     result;
  uint32  width, height, depth;
  uint8   color;

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
WinPalette(uint8 operation, int16 startIndex, uint16 paletteEntries, RGBColorType *tableP)
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
BmpCreate(Coord width, Coord height, uint8 depth, ColorTableType *colortableP, uint16 *error)
{
  BitmapType *result;
  uint16      x;

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
WinCreateBitmapWindow(BitmapType *bitmapP, uint16 *error)
{
  WinHandle result;
  uint16    x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(bitmapP)
  PACE_PARAMS_ADD32(error)
  PACE_PARAMS_DONE()
  result = (WinHandle)PACE_EXEC(sysTrapWinCreateBitmapWindow, kPceNativeWantA0)

  // fixup return values (in pointer)
  if (error) { x = __byte_swap16(*error); *error = x; }

  return result;
}

BitmapTypeV3 *
BmpCreateBitmapV3(BitmapType *bitmapP, uint16 density, void *bitsP, const ColorTableType *colortableP)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(bitmapP)
  PACE_PARAMS_ADD16(density)
  PACE_PARAMS_ADD32(bitsP)
  PACE_PARAMS_ADD32(colortableP)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(HDSelectorBmpCreateBitmapV3)
  return (BitmapTypeV3 *)PACE_EXEC(sysTrapSysHighDensitySelector, kPceNativeWantA0)
}

uint16        
WinSetCoordinateSystem(uint16 coordSys)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(coordSys)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(HDSelectorWinSetCoordinateSystem)
  return (uint16)PACE_EXEC(sysTrapSysHighDensitySelector, 0)
}

Err
WinScreenGetAttribute(uint8 selector, uint32 *attrP)
{
  Err    result;
  uint32 x;

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
EvtGetPenNative(WinHandle winH, int16 *pScreenX, int16 *pScreenY, Boolean *pPenDown)
{
  int16   x;
  Boolean y;

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
  if (pScreenY) { y = *pPenDown;                *pPenDown = y; }
}

Err 
VFSFileOpen(uint16 volRefNum, uint8 *pathNameP, uint16 openMode, FileRef *fileRefP)
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
VFSFileRead(FileRef fileRef, uint32 numBytes, void *bufP, uint32 *numBytesReadP)
{
  Err    result;
  uint32 x;

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
VFSFileWrite(FileRef fileRef, uint32 numBytes, void *dataP, uint32 *numBytesWrittenP)
{
  Err    result;
  uint32 x;

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
VFSFileSeek(FileRef fileRef, uint16 origin, int32 offset)
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
VFSFileTell(FileRef fileRef, uint32 *filePosP)
{
  Err    result;
  uint32 x;

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
VFSVolumeEnumerate(uint16 *volRefNumP, uint32 *volIteratorP)
{
  Err    result;
  uint32 x;
  uint16 y;

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

uint16
PINGetInputAreaState()
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinPINGetInputAreaState)
  return (uint16)PACE_EXEC(sysTrapSysPinSelector, 0)
}

void
PINSetInputAreaState(uint16 state)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(state)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinPINSetInputAreaState)
  PACE_EXEC(sysTrapSysPinSelector, 0)
}

void
PINSetInputTriggerState(uint16 state)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(state)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinPINSetInputTriggerState)
  PACE_EXEC(sysTrapSysPinSelector, 0)
}

void          
StatGetAttribute(uint16 selector, uint32* dataP)
{
  uint32 x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(selector)
  PACE_PARAMS_ADD32(dataP)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinStatGetAttribute)
  PACE_EXEC(sysTrapSysPinSelector, 0)

  // fixup return values (in pointer)
  if (dataP) { x = __byte_swap32(*dataP); *dataP = x; }
}

Err
SysLibLoad(uint32 libType, uint32 libCreator, uint16 *refNumP)
{
  Err    result;
  uint16 x;

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

Err
SysLibFind(uint8 *nameP, uint16 *refNumP)
{
  Err    result;
  uint16 x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(nameP)
  PACE_PARAMS_ADD32(refNumP)
  PACE_PARAMS_DONE()
  result = (Err)PACE_EXEC(sysTrapSysLibFind, 0)

  // fixup return values (in pointer)
  if (refNumP) { x = __byte_swap16(*refNumP); *refNumP = x; }

  return result;
}

Err
SysLibOpen(uint16 refNum)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysLibTrapOpen, 0)
}

Err
SysLibClose(uint16 refNum)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysLibTrapClose, 0)
}

Err
HRWinScreenMode(uint16 refNum, uint8 operation, uint32 *widthP, uint32 *heightP, uint32 *depthP, Boolean *enableColorP)
{
  Err     result;
  uint32  width, height, depth;
  uint8   color;

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
    result = (Err)PACE_EXEC(sysTrapHRWinScreenMode, 0)
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

void
HRWinGetDisplayExtent(uint16 refNum, Coord *extentX, Coord *extentY)
{
  Coord x;

  PACE_PARAMS_INIT()  
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_ADD32(extentX)
  PACE_PARAMS_ADD32(extentY)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapHRWinGetDisplayExtent, 0)

  // fixup return values (in pointer)
  if (extentX) { x = __byte_swap16(*extentX); *extentX = x; }
  if (extentY) { x = __byte_swap16(*extentY); *extentY = x; }
}

BitmapType *
HRBmpCreate(uint16 refNum, Coord width, Coord height, uint8 depth, ColorTableType *colortableP, uint16 *error)
{
  BitmapType *result;
  uint16      x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_ADD16(width)
  PACE_PARAMS_ADD16(height)
  PACE_PARAMS_ADD8(depth)
  PACE_PARAMS_ADD32(colortableP)
  PACE_PARAMS_ADD32(error)
  PACE_PARAMS_DONE()
  result = (BitmapType *)PACE_EXEC(sysTrapHRBmpCreate, kPceNativeWantA0)

  // fixup return values (in pointer)
  if (error) { x = __byte_swap16(*error); *error = x; }

  return result;
}

WinHandle
HRWinCreateBitmapWindow(uint16 refNum, BitmapType *bitmapP, uint16 *error)
{
  WinHandle result;
  uint16    x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_ADD32(bitmapP)
  PACE_PARAMS_ADD32(error)
  PACE_PARAMS_DONE()
  result = (WinHandle)PACE_EXEC(sysTrapHRWinCreateBitmapWindow, kPceNativeWantA0)

  // fixup return values (in pointer)
  if (error) { x = __byte_swap16(*error); *error = x; }

  return result;
}

void
HRWinCopyRectangle(uint16 refNum, WinHandle srcWin, WinHandle dstWin, RectangleType *srcRect, Coord destX, Coord destY, uint8 mode)
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

  PACE_EXEC(sysTrapHRWinCopyRectangle, 0)

  // fixup return values (in pointer)
  srcRect->topLeft.x = __byte_swap16(srcRect->topLeft.x);
  srcRect->topLeft.y = __byte_swap16(srcRect->topLeft.y);
  srcRect->extent.x  = __byte_swap16(srcRect->extent.x);
  srcRect->extent.y  = __byte_swap16(srcRect->extent.y);
}

Err
SilkLibResizeDispWin(uint16 refNum, uint8 win)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_ADD8(win)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapSilkLibResizeDispWin, 0)
}

Err
SilkLibEnableResize(uint16 refNum)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapSilkLibEnableResize, 0)
}

Err
SilkLibDisableResize(uint16 refNum)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(refNum)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapSilkLibDisableResize, 0)
}

/********************************* E O F *********************************/
