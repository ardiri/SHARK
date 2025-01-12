/******************************************************************************
 *
 * Copyright (c) 2002 Sony Corporation.
 * All rights reserved.
 *
 * File: SonySilkLib.h
 *
 * Description:
 *        This file defines the Sony virtual silk library.
 *
 * History:
 *
 *****************************************************************************/

#ifndef __SONYSILKLIB_H__
#define __SONYSILKLIB_H__

/* ------------------------------------- */
/* Trap def                              */
/* ------------------------------------- */

#define sonySysFileCSilkLib      'SeSi'
#define sonySysLibNameSilk       "Sony Silk Library"

#define SILK_LIB_TRAP(trapNum)   SYS_TRAP(trapNum)

/* ------------------------------------- */
/* Constants                             */
/* ------------------------------------- */

#define sonySilkErrorClass       (oemErrorClass | 0x700)

#define silkLibErrParam          (sonySilkErrorClass | 1)  // invalid parameter
#define silkLibErrNotOpen        (sonySilkErrorClass | 2)  // library is not open
#define silkLibErrStillOpen      (sonySilkErrorClass | 3)  // returned from SilkLibClose() if
                                                           // the library is still open by others
#define silkLibErrNotAvailable   (sonySilkErrorClass | 4)  // memory error occurred
#define silkLibErrResizeDisabled (sonySilkErrorClass | 5)  // cannot resize

#define vskErrParam               silkLibErrParam
#define vskErrNotOpen             silkLibErrNotOpen
#define vskErrStillOpen           silkLibErrStillOpen
#define vskErrNotAvailable        silkLibErrNotAvailable
#define vskErrResizeDisabled      silkLibErrResizeDisabled
#define vskErrSlkwNotFound        (sonySilkErrorClass | 6)
#define vskErrSlkwOpenFailed      (sonySilkErrorClass | 7)
#define vskErrSlkwCloseFailed     (sonySilkErrorClass | 8)
#define vskErrSlkwStartFailed     (sonySilkErrorClass | 9)
#define vskErrSlkwStopFailed      (sonySilkErrorClass | 10)
#define vskErrSlkwLoadFailed      (sonySilkErrorClass | 11)
#define vskErrFuncNotAvailable    (sonySilkErrorClass | 12)

/*    Window Extent    */
#define hrSilkHeight              (130)
#define hrSilkWidth               (320)
#define stdSilkHeight             (65)
#define stdSilkWidth              (160)

#define hrStatusHeight            (30)
#define hrStatusWidth             (320)
#define stdStatusHeight           (15)
#define stdStatusWidth            (160)

#define silkLibAPIVertion         (0x00000002)
#define vskAPIVertion             silkLibAPIVertion
#define vskVersionNum1            (0x00010000)
#define vskVersionNum2            (0x00020000)

/*    stateType      */
#define vskStateResize            (0)
#define silkResizeNormal          (0)
#define silkResizeToStatus        (1)
#define silkResizeMax             (2)
#define vskResizeMax              silkResizeNormal
#define vskResizeMin              silkResizeToStatus
#define vskResizeNone             silkResizeMax
#define vskStateEnable            (1)
#define vskStateAnimation         (2)
#define vskAnimationNone          (0)
#define vskAnimationFast          (1)
#define vskAnimationSlow          (2)
#define vskStateAutoResize        (3)

/*    slkwType        */
#define vskSlkwTypeSilk           (0)
#define vskSlkwTypeStatus         (1)

/* ------------------------------------- */
/* Type def                              */
/* ------------------------------------- */

#define silkLibTrapResizeDispWin  sysLibTrapCustom
#define silkLibTrapEnableResize   sysLibTrapCustom+1
#define silkLibTrapDisableResize  sysLibTrapCustom+2
#define silkLibTrapGetAPIVersion  sysLibTrapCustom+3
#define silkLibLastTrap           sysLibTrapCustom+4

#define VskTrapGetAPIVersion      silkLibTrapGetAPIVersion
#define VskTrapSetCurrentSlkw     silkLibTrapGetAPIVersion+1
#define VskTrapGetCurrentSlkw     silkLibTrapGetAPIVersion+2
#define VskTrapSetState           silkLibTrapGetAPIVersion+3
#define VskTrapGetState           silkLibTrapGetAPIVersion+4
#define VskTrapEnablePalmSilk     silkLibTrapGetAPIVersion+5
#define VskTrapGetPalmSilkEnabled silkLibTrapGetAPIVersion+6
#define VskTrapTimerWrite         silkLibTrapGetAPIVersion+7
#define VskTrapDoCommand          silkLibTrapGetAPIVersion+8
#define VskTrapSetDrawWindow      silkLibTrapGetAPIVersion+9
#define VskTrapRestoreDrawWindow  silkLibTrapGetAPIVersion+10
#define VskLastTrap               silkLibTrapGetAPIVersion+11

/* ------------------------------------- */
/* API Prototypes                        */
/* ------------------------------------- */

extern Err SilkLibOpen(UInt16 refNum)
        SILK_LIB_TRAP(sysLibTrapOpen);

extern Err SilkLibClose(UInt16 refNum)
        SILK_LIB_TRAP(sysLibTrapClose);

extern Err SilkLibSleep(UInt16 refNum)
        SILK_LIB_TRAP(sysLibTrapSleep);

extern Err SilkLibWake(UInt16 refNum)
        SILK_LIB_TRAP(sysLibTrapWake);

extern Err SilkLibResizeDispWin(UInt16 refNum, UInt8 win)
        SILK_LIB_TRAP(silkLibTrapResizeDispWin);

extern Err SilkLibEnableResize(UInt16 refNum)
        SILK_LIB_TRAP(silkLibTrapEnableResize);

extern Err SilkLibDisableResize(UInt16 refNum)
        SILK_LIB_TRAP(silkLibTrapDisableResize);

extern UInt32 SilkLibGetAPIVersion(UInt16 refNum)
        SILK_LIB_TRAP(silkLibTrapGetAPIVersion);

// new APIs
extern Err VskOpen(UInt16 refNum)
        SILK_LIB_TRAP(sysLibTrapOpen);

extern Err VskClose(UInt16 refNum)
        SILK_LIB_TRAP(sysLibTrapClose);

extern Err VskSleep(UInt16 refNum)
        SILK_LIB_TRAP(sysLibTrapSleep);

extern Err VskWake(UInt16 refNum)
        SILK_LIB_TRAP(sysLibTrapWake);

// this will be removed
//extern Err VskResize(UInt16 refNum, UInt8 reqSize);

// this will be removed
//extern Err VskEnableResize(UInt16 refNum);

// this will be removed
//extern Err VskDisableResize(UInt16 refNum);

extern UInt32 VskGetAPIVersion(UInt16 refNum)
        SILK_LIB_TRAP(VskTrapGetAPIVersion);

extern Err VskSetCurrentSlkw(UInt16 refNum, UInt16 slkwType, UInt32 creator)
        SILK_LIB_TRAP(VskTrapSetCurrentSlkw);

extern Err VskGetCurrentSlkw(UInt16 refNum, UInt16 slkwType, UInt32 *creatorP)
        SILK_LIB_TRAP(VskTrapGetCurrentSlkw);

extern Err VskSetState(UInt16 refNum, UInt16 stateType, UInt16 state)
        SILK_LIB_TRAP(VskTrapSetState);

extern Err VskGetState(UInt16 refNum, UInt16 stateType, UInt16 *stateP)
        SILK_LIB_TRAP(VskTrapGetState);

extern Err VskEnablePalmSilk(UInt16 refNum, Boolean enable)
        SILK_LIB_TRAP(VskTrapEnablePalmSilk);

extern Err VskGetPalmSilkEnabled(UInt16 refNum, Boolean *graffiti, Boolean *penButton)
        SILK_LIB_TRAP(VskTrapGetPalmSilkEnabled);

extern Err VskTimerWrite(UInt16 refNum, UInt16 slkwType, UInt32 interval/*msec*/)
        SILK_LIB_TRAP(VskTrapTimerWrite);

extern Err VskDoCommand(UInt16 refNum, UInt32 creator, UInt16 command,
                UInt32 data1, UInt32 data2)
        SILK_LIB_TRAP(VskTrapDoCommand);

extern Err VskSetDrawWindow(UInt16 refNum, UInt16 slkwType)
        SILK_LIB_TRAP(VskTrapSetDrawWindow);

extern Err VskRestoreDrawWindow(UInt16 refNum, UInt16 slkwType)
        SILK_LIB_TRAP(VskTrapRestoreDrawWindow);

#endif // __SONYSILKLIB_H__

