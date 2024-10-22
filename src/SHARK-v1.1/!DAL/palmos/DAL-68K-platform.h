/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-68K-platform.h
 */

#ifndef DAL_PLATFORM
#define DAL_PLATFORM

//
// STANDARD HEADERS
//

#include <PalmOS.h>
#include <ExpansionMgr.h>
#include <VFSMgr.h>
#include <DLServer.h>

//
// HARDWARE EXTENTIONS:  HANDERA
//

#define sysTrapHanderaSelector sysTrapOEMDispatch

#define _TRG_CALL_WITH_16BIT_SELECTOR(table, vector, selector) __attribute__ ((__callseq__ ( "move.w #" _Str(selector) ",%%d2; " "trap #" _Str(table) "; dc.w " _Str(vector) )))
#define TRG_TRAP(sel) _TRG_CALL_WITH_16BIT_SELECTOR(_SYSTEM_TABLE, sysTrapHanderaSelector, sel)
#define EXT_TRAP(x) TRG_TRAP(x)

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

#define trgMinVgaFunction          0x0200
#define trgSetScreenMode           (trgMinVgaFunction +  1)
#define trgFrmModify               (trgMinVgaFunction +  15)

extern Err           VgaSetScreenMode(VgaScreenModeType, VgaRotateModeType)
  EXT_TRAP(trgSetScreenMode);

extern Err           VgaFormModify(FormType *, VgaFormModifyType)
  EXT_TRAP(trgFrmModify);

#define trgMinSilkFunction         0x0700
#define trgDrawSilkWindow          (trgMinSilkFunction +  3)
#define trgEraseSilkWindow         (trgMinSilkFunction +  4)
#define trgSilkWindowShown         (trgMinSilkFunction + 13)

extern void          SilkMaximizeWindow()
  EXT_TRAP(trgDrawSilkWindow);

extern void          SilkMinimizeWindow()
  EXT_TRAP(trgEraseSilkWindow);

extern Boolean       SilkWindowMaximized()
  EXT_TRAP(trgSilkWindowShown);

//
// HARDWARE EXTENSIONS:  SONY
//

#define HR_TRAP(x) SYS_TRAP(x)

#define HRTrapWinCopyRectangle      (sysLibTrapCustom+2)
#define HRTrapWinCreateBitmapWindow (sysLibTrapCustom+3)
#define HRTrapWinGetDisplayExtent   (sysLibTrapCustom+26)
#define HRTrapWinScreenMode         (sysLibTrapCustom+48)
#define HRTrapBmpCreate             (sysLibTrapCustom+55)

extern Err           HROpen(UInt16)
  HR_TRAP(sysLibTrapOpen);

extern Err           HRClose(UInt16)
  HR_TRAP(sysLibTrapClose);

extern void HRWinCopyRectangle(UInt16 refNum, WinHandle srcWin, WinHandle dstWin, RectangleType *srcRect, Coord destX, Coord destY, UInt8 mode)
  HR_TRAP(HRTrapWinCopyRectangle);

extern WinHandle     HRWinCreateBitmapWindow(UInt16 refNum, BitmapType *bitmapP, Err *error)
  HR_TRAP(HRTrapWinCreateBitmapWindow);

extern void          HRWinGetDisplayExtent(UInt16, Coord *, Coord *)
  HR_TRAP(HRTrapWinGetDisplayExtent);

extern Err           HRWinScreenMode(UInt16, WinScreenModeOperation, UInt32 *, UInt32 *, UInt32 *, Boolean *)
  HR_TRAP(HRTrapWinScreenMode);

extern BitmapType   *HRBmpCreate(UInt16 refNum, Coord width, Coord height, UInt8 depth, ColorTableType *colortableP, Err *error)
  HR_TRAP( HRTrapBmpCreate);

#define SILK_LIB_TRAP(x) SYS_TRAP(x)

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

extern Err           SilkLibOpen(UInt16)
  SILK_LIB_TRAP(sysLibTrapOpen);

extern Err           SilkLibClose(UInt16)
  SILK_LIB_TRAP(sysLibTrapClose);

extern Err           SilkLibResizeDispWin(UInt16, UInt8)
  SILK_LIB_TRAP(silkLibTrapResizeDispWin);

extern Err           SilkLibEnableResize(UInt16)
  SILK_LIB_TRAP(silkLibTrapEnableResize);

extern Err           SilkLibDisableResize(UInt16)
  SILK_LIB_TRAP(silkLibTrapDisableResize);

#define VskOpen  SilkLibOpen
#define VskClose SilkLibClose
extern Err           VskSetState(UInt16, UInt16, UInt16)
  SILK_LIB_TRAP(vskLibTrapSetState);

//
// HARDWARE EXTENSIONS:  PALM HDD
//

#define sysTrapSysHighDensitySelector 0xA3EC
#define HIGH_DENSITY_TRAP(selector) _SYSTEM_API(_CALL_WITH_SELECTOR)(_SYSTEM_TABLE, sysTrapSysHighDensitySelector, selector)

typedef enum 
{
  winScreenWidth,
  winScreenHeight
} WinScreenAttrType;

typedef struct BitmapTypeV3   BitmapTypeV3;

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

extern BitmapTypeV3 *BmpCreateBitmapV3(BitmapType *bitmapP, UInt16 density, void *bitsP, ColorTableType *colorTableP)
  HIGH_DENSITY_TRAP(HDSelectorBmpCreateBitmapV3);

extern UInt16 WinSetCoordinateSystem(UInt16 coordSys)
  HIGH_DENSITY_TRAP(HDSelectorWinSetCoordinateSystem);

extern Err           WinScreenGetAttribute(WinScreenAttrType, UInt32 *)
  HIGH_DENSITY_TRAP(HDSelectorWinScreenGetAttribute);

extern void          EvtGetPenNative(WinHandle, Int16 *, Int16 *, Boolean *)
  HIGH_DENSITY_TRAP(HDSelectorEvtGetPenNative);

#define sysTrapSysPinSelector 0xA470
#define PINS_TRAP(selector) _SYSTEM_API(_CALL_WITH_SELECTOR)(_SYSTEM_TABLE, sysTrapSysPinSelector, selector)

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

extern Err           PINSetInputAreaState(UInt16)
  PINS_TRAP(pinPINSetInputAreaState);

extern UInt16        PINGetInputAreaState()
  PINS_TRAP(pinPINGetInputAreaState);

extern Err           PINSetInputTriggerState(UInt16)
  PINS_TRAP(pinPINSetInputTriggerState);

extern Err           FrmSetDIAPolicyAttr(FormType *, UInt16)
  PINS_TRAP(pinFrmSetDIAPolicyAttr);

extern Err           StatHide(void)
  PINS_TRAP(pinStatHide);

extern Err           StatShow(void)
  PINS_TRAP(pinStatShow);

extern Err           StatGetAttribute(UInt16, UInt32 *)
  PINS_TRAP(pinStatGetAttribute);

extern Err           SysSetOrientation(UInt16 orientation)
  PINS_TRAP(pinSysSetOrientation);

extern Err           SysSetOrientationTriggerState(UInt16 state)
  PINS_TRAP(pinSysSetOrientationTriggerState);

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

typedef UInt32          SndStreamRef;
typedef Int8            SndStreamMode;
typedef Int8            SndStreamWidth;
typedef Int16           SndSampleType;

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