/*************************************************************************
 *
 * Copyright (c) 2002-2003 Mobile Wizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-armlet-palmos.h
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#ifndef _DAL_ARMLET_PALMOS_
#define _DAL_ARMLET_PALMOS_

/*************************************************************************
 *
 *     Include Files
 * 
 *************************************************************************/

#include "DAL-prv.h"

/*************************************************************************
 *
 *     PalmOS data types/constants
 * 
 *************************************************************************/

#define evtWaitForever              -1
#define errNone                     0
#define dmModeWrite                 0x0002
#define sndMaxAmp                   64

#define fileModeReadOnly            (0x80000000UL)
#define fileOriginBeginning         1
#define fileOriginCurrent           2
#define fileOriginEnd               3
#define fileOpGetEOFStatus          2
#define fileErrorClass              0x1600
#define fileErrEOF                  (fileErrorClass | 16)

#define vfsIteratorStart            0L
#define vfsIteratorStop             0xffffffffL
#define vfsModeRead                 (0x0002U)
#define vfsOriginBeginning          0
#define vfsOriginCurrent            1
#define vfsOriginEnd                2

#define sysFtrCreator               0x70737973  // 'psys' == 0x70737973
#define sysFileTLibrary             0x6c696272  // 'libr' == 0x6c696272
#define sysFileCVFSMgr              0x7666736d  // 'vfsm' == 0x7666736d
#define sysFtrNumROMVersion         1
#define sysFtrNumWinVersion         24
#define sysFtrNumUIHardwareFlags    27
#define sysFtrNumUIHardwareHas5Way  0x00000001
#define vfsFtrIDVersion             0

#define sonySysFileCHRLib		        0x536c4872  // 'SlHr' == 0x536c4872
#define sonySysFileCSilkLib         0x53655369  // 'SeSi' == 0x53655369
#define sonySysLibNameHR		        "Sony HR Library"
#define sonySysLibNameSilk          "Sony Silk Library"

#define silkResizeNormal            0
#define silkResizeToStatus          1

#define ftrErrorClass               0x0C00
#define ftrErrNoSuchFeature         (ftrErrorClass | 2)

#define sysROMStageDevelopment      0

#define sysMakeROMVersion(major, minor, fix, stage, buildNum) \
        ( \
          (((uint32)( (uint8)(major) & 0x0FF)) << 24) | \
          (((uint32)( (uint8)(minor) & 0x00F)) << 20) | \
          (((uint32)( (uint8)(fix)   & 0x00F)) << 16) | \
          (((uint32)( (uint8)(stage) & 0x00F)) << 12) | \
          (((uint32)((uint16)(buildNum) & 0x0FFF))) \
         )

#define kDensityLow                72
#define kDensityOneAndAHalf        108
#define kDensityDouble             144
#define winScreenWidth             0
#define winScreenHeight            1
#define kCoordinatesNative         0
#define kCoordinatesStandard       72

#define winScreenModeSetToDefaults 2
#define winScreenModeSet           3 
#define winPaletteSet              1
#define winPaletteSetToDefault     2
#define winPaint                   0

#define sndCmdFrqOn                3

#define keyBitPower                0x00000001
#define keyBitPageUp               0x00000002
#define keyBitPageDown             0x00000004
#define keyBitHard1                0x00000008
#define keyBitHard2                0x00000010
#define keyBitHard3                0x00000020
#define keyBitHard4                0x00000040
#define keyBitRockerUp             0x00010000
#define keyBitRockerDown           0x00020000
#define keyBitRockerLeft           0x00040000
#define keyBitRockerRight          0x00080000
#define keyBitRockerSelect         0x00100000
#define keyBitNavLeft              0x01000000
#define keyBitNavRight             0x02000000
#define keyBitNavSelect            0x04000000
#define keyBitTriggerA             0x04000000
#define keyBitTriggerB             0x08000000
#define keyBitActionA              0x10000000
#define keyBitActionB              0x20000000
#define keyBitActionC              0x40000000
#define keyBitActionD              0x80000000
#define keyBitsAll                 0xFFFFFFFF

#define vchrPageUp                 0x000B
#define vchrPageDown               0x000C
#define vchrHardKeyMin             0x0200
#define vchrHardKeyMax             0x02FF
#define vchrHard1                  0x0204
#define vchrHard2                  0x0205
#define vchrHard3                  0x0206
#define vchrHard4                  0x0207
#define vchrPalmMin                0x0500
#define vchrNavChange              (vchrPalmMin + 3)

#define vchrJogUp                  0x1700
#define vchrJogDown                0x1701
#define vchrJogPress               0x1705

#define vchrThumbWheelUp           0x012E  // optional thumb-wheel up
#define vchrThumbWheelDown         0x012F  // optional thumb-wheel down
#define vchrThumbWheelPush         0x0130  // optional thumb-wheel press/center

#define vchrRockerUp               0x0132  // 5-way rocker up
#define vchrRockerDown             0x0133  // 5-way rocker down
#define vchrRockerLeft             0x0134  // 5-way rocker left
#define vchrRockerRight            0x0135  // 5-way rocker right
#define vchrRockerCenter           0x0136  // 5-way rocker center/press

// tapwave device
#undef vcharTapWaveMin
#undef vcharTapWaveMax
#define vchrTapWaveMin             0x1B00
#define vchrTapWaveMax             0x1BFF

#define vchrTriggerLeft            (vchrTapWaveMin + 4)
#define vchrTriggerRight           (vchrTapWaveMin + 5)
#define vchrActionUp               (vchrTapWaveMin + 6)
#define vchrActionRight            (vchrTapWaveMin + 7)
#define vchrActionDown             (vchrTapWaveMin + 8)
#define vchrActionLeft             (vchrTapWaveMin + 9)

#define vchrTriggerA               vchrTriggerLeft
#define vchrTriggerB               vchrTriggerRight
#define vchrActionA                vchrActionUp
#define vchrActionB                vchrActionRight
#define vchrActionC                vchrActionDown
#define vchrActionD                vchrActionLeft

#define navBitUp                   0x0001
#define navBitDown                 0x0002
#define navBitLeft                 0x0004
#define navBitRight                0x0008
#define navBitSelect               0x0010
#define navBitsAll                 0x001F
#define navChangeUp                0x0100
#define navChangeDown              0x0200
#define navChangeLeft              0x0400
#define navChangeRight             0x0800
#define navChangeSelect            0x1000
#define navChangeBitsAll           0x1F00

#define navFtrCreator              0x66776179  // 'fway' == 0x66776179
#define navFtrVersion              0

#define twFtrCreator               0x54707776  // 'Tpwv' == 0x54707776
#define twFtrAPIVersion            0x0000

#define hardKeyMin                 vchrHardKeyMin
#define hardKeyMax                 vchrHardKeyMax

#define commandKeyMask             0x0008
#define appEvtHookKeyMask          0x0200
#define libEvtHookKeyMask          0x0400
#define virtualKeyMask             (appEvtHookKeyMask | libEvtHookKeyMask | commandKeyMask)

#define pinCreator                 0x70696e73  // 'pins' == 0x70696e73
#define pinFtrAPIVersion           1

#define statLibCreator		         0x73426172  // 'sBar' == 0x73426172
#define statFtrNumVersion	         0

#define pinInputAreaOpen           0
#define pinInputAreaClosed         1
#define pinInputTriggerEnabled     0
#define pinInputTriggerDisabled    1

#define statAttrBarVisible         0
#define statAttrDimension          1

#define sysFtrNumProcessorID       2
#define sysFtrNumOEMCompanyID      20
#define sysFtrNumOEMDeviceID       21

#define sysFtrNumProcessor328      0x00010000
#define sysFtrNumProcessorEZ       0x00020000
#define sysFtrNumProcessorVZ       0x00030000
#define sysFtrNumProcessorSuperVZ  0x00040000

// palmos specific
typedef int8    Char;
typedef uint8   Boolean;
typedef void   *MemPtr;
typedef void   *MemHandle;
typedef void   *DmOpenRef;
typedef uint32  DmResType;
typedef uint32  LocalID;
typedef uint16  DmResID;
typedef int16   Err;
typedef int16   Coord;
typedef void   *FileHand;
typedef uint32  FileRef;

typedef struct  EventType      EventType;
typedef struct  WindowType     WindowType;
typedef struct  BitmapType     BitmapType;
typedef struct  BitmapTypeV3   BitmapTypeV3;
typedef struct  ColorTableType ColorTableType;
typedef struct  RGBColorType   RGBColorType;
typedef struct  PointType      { Coord     x;       Coord     y;      } PointType;
typedef struct  RectangleType  { PointType topLeft; PointType extent; } RectangleType;
typedef struct  SndCommandType { uint8 cmd; uint8 reserved; uint32 param1; uint16 param2; uint16 param3; } SndCommandType;

typedef WindowType *WinHandle;

/*************************************************************************
 *
 *     PalmOS traps/selectors
 * 
 *************************************************************************/

#define sysTrapMemChunkNew                  0xA011
#define sysTrapMemChunkFree                 0xA012
#define sysTrapMemPtrNew                    0xA013
#define sysTrapMemPtrRecoverHandle          0xA014
#define sysTrapMemHandleLock                0xA021
#define sysTrapMemHandleUnlock              0xA022
#define sysTrapMemMove                      0xA026
#define sysTrapMemSet                       0xA027
#define sysTrapMemHandleSize                0xA02D
#define sysTrapMemSemaphoreReserve          0xA03C
#define sysTrapMemSemaphoreRelease          0xA03D
#define sysTrapDmCreateDatabase             0xA041
#define sysTrapDmDeleteDatabase             0xA042
#define sysTrapDmFindDatabase               0xA045
#define sysTrapDmOpenDatabase               0xA049
#define sysTrapDmCloseDatabase              0xA04A
#define sysTrapDmGetResource                0xA05F
#define sysTrapDmReleaseResource            0xA061
#define sysTrapDmFindResource               0xA065
#define sysTrapDmNewResource                0xA06C
#define sysTrapDmRemoveResource             0xA06D
#define sysTrapDmWrite                      0xA076
#define sysTrapErrDisplayFileLineMsg        0xA084
#define sysTrapSysTaskDelay                 0xA0A0
#define sysTrapSysFatalAlert                0xA0AD
#define sysTrapStrCopy                      0xA0C5
#define sysTrapStrCat                       0xA0C6
#define sysTrapStrLen                       0xA0C7
#define sysTrapStrIToA                      0xA0C9
#define sysTrapStrAToI                      0xA0CE
#define sysTrapTimGetSeconds                0xA0F5
#define sysTrapEvtGetPen                    0xA11E
#define sysTrapWinDeleteWindow              0xA1F8
#define sysTrapWinSetDrawWindow             0xA1FD
#define sysTrapWinGetDisplayWindow          0xA200
#define sysTrapWinCopyRectangle             0xA209
#define sysTrapSndDoCmd                     0xA233
#define sysTrapFtrGet                       0xA27B
#define sysTrapKeyCurrentState              0xA2A2
#define sysTrapKeySetMask                   0xA2CF
#define sysTrapSysTicksPerSecond            0xA2E9
#define sysTrapFileOpen                     0xA332
#define sysTrapFileClose                    0xA333
#define sysTrapFileReadLow                  0xA335
#define sysTrapFileWrite                    0xA336
#define sysTrapFileSeek                     0xA337
#define sysTrapFileTell                     0xA338
#define sysTrapFileControl                  0xA33A
#define sysTrapWinScreenMode                0xA33E
#define sysTrapBmpGetBits                   0xA376
#define sysTrapWinPalette                   0xA39D
#define sysTrapWinGetBitmap                 0xA3A2
#define sysTrapBmpCreate                    0xA3DD
#define sysTrapBmpDelete                    0xA3DE
#define sysTrapWinCreateBitmapWindow        0xA3E4

// trap/selector based
#define sysTrapSysHighDensitySelector       0xA3EC
#define HDSelectorBmpCreateBitmapV3         7
#define HDSelectorWinSetCoordinateSystem    8
#define HDSelectorWinScreenGetAttribute     14
#define HDSelectorEvtGetPenNative           17

#define sysTrapFileSystemDispatch           0xA348
#define vfsTrapFileOpen                     3
#define vfsTrapFileClose                    4
#define vfsTrapFileRead                     6
#define vfsTrapFileWrite                    7
#define vfsTrapFileSeek                     10
#define vfsTrapFileEOF                      11
#define vfsTrapFileTell                     12
#define vfsTrapVolumeEnumerate              27

#define sysTrapSysPinSelector               0xA470
#define pinPINSetInputAreaState             0
#define pinPINGetInputAreaState             1
#define pinPINSetInputTriggerState          2
#define pinStatGetAttribute                 18

// system libraries
#define sysTrapSysLibLoad                   0xA2AC
#define sysTrapSysLibFind                   0xA0BA
#define sysLibTrapOpen                      0xA801
#define sysLibTrapClose                     0xA802
#define sysLibTrapCustom                    0xA805

#define sysTrapHRWinScreenMode              (sysLibTrapCustom+48)
#define sysTrapHRWinGetDisplayExtent        (sysLibTrapCustom+26)
#define sysTrapHRBmpCreate                  (sysLibTrapCustom+55)
#define sysTrapHRWinCreateBitmapWindow      (sysLibTrapCustom+3)
#define sysTrapHRWinCopyRectangle           (sysLibTrapCustom+2)

#define sysTrapSilkLibResizeDispWin         (sysLibTrapCustom)
#define sysTrapSilkLibEnableResize          (sysLibTrapCustom+1)
#define sysTrapSilkLibDisableResize         (sysLibTrapCustom+2)

/*************************************************************************
 *
 *     PalmOS function prototypes
 * 
 *************************************************************************/

#define MemPtrFree MemChunkFree
#define ErrFatalDisplay(msg) \
        ErrDisplayFileLineMsg(msg, (uint16)__LINE__, msg)
#define FileRead(stream, bufP, objSize, numObj, errP) \
        FileReadLow((stream), (bufP), 0, false, (objSize), (numObj), (errP))
#define FileEOF(__stream__)                           \
        (FileControl(fileOpGetEOFStatus, (__stream__), NULL, NULL) == fileErrEOF)

MemPtr        MemChunkNew(uint16, uint32, uint16);
void          MemChunkFree(MemPtr);
MemPtr        MemPtrNew(uint32);
MemHandle     MemPtrRecoverHandle(MemPtr);
MemPtr        MemHandleLock(MemHandle);
void          MemHandleUnlock(MemHandle);
void          MemMove(void *, void *, int32);
void          MemSet(void *, uint32, uint8);
uint32        MemHandleSize(MemHandle);
Err           MemSemaphoreReserve(Boolean);
Err           MemSemaphoreRelease(Boolean);
Err           SysTaskDelay(int32);
Err           DmCreateDatabase(uint16, int8 *, uint32, uint32, Boolean);
LocalID       DmFindDatabase(uint16, int8 *);
void          DmDeleteDatabase(uint16, LocalID);
DmOpenRef     DmOpenDatabase(uint16, LocalID, uint16);
void          DmCloseDatabase(DmOpenRef);
MemHandle     DmGetResource(DmResType, DmResID);
void          DmReleaseResource(MemHandle);
uint16        DmFindResource(DmOpenRef, DmResType, DmResID, MemHandle);
MemHandle     DmNewResource(DmOpenRef, DmResType, DmResID, uint32);
Err           DmRemoveResource(DmOpenRef, uint16);
Err           DmWrite(void *, uint32, void *, uint32);
void          ErrDisplayFileLineMsg(int8 *, uint16, int8 *);
Err           SysTaskDelay(int32);
uint16        SysFatalAlert(int8 *);
uint8        *StrCopy(uint8 *, uint8 *);
uint8        *StrCat(uint8 *, uint8 *);
uint16        StrLen(uint8 *);
uint8        *StrIToA(uint8 *, int32);
uint16        StrAToI(uint8 *);
uint32        TimGetSeconds();
void          EvtGetPen(int16 *, int16 *, Boolean *);
void          WinDeleteWindow(WinHandle, Boolean);
WinHandle     WinSetDrawWindow(WinHandle);
WinHandle     WinGetDisplayWindow();
void          WinCopyRectangle(WinHandle, WinHandle, RectangleType *, Coord, Coord, uint8);
Err           SndDoCmd(void *, SndCommandType *, Boolean);
Err           FtrGet(uint32, uint16, uint32 *);
uint32        KeyCurrentState();
uint32        KeySetMask(uint32);
uint16        SysTicksPerSecond();
FileHand      FileOpen(uint16, uint8 *, uint32, uint32, uint32, Err *);
Err           FileClose(FileHand);
int32         FileReadLow(FileHand, void *, int32, Boolean, int32, int32, Err *);
int32         FileWrite(FileHand, void *, int32, int32, Err *);
Err           FileSeek(FileHand, int32, uint8);
int32         FileTell(FileHand, int32 *, Err *);
Err           FileControl(uint8 op, FileHand, void *, int32 *);
Err           WinScreenMode(uint8, uint32 *, uint32 *, uint32 *, Boolean *);
void         *BmpGetBits(BitmapType *);
void          WinPalette(uint8, int16, uint16, RGBColorType *);
BitmapType   *WinGetBitmap(WinHandle);
BitmapType   *BmpCreate(Coord, Coord, uint8, ColorTableType *, uint16 *);
Err           BmpDelete(BitmapType *);
WinHandle     WinCreateBitmapWindow(BitmapType *, uint16 *);

BitmapTypeV3 *BmpCreateBitmapV3(BitmapType *, uint16, void *, const ColorTableType *);
uint16        WinSetCoordinateSystem(uint16);
Err           WinScreenGetAttribute(uint8, uint32 *);
void          EvtGetPenNative(WinHandle, int16 *, int16 *, Boolean *);

Err           VFSFileOpen(uint16, uint8 *, uint16, FileRef *);
Err           VFSFileClose(FileRef);
Err           VFSFileRead(FileRef, uint32, void *, uint32 *);
Err           VFSFileWrite(FileRef, uint32, void *, uint32 *);
Err           VFSFileSeek(FileRef, uint16, int32);
Err           VFSFileEOF(FileRef);
Err           VFSFileTell(FileRef, uint32 *);
Err           VFSVolumeEnumerate(uint16 *, uint32 *);

uint16        PINGetInputAreaState();
void          PINSetInputAreaState(uint16);
void          PINSetInputTriggerState(uint16);
void          StatGetAttribute(uint16, uint32 *);

Err           SysLibLoad(uint32, uint32, uint16 *);
Err           SysLibFind(uint8 *, uint16 *);
Err           SysLibOpen(uint16);
Err           SysLibClose(uint16);

#define       HROpen          SysLibOpen
#define       HRClose         SysLibClose
Err           HRWinScreenMode(uint16, uint8, uint32 *, uint32 *, uint32 *, Boolean *);
void          HRWinGetDisplayExtent(uint16, Coord *, Coord *);
BitmapType   *HRBmpCreate(uint16, Coord, Coord, uint8, ColorTableType *, uint16 *);
WinHandle     HRWinCreateBitmapWindow(uint16, BitmapType *, uint16 *error);
void          HRWinCopyRectangle(uint16, WinHandle, WinHandle, RectangleType *, Coord, Coord, uint8);

#define       SilkLibOpen     SysLibOpen
#define       SilkLibClose    SysLibClose
Err           SilkLibResizeDispWin(uint16, uint8);
Err           SilkLibEnableResize(uint16);
Err           SilkLibDisableResize(uint16);

#endif

/********************************* E O F *********************************/
