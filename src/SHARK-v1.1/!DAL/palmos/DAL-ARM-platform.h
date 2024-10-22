/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-ARM-platform.h
 */

#ifndef DAL_PLATFORM
#define DAL_PLATFORM

//
// STANDARD HEADERS
//

typedef signed char     Int8;
typedef signed short    Int16;
typedef signed long     Int32;
typedef unsigned char   UInt8;
typedef unsigned short  UInt16;
typedef unsigned long   UInt32;
typedef unsigned char   Boolean;
typedef char            Char;
typedef UInt16          WChar;
typedef UInt16          Err;
typedef Int16           Coord;
typedef void *          MemPtr;
typedef struct _opaque *MemHandle;

#define sysFtrCreator                       0x70737973  // 'psys' == 0x70737973
#define sysFileTLibrary                     0x6c696272  // 'libr' == 0x6c696272

#define prefSysSoundVolume                  30
#define prefGameSoundVolume                 31

#define sysAppLaunchCmdNormalLaunch         0
#define sysFtrNumROMVersion                 1
#define sysFtrNumOEMCompanyID               20
#define sysFtrNumOEMDeviceID                21

#define sysROMStageDevelopment              0
#define sysMakeROMVersion(major, minor, fix, stage, buildNum) \
        ( \
          (((UInt32)( (UInt8)(major) & 0x0FF)) << 24) | \
          (((UInt32)( (UInt8)(minor) & 0x00F)) << 20) | \
          (((UInt32)( (UInt8)(fix)   & 0x00F)) << 16) | \
          (((UInt32)( (UInt8)(stage) & 0x00F)) << 12) | \
          (((UInt32)((UInt16)(buildNum) & 0x0FFF))) \
         )

#define errNone                             0
#define ftrErrorClass                       0x0C00
#define ftrErrNoSuchFeature                 (ftrErrorClass | 2)

#define sysTrapOEMDispatch                  0xA349

// 
// system
//

#define evtWaitForever                      -1
#define noPreferenceFound                   -1

typedef struct EventType { UInt8 data[24]; } EventType;

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
#define sysTrapSysTaskDelay                 0xA0A0
#define sysTrapSysHandleEvent               0xA0A9
#define sysTrapSysLibFind                   0xA0BA
#define sysTrapTimGetSeconds                0xA0F5
#define sysTrapTimGetTicks                  0xA0F7
#define sysTrapSysReset                     0xA08C
#define sysTrapEvtAddEventToQueue           0xA11B
#define sysTrapEvtGetEvent                  0xA11D
#define sysTrapEvtFlushPenQueue             0xA125
#define sysTrapFtrGet                       0xA27B
#define sysTrapFtrSet                       0xA27C
#define sysTrapSysLibLoad                   0xA2AC
#define sysTrapPrefGetPreference            0xA2D1
#define sysTrapPrefGetAppPreferences        0xA2D3
#define sysTrapPrefSetAppPreferences        0xA2D4
#define sysTrapSysTicksPerSecond            0xA2E9
#define sysTrapFtrPtrNew                    0xA35A
#define sysTrapFtrPtrFree                   0xA35B
#define sysLibTrapOpen                      0xA801
#define sysLibTrapClose                     0xA802
#define sysLibTrapCustom                    0xA805

#define MemPtrFree MemChunkFree
extern MemPtr        MemChunkNew(UInt16, UInt32, UInt16);
extern void          MemChunkFree(MemPtr);
extern MemPtr        MemPtrNew(UInt32);
extern MemHandle     MemPtrRecoverHandle(MemPtr);
extern MemPtr        MemHandleLock(MemHandle);
extern void          MemHandleUnlock(MemHandle);
extern void          MemMove(void *, void *, Int32);
extern void          MemSet(void *, UInt32, UInt8);
extern UInt32        MemHandleSize(MemHandle);
extern Err           MemSemaphoreReserve(Boolean);
extern Err           MemSemaphoreRelease(Boolean);
extern Err           SysTaskDelay(Int32);
extern Boolean       SysHandleEvent(EventType *);
extern Err           SysLibFind(UInt8 *, UInt16 *);
extern UInt32        TimGetSeconds();
extern UInt32        TimGetTicks();
extern void          SysReset();
extern void          EvtAddEventToQueue(EventType *);
extern void          EvtGetEvent(EventType *, Int32);
extern Err           EvtFlushPenQueue();
extern Err           FtrGet(UInt32, UInt16, UInt32 *);
extern Err           FtrSet(UInt32, UInt16, UInt32);
extern Err           SysLibLoad(UInt32, UInt32, UInt16 *);
extern Int32         PrefGetPreference(UInt8);
extern Int16         PrefGetAppPreferences(UInt32, UInt16, void *, UInt16 *, Boolean);
extern void          PrefSetAppPreferences(UInt32, UInt16, Int16, void *, UInt16, Boolean);
extern UInt16        SysTicksPerSecond();
extern Err           FtrPtrNew(UInt32, UInt16, UInt32, void **);
extern Err           FtrPtrFree(UInt32, UInt16);
extern Err           SysLibOpen(UInt16);
extern Err           SysLibClose(UInt16);

//
// input
//

#define chrBackspace                        0x0008
#define vchrPageUp                          0x000B
#define vchrPageDown                        0x000C
#define vchrNextField                       0x0103
#define vchrMenu                            0x0105
#define vchrPrevField                       0x010C
#define vchrHard1                           0x0204
#define vchrHard2                           0x0205
#define vchrHard3                           0x0206
#define vchrHard4                           0x0207
#define vchrCFlashMin                       0x1500

#define keyBitPageUp                        0x00000002
#define keyBitPageDown                      0x00000004
#define keyBitHard1                         0x00000008
#define keyBitHard2                         0x00000010
#define keyBitHard3                         0x00000020
#define keyBitHard4                         0x00000040

#define sysTrapEvtGetPen                    0xA11E
#define sysTrapKeyCurrentState              0xA2A2

extern UInt32        KeyCurrentState();
extern void          EvtGetPen(Int16 *, Int16 *, Boolean *);

// 
// user interface
//

#define frmRedrawUpdateCode                 0x8000

typedef struct FormType       FormType;
typedef struct WindowType     WindowType;

typedef WindowType     *WinHandle;

typedef enum 
{
  nilEvent       = 0,
  penDownEvent,
  penMoveEvent   = 3,
  keyDownEvent,
  winEnterEvent,
  winExitEvent,
  menuEvent      = 21,
  appStopEvent   = 22,
  frmLoadEvent,
  frmOpenEvent,
  frmUpdateEvent = 26,
  frmCloseEvent  = 28,
  firstUserEvent = 0x6000,
  lastUserEvent  = 0x7FFF
} eventsEnum;

typedef Boolean FormEventHandlerType(EventType *eventP);

#define sysTrapFrmInitForm                  0xA16F
#define sysTrapFrmDrawForm                  0xA171
#define sysTrapFrmGetActiveForm             0xA173
#define sysTrapFrmSetActiveForm             0xA174
#define sysTrapFrmGetWindowHandle           0xA17C
#define sysTrapFrmGetFormPtr                0xA17E
#define sysTrapFrmCustomAlert               0xA194
#define sysTrapFrmGotoForm                  0xA19B
#define sysTrapFrmUpdateForm                0xA19D
#define sysTrapFrmSetEventHandler           0xA19F
#define sysTrapFrmCloseAllForms             0xA1A1

extern FormType     *FrmInitForm(UInt16);
extern void          FrmDrawForm(FormType *);
extern FormType     *FrmGetActiveForm();
extern void          FrmSetActiveForm(FormType *);
extern WinHandle     FrmGetWindowHandle(FormType *);
extern FormType     *FrmGetFormPtr(UInt16);
extern UInt16        FrmCustomAlert(UInt16, UInt8 *, UInt8 *, UInt8 *);
extern void          FrmGotoForm(UInt16);
extern void          FrmUpdateForm(UInt16, UInt16);
extern void          FrmSetEventHandler(FormType *, FormEventHandlerType *);
extern void          FrmCloseAllForms();

// 
// data manager
//

typedef UInt32          LocalID;
typedef void *          DmOpenRef;
typedef UInt32          DmResType;
typedef UInt16          DmResID;

#define dmModeWrite                         0x0002

#define sysTrapDmCreateDatabase             0xA041
#define sysTrapDmDeleteDatabase             0xA042
#define sysTrapDmFindDatabase               0xA045
#define sysTrapDmOpenDatabase               0xA049
#define sysTrapDmCloseDatabase              0xA04A
#define sysTrapDmGetResource                0xA05F
#define sysTrapDmReleaseResource            0xA061
#define sysTrapDmNumResources               0xA067
#define sysTrapDmGetResourceIndex           0xA06E
#define sysTrapDmFindResource               0xA065
#define sysTrapDmNewResource                0xA06C
#define sysTrapDmRemoveResource             0xA06D
#define sysTrapDmWrite                      0xA076

extern Err           DmCreateDatabase(UInt16, Int8 *, UInt32, UInt32, Boolean);
extern void          DmDeleteDatabase(UInt16, LocalID);
extern LocalID       DmFindDatabase(UInt16, Int8 *);
extern DmOpenRef     DmOpenDatabase(UInt16, LocalID, UInt16);
extern void          DmCloseDatabase(DmOpenRef);
extern MemHandle     DmGetResource(DmResType, DmResID);
extern void          DmReleaseResource(MemHandle);
extern MemHandle     DmGetResourceIndex(DmOpenRef, UInt16);
extern UInt16        DmNumResources(DmOpenRef);
extern UInt16        DmFindResource(DmOpenRef, DmResType, DmResID, MemHandle);
extern MemHandle     DmNewResource(DmOpenRef, DmResType, DmResID, UInt32);
extern Err           DmRemoveResource(DmOpenRef, UInt16);
extern Err           DmWrite(void *, UInt32, void *, UInt32);

//
// dlk manager
//

#define sysTrapDlkGetSyncInfo               0xA2A9

extern Err           DlkGetSyncInfo(UInt32 *, UInt32 *, void *, UInt8 *, UInt8 *, UInt32 *);

//
// file streaming
//

typedef void *          FileHand;

#define fileModeReadOnly                    (0x80000000UL)
#define fileOriginBeginning                 1
#define fileOriginCurrent                   2
#define fileOriginEnd                       3
#define fileOpGetEOFStatus                  2
#define fileErrorClass                      0x1600
#define fileErrEOF                          (fileErrorClass | 16)

#define sysTrapFileOpen                     0xA332
#define sysTrapFileClose                    0xA333
#define sysTrapFileReadLow                  0xA335
#define sysTrapFileWrite                    0xA336
#define sysTrapFileSeek                     0xA337
#define sysTrapFileTell                     0xA338
#define sysTrapFileControl                  0xA33A

extern FileHand      FileOpen(UInt16, UInt8 *, UInt32, UInt32, UInt32, Err *);
extern Err           FileClose(FileHand);
extern Int32         FileReadLow(FileHand, void *, Int32, Boolean, Int32, Int32, Err *);
extern Int32         FileWrite(FileHand, void *, Int32, Int32, Err *);
extern Err           FileSeek(FileHand, Int32, UInt8);
extern Int32         FileTell(FileHand, Int32 *, Err *);
extern Err           FileControl(UInt8 op, FileHand, void *, Int32 *);

#define FileRead(stream, bufP, objSize, numObj, errP) \
        FileReadLow((stream), (bufP), 0, false, (objSize), (numObj), (errP))
#define FileEOF(__stream__)                           \
        (FileControl(fileOpGetEOFStatus, (__stream__), NULL, NULL) == fileErrEOF)

//
// vfs manager
//

#define sysFileCVFSMgr                      0x7666736d  // 'vfsm' == 0x7666736d
#define vfsFtrIDVersion                     0

typedef struct VolumeInfoTag
{
  UInt32 attributes;
  UInt32 fsType;
  UInt32 fsCreator;
  UInt32 mountClass;
  UInt16 slotLibRefNum;
  UInt16 slotRefNum;
  UInt32 mediaType;
  UInt32 reserved;
} VolumeInfoType;

typedef UInt32          FileRef;

#define vfsIteratorStart                    0L
#define vfsIteratorStop                     0xffffffffL
#define vfsModeRead                         (0x0002U)
#define vfsOriginBeginning                  0
#define vfsOriginCurrent                    1
#define vfsOriginEnd                        2

#define sysTrapFileSystemDispatch           0xA348
#define vfsTrapFileOpen                     3
#define vfsTrapFileClose                    4
#define vfsTrapFileRead                     6
#define vfsTrapFileWrite                    7
#define vfsTrapFileSeek                     10
#define vfsTrapFileEOF                      11
#define vfsTrapFileTell                     12
#define vfsTrapVolumeEnumerate              27
#define vfsTrapVolumeInfo                   28
#define vfsTrapVolumeSize                   31

extern Err           VFSFileOpen(UInt16, UInt8 *, UInt16, FileRef *);
extern Err           VFSFileClose(FileRef);
extern Err           VFSFileRead(FileRef, UInt32, void *, UInt32 *);
extern Err           VFSFileWrite(FileRef, UInt32, void *, UInt32 *);
extern Err           VFSFileSeek(FileRef, UInt16, Int32);
extern Err           VFSFileEOF(FileRef);
extern Err           VFSFileTell(FileRef, UInt32 *);
extern Err           VFSVolumeEnumerate(UInt16 *, UInt32 *);
extern Err           VFSVolumeInfo(UInt16 volRefNum, VolumeInfoType *volInfoP);
extern Err           VFSVolumeSize(UInt16 volRefNum, UInt32 *volumeUsedP, UInt32 *volumeTotalP);

//
// sound manager
//

typedef UInt32          SndStreamRef;
typedef Int8            SndStreamMode;
typedef Int8            SndStreamWidth;
typedef Int16           SndSampleType;

typedef struct SndCommandType { UInt8  cmd; UInt8  reserved; Int32  param1; UInt16 param2; UInt16 param3; } SndCommandType;

enum SndStreamModeTag  { sndInput, sndOutput };
enum SndStreamWidthTag { sndMono,  sndStereo };
enum SndSampleTypeTag  { sndInt8 = 0x01, sndUInt8 = 0x11, sndInt16Big = 0x02, sndInt16Little  = 0x12, sndInt32Big = 0x04, sndInt32Little  = 0x14, sndFloatBig = 0x24, sndFloatLittle  = 0x34 };

typedef Err (*SndStreamBufferCallback)(void *userdata, SndStreamRef channel, void *buffer, UInt32 numberofframes);

#define sndMaxAmp                           64

#define sndCmdFrqOn                         3

#define sysTrapSndDoCmd                     0xA233
#define sysTrapSndStreamCreate              0xA45B
#define sysTrapSndStreamDelete              0xA45C
#define sysTrapSndStreamStart               0xA45D
#define sysTrapSndStreamStop                0xA45F
#define sysTrapSndStreamSetVolume           0xA460

extern Err           SndDoCmd(void *, SndCommandType *, Boolean);
extern Err           SndStreamCreate(SndStreamRef *, SndStreamMode, UInt32, SndSampleType, SndStreamWidth, SndStreamBufferCallback *, void *, UInt32, Boolean);
extern Err           SndStreamDelete(SndStreamRef);
extern Err           SndStreamStart(SndStreamRef);
extern Err           SndStreamStop(SndStreamRef);
extern Err           SndStreamSetVolume(SndStreamRef, Int32);

// 
// window manager
//

#define sysFtrNumWinVersion                 24

#define winPaletteSet                       1
#define winPaletteSetToDefault              2
#define winPaint                            0

typedef enum 
{
  winScreenModeGetDefaults = 0,
  winScreenModeGet,
  winScreenModeSetToDefaults,
  winScreenModeSet,
  winScreenModeGetSupportedDepths,
  winScreenModeGetSupportsColor
} WinScreenModeOperation;

typedef struct RectangleType 
{
  struct { Coord x; Coord y; } topLeft;
  struct { Coord x; Coord y; } extent;
} RectangleType;

typedef struct BitmapType     BitmapType;
typedef struct BitmapTypeV3   BitmapTypeV3;
typedef struct ColorTableType ColorTableType;
typedef struct RGBColorType   { UInt8 index; UInt8 r; UInt8 g; UInt8 b; } RGBColorType;

#define sysTrapWinDeleteWindow              0xA1F8
#define sysTrapWinSetDrawWindow             0xA1FD
#define sysTrapWinGetActiveWindow           0xA1FF
#define sysTrapWinGetDisplayWindow          0xA200
#define sysTrapWinCopyRectangle             0xA209
#define sysTrapWinSetBounds                 0xA300
#define sysTrapWinScreenMode                0xA33E
#define sysTrapBmpGetBits                   0xA376
#define sysTrapWinPalette                   0xA39D
#define sysTrapWinGetBitmap                 0xA3A2
#define sysTrapBmpCreate                    0xA3DD
#define sysTrapBmpDelete                    0xA3DE
#define sysTrapWinCreateBitmapWindow        0xA3E4
#define sysTrapWinGetBounds                 0xA456

#define WinSetWindowBounds WinSetBounds
extern void          WinDeleteWindow(WinHandle, Boolean);
extern WinHandle     WinSetDrawWindow(WinHandle);
extern WinHandle     WinGetActiveWindow();
extern WinHandle     WinGetDisplayWindow();
extern void          WinCopyRectangle(WinHandle, WinHandle, RectangleType *, Coord, Coord, UInt8);
extern void          WinSetBounds(WinHandle, RectangleType *);
extern Err           WinScreenMode(UInt8, UInt32 *, UInt32 *, UInt32 *, Boolean *);
extern void         *BmpGetBits(BitmapType *);
extern void          WinPalette(UInt8, Int16, UInt16, RGBColorType *);
extern BitmapType   *WinGetBitmap(WinHandle);
extern BitmapType   *BmpCreate(Coord, Coord, UInt8, ColorTableType *, UInt16 *);
extern Err           BmpDelete(BitmapType *);
extern WinHandle     WinCreateBitmapWindow(BitmapType *, UInt16 *);
extern void          WinGetBounds(WinHandle, RectangleType *);

/*************************************************************************/

//
// HARDWARE EXTENTIONS:  HANDERA
//

#define sysTrapHanderaSelector              sysTrapOEMDispatch

typedef enum
{
  screenMode1To1 = 1
} VgaScreenModeType;

typedef enum
{
  rotateModeNone = 0
} VgaRotateModeType;

typedef enum
{
  vgaFormModify160To240 = 0
} VgaFormModifyType;

#define trgMinVgaFunction                   0x0200
#define trgSetScreenMode                    (trgMinVgaFunction +  1)
#define trgFrmModify                        (trgMinVgaFunction + 15)

extern Err           VgaSetScreenMode(VgaScreenModeType, VgaRotateModeType);
extern Err           VgaFormModify(FormType *, VgaFormModifyType);

#define trgMinSilkFunction                  0x0700
#define trgDrawSilkWindow                   (trgMinSilkFunction +  3)
#define trgEraseSilkWindow                  (trgMinSilkFunction +  4)
#define trgSilkWindowShown                  (trgMinSilkFunction + 13)

extern void          SilkMaximizeWindow();
extern void          SilkMinimizeWindow();
extern Boolean       SilkWindowMaximized();

//
// HARDWARE EXTENSIONS:  SONY
//

#define HRTrapWinCopyRectangle              (sysLibTrapCustom+2)
#define HRTrapWinCreateBitmapWindow         (sysLibTrapCustom+3)
#define HRTrapWinGetDisplayExtent           (sysLibTrapCustom+26)
#define HRTrapWinScreenMode                 (sysLibTrapCustom+48)
#define HRTrapBmpCreate                     (sysLibTrapCustom+55)


extern Err           HROpen(UInt16);
extern Err           HRClose(UInt16);
extern void          HRWinCopyRectangle(UInt16 refNum, WinHandle srcWin, WinHandle dstWin, RectangleType *srcRect, Coord destX, Coord destY, UInt8 mode);
extern WinHandle     HRWinCreateBitmapWindow(UInt16 refNum, BitmapType *bitmapP, Err *error);
extern void          HRWinGetDisplayExtent(UInt16, Coord *, Coord *);
extern Err           HRWinScreenMode(UInt16, WinScreenModeOperation, UInt32 *, UInt32 *, UInt32 *, Boolean *);
extern BitmapType   *HRBmpCreate(UInt16 refNum, Coord width, Coord height, UInt8 depth, ColorTableType *colortableP, Err *error);

#define silkResizeNormal                    0
#define silkResizeToStatus                  1
#define silkResizeMax                       2
#define vskResizeMax                        silkResizeNormal
#define vskResizeMin                        silkResizeToStatus
#define vskResizeNone                       silkResizeMax

#define vskStateResize                      0
#define vskStateEnable                      1

#define vskResizeDisable                    0
#define vskResizeVertically                 (1<<0)
#define vskResizeHorizontally               (1<<1)

#define silkLibTrapResizeDispWin             sysLibTrapCustom
#define silkLibTrapEnableResize             (sysLibTrapCustom+1)
#define silkLibTrapDisableResize            (sysLibTrapCustom+2)
#define vskLibTrapSetState                  (sysLibTrapCustom+6)

extern Err           SilkLibOpen(UInt16);
extern Err           SilkLibClose(UInt16);
extern Err           SilkLibResizeDispWin(UInt16, UInt8);
extern Err           SilkLibEnableResize(UInt16);
extern Err           SilkLibDisableResize(UInt16);

#define VskOpen(x)  SilkLibOpen(x)
#define VskClose(x) SilkLibClose(x)
extern Err           VskSetState(UInt16, UInt16, UInt16);

//
// HARDWARE EXTENSIONS:  PALM HDD
//

#define sysTrapSysHighDensitySelector       0xA3EC

typedef enum 
{
  winScreenWidth,
  winScreenHeight
} WinScreenAttrType;

#define winDisplayChangedEvent              0x4101

#define sysFtrNumUIHardwareFlags            27
#define sysFtrNumUIHardwareHas5Way          0x00000001

#define HDSelectorBmpCreateBitmapV3         7
#define HDSelectorWinSetCoordinateSystem    8
#define HDSelectorWinScreenGetAttribute     14
#define HDSelectorEvtGetPenNative           17

#define kDensityLow                         72
#define kDensityOneAndAHalf                 108
#define kDensityDouble                      144

#define kCoordinatesNative                  0
#define kCoordinatesStandard                72

extern BitmapTypeV3 *BmpCreateBitmapV3(BitmapType *bitmapP, UInt16 density, void *bitsP, ColorTableType *colorTableP);
extern UInt16        WinSetCoordinateSystem(UInt16 coordSys);
extern Err           WinScreenGetAttribute(WinScreenAttrType, UInt32 *);
extern void          EvtGetPenNative(WinHandle, Int16 *, Int16 *, Boolean *);

#define sysTrapSysPinSelector               0xA470

typedef enum
{
  pinInputAreaOpen = 0,
  pinInputAreaClosed
} PINInputAreaStateType;

typedef enum 
{
  pinInputTriggerEnabled,
  pinInputTriggerDisabled,
  pinInputTriggerNone
} PINInputTriggerStateType;

#define sysOrientationUser                  0
#define sysOrientationPortrait              1
#define sysOrientationLandscape             2
#define sysOrientationReversePortrait       3
#define sysOrientationReverseLandscape      4

#define sysOrientationTriggerDisabled       0
#define sysOrientationTriggerEnabled        1

#define statAttrDimension                   1
#define frmDIAPolicyCustom                  1

#define sysNotifyDisplayResizedEvent        'scrs'

#define pinAPIVersion1_0                    0x01000000
#define pinAPIVersion1_1                    0x01100000

#define pinPINSetInputAreaState             0
#define pinPINGetInputAreaState             1
#define pinPINSetInputTriggerState          2
#define pinFrmSetDIAPolicyAttr              14
#define pinStatHide                         16
#define pinStatShow                         17
#define pinStatGetAttribute                 18
#define pinSysSetOrientation                20
#define pinSysSetOrientationTriggerState    22

extern Err           PINSetInputAreaState(UInt16);
extern UInt16        PINGetInputAreaState();
extern Err           PINSetInputTriggerState(UInt16);
extern Err           FrmSetDIAPolicyAttr(FormType *, UInt16);
extern Err           StatHide();
extern Err           StatShow();
extern Err           StatGetAttribute(UInt16, UInt32 *);
extern Err           SysSetOrientation(UInt16 orientation);
extern Err           SysSetOrientationTriggerState(UInt16 state);
//
// HARDWARE EXTENSIONS:  TAPWAVE ZODIAC
//

#undef vcharTapWaveMin
#undef vcharTapWaveMax
#define vchrTapWaveMin                      0x1B00
#define vchrTapWaveMax                      0x1BFF

#define vchrTwConfig                        (vchrTapWaveMin + 1)
#define vchrBluetooth                       (vchrTapWaveMin + 2)

#define vchrFunction                        (vchrTapWaveMin + 3)
#define vchrTriggerLeft                     (vchrTapWaveMin + 4)
#define vchrTriggerRight                    (vchrTapWaveMin + 5)
#define vchrActionUp                        (vchrTapWaveMin + 6)
#define vchrActionRight                     (vchrTapWaveMin + 7)
#define vchrActionDown                      (vchrTapWaveMin + 8)
#define vchrActionLeft                      (vchrTapWaveMin + 9)

#define vchrTriggerA                        vchrTriggerLeft
#define vchrTriggerB                        vchrTriggerRight
#define vchrActionA                         vchrActionUp
#define vchrActionB                         vchrActionRight
#define vchrActionC                         vchrActionDown
#define vchrActionD                         vchrActionLeft

#define keyBitFunction                      0x00008000
#define keyBitTriggerA                      0x04000000
#define keyBitTriggerB                      0x08000000
#define keyBitActionA                       0x10000000
#define keyBitActionB                       0x20000000
#define keyBitActionC                       0x40000000
#define keyBitActionD                       0x80000000

//
// HARDWARE EXTENSIONS:  STREAMING AUDIO
// 

#define sndFtrIDVersion                     0

//
// HARDWARE EXTENSIONS:  PALM GENERIC
//

#define keyBitJogUp                         0x00001000
#define keyBitJogDown                       0x00002000
#define keyBitJogPush                       0x00004000
#define keyBitRockerUp                      0x00010000
#define keyBitRockerDown                    0x00020000
#define keyBitRockerLeft                    0x00040000
#define keyBitRockerRight                   0x00080000
#define keyBitRockerSelect                  0x00100000
#define keyBitNavLeft                       0x01000000
#define keyBitNavRight                      0x02000000
#define keyBitNavSelect                     0x04000000

#define vchrThumbWheelUp                    0x012E
#define vchrThumbWheelDown                  0x012F
#define vchrThumbWheelPush                  0x0130
#define vchrThumbWheelBack                  0x0131
#define vchrRockerUp                        0x0132
#define vchrRockerDown                      0x0133
#define vchrRockerLeft                      0x0134
#define vchrRockerRight                     0x0135
#define vchrRockerCenter                    0x0136

#define vchrJogUp                           0x1700
#define vchrJogDown                         0x1701
#define vchrJogPageUp                       0x1703
#define vchrJogPageDown                     0x1704
#define vchrJogPress                        0x1705

#define vchrTrgJogUp                        (vchrCFlashMin + 4)
#define vchrTrgJogDown                      (vchrCFlashMin + 5)
#define vchrTrgJogPush                      (vchrCFlashMin + 6)
#define vchrSilkToggle                      (vchrCFlashMin + 10)

#ifdef vchrPalmMin
#undef vchrPalmMin
#endif
#define vchrPalmMin                         0x0500

#define vchrNavChange                       (vchrPalmMin + 3)
#define navBitLeft                          0x0004
#define navBitRight                         0x0008
#define navBitSelect                        0x0010

#endif

/********************************* E O F *********************************/
