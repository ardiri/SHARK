/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)PACE.h
 */

// Custom Built:
// - palmos 4.x SDK headers optimized for use of r9 table PACE callbacks

#define __byte_swap16(n) \
        ( ((((uint16) n) << 8) & 0xFF00) | \
          ((((uint16) n) >> 8) & 0x00FF) )

#define __byte_swap32(n) \
        ( ((((uint32) n) << 24) & 0xFF000000) |  \
          ((((uint32) n) <<  8) & 0x00FF0000) |  \
          ((((uint32) n) >>  8) & 0x0000FF00) |  \
          ((((uint32) n) >> 24) & 0x000000FF) )

#define __read_unaligned32(addr)  \
        ( ((((uint8 *)(addr))[0]) << 24) | \
          ((((uint8 *)(addr))[1]) << 16) | \
          ((((uint8 *)(addr))[2]) <<  8) | \
          ((((uint8 *)(addr))[3])) )

#define __write_unaligned32(addr, value) \
        ( ((uint8 *)(addr))[0] = (uint8)((uint32)(value) >> 24), \
          ((uint8 *)(addr))[1] = (uint8)((uint32)(value) >> 16), \
          ((uint8 *)(addr))[2] = (uint8)((uint32)(value) >>  8), \
          ((uint8 *)(addr))[3] = (uint8)((uint32)(value)) )

#define __read_byte32(addr)  \
        ( ((((uint8 *)(addr))[3]) << 24) | \
          ((((uint8 *)(addr))[2]) << 16) | \
          ((((uint8 *)(addr))[1]) <<  8) | \
          ((((uint8 *)(addr))[0])) )

#define __read_byte32_16(addr)  \
        ( ((((uint16 *)(addr))[1]) << 16) | \
          ((((uint16 *)(addr))[0])) )

#define __write_byte32(addr, value) \
        ( ((uint8 *)(addr))[3] = (uint8)((uint32)(value) >> 24), \
          ((uint8 *)(addr))[2] = (uint8)((uint32)(value) >> 16), \
          ((uint8 *)(addr))[1] = (uint8)((uint32)(value) >>  8), \
          ((uint8 *)(addr))[0] = (uint8)((uint32)(value)) )

#define __write_byte32_16(addr, value) \
        ( ((uint16 *)(addr))[1] = (uint16)((uint32)(value) >> 16), \
          ((uint16 *)(addr))[0] = (uint16)((uint32)(value)) )

#define __read_unaligned16(addr)  \
        ( ((((uint8 *)(addr))[0]) <<  8) | \
          ((((uint8 *)(addr))[1])) )

#define __read_byte16(addr)  \
        ( ((((uint8 *)(addr))[1]) <<  8) | \
          ((((uint8 *)(addr))[0])) )

#define __write_unaligned16(addr, value) \
        ( ((uint8 *)(addr))[0] = (uint8)((uint16)(value) >>  8), \
          ((uint8 *)(addr))[1] = (uint8)((uint16)(value)) )

#define __write_byte16(addr, value) \
        ( ((uint8 *)(addr))[1] = (uint8)((uint16)(value) >>  8), \
          ((uint8 *)(addr))[0] = (uint8)((uint16)(value)) )

//------------------------------------------------------------------------
// PalmOS: Data Types
//------------------------------------------------------------------------

// generics

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
typedef UInt32          LocalID;

typedef void *          MemPtr;
typedef struct _opaque *MemHandle;

// user interface

typedef Int16           Coord;

typedef struct
{
  Coord x;
  Coord y;
} PointType;

typedef struct 
{
  PointType  topLeft;
  PointType  extent;
} RectangleType;

typedef struct FormType FormType;

typedef struct WindowType WindowType;
typedef WindowType *WinHandle;

//------------------------------------------------------------------------
// PalmOS: Constants
//------------------------------------------------------------------------

#ifndef NULL
#define NULL 0
#endif

#ifndef true
  enum {false = 0, true};
#endif

#define sysAppLaunchCmdNormalLaunch     0x0000

#define errNone                         0x0000

//------------------------------------------------------------------------
// PalmOS: Memory
//------------------------------------------------------------------------

#define MemPtrFree MemChunkFree

// function prototypes

extern void    MemSet(MemPtr, Int32, UInt8);
extern MemPtr  MemPtrNew(UInt32);
extern MemPtr  MemChunkNew(UInt32, UInt32, UInt16);
extern void    MemChunkFree(MemPtr);
extern UInt32  MemHandleSize(MemHandle);
extern MemPtr  MemHandleLock(MemHandle);
extern Err     MemHandleUnlock(MemHandle);
extern Err     MemHeapFreeBytes(UInt16, UInt32 *, UInt32 *);
extern void    HALMemorySetStorageAreaProtectionState(Boolean);

// ::PACE/MemSet.o
// ::PACE/MemPtrNew.o
// ::PACE/MemChunkNew.o
// ::PACE/MemChunkFree.o
// ::PACE/MemHandleSize.o
// ::PACE/MemHandleLock.o
// ::PACE/MemHandleUnlock.o
// ::PACE/MemSemaphoreReserve.o
// ::PACE/MemSemaphoreRelease.o
// ::PACE/MemHeapFreeBytes.o
// ::PACE/HALMemorySetStorageAreaProtectionState.o

//------------------------------------------------------------------------
// PalmOS: Data Manager
//------------------------------------------------------------------------

typedef UInt32 DmResType;
typedef UInt16 DmResID;
typedef void * DmOpenRef;

#define dmModeReadWrite                 0x0003

#define dmMaxRecordIndex                0xffff

extern LocalID   DmFindDatabase(const Char *);
extern Err       DmCreateDatabase(const Char *, UInt32, UInt32, Boolean);
extern Err       DmDeleteDatabase(LocalID);
extern DmOpenRef DmOpenDatabase(LocalID, UInt16);
extern Err       DmCloseDatabase(DmOpenRef);
extern UInt16    DmFindResource(DmOpenRef, DmResType, DmResID, MemHandle);
extern MemHandle DmNewResource(DmOpenRef, DmResType, DmResID, UInt32);
extern MemHandle DmGetResource(DmResType, DmResID);
extern MemHandle DmGetResourceIndex(DmOpenRef, UInt16);
extern UInt16    DmNumResources(DmOpenRef);
extern void      DmRemoveResource(DmOpenRef, UInt16);
extern void      DmReleaseResource(MemHandle);
extern Err       DmWrite(void *, UInt32, const void *, UInt32);

// ::PACE/DmFindDatabase.o
// ::PACE/DmCreateDatabase.o
// ::PACE/DmDeleteDatabase.o
// ::PACE/DmOpenDatabase.o
// ::PACE/DmCloseDatabase.o
// ::PACE/DmFindResource.o
// ::PACE/DmNewResource.o
// ::PACE/DmGetResource.o
// ::PACE/DmGetResourceIndex.o
// ::PACE/DmNumResources.o
// ::PACE/DmRemoveResource.o
// ::PACE/DmReleaseResource.o
// ::PACE/DmWrite.o

//------------------------------------------------------------------------
// PalmOS: Feature Manager
//------------------------------------------------------------------------

typedef struct 
{
  UInt32   notifyType;
  UInt32   broadcaster;
  void    *notifyDetailsP;
  void    *userDataP;
  Boolean  handled;
  UInt8    reserved2;
} SysNotifyParamType;

typedef Err     (*SysNotifyProcPtr)(SysNotifyParamType *notifyParamsP);

#define sysNotifyNormalPriority         0

#define sysFtrCreator                   'psys' 

#define sysFtrNumROMVersion             1
#define sysFtrNumOEMCompanyID           20
#define sysFtrNumOEMDeviceID            21
#define sysFtrNumWinVersion             24

extern Err    FtrGet(UInt32, UInt16, UInt32 *);
extern Err    FtrPtrNew(UInt32, UInt16, UInt32, void **);
extern Err    FtrPtrFree(UInt32, UInt16);

extern Err    SysCurAppDatabase(LocalID *);
extern Err    SysNotifyRegister(LocalID, UInt32,
                                SysNotifyProcPtr, Int8, void *);
extern Err    SysNotifyUnregister(LocalID, UInt32, Int8);

// ::PACE/FtrGet.o
// ::PACE/FtrPtrNew.o
// ::PACE/FtrPtrFree.o
// ::PACE/SysCurAppDatabase.o
// ::PACE/SysNotifyRegister.o
// ::PACE/SysNotifyUnregister.o

//------------------------------------------------------------------------
// PalmOS: Events
//------------------------------------------------------------------------

#define evtWaitForever                  -1
#define evtNoWait                        0

#define nilEvent                        0x0000
#define penDownEvent                    0x0001
#define penMoveEvent                    0x0003
#define keyDownEvent                    0x0004
#define winEnterEvent                   0x0005
#define winExitEvent                    0x0006
#define menuEvent                       0x0015 // 21
#define appStopEvent                    0x0016 // 22
#define frmLoadEvent                    0x0017 // 23
#define frmOpenEvent                    0x0018 // 24
#define frmUpdateEvent                  0x001A // 26
#define frmCloseEvent                   0x001C // 28

#define winDisplayChangedEvent          0x4101
#define winDisplayChangedEvent_CUSTOM   0x5999 // SHARK DEFINE!!

typedef struct EventType 
{
   UInt16  eType;
   Boolean penDown;
   UInt8   tapCount;
   Int16   screenX;
   Int16   screenY;
   
   union 
   {
     struct 
     { 
       UInt16        datum[8];
     } generic;

     struct 
     {
       WChar         chr;
       UInt16        keyCode;
       UInt16        modifiers;
     } keyDown;     

     struct
     {
       UInt16        formID;
     } frmLoad;

     struct
     {
       RectangleType newBounds;
     } winDisplayChanged;
   } data;

} EventType;

typedef Boolean FormEventHandlerType(EventType *eventP);

#define EvtGetEvent              SysEventGet
#define EvtAddEventToQueue       SysEventAddToQueue
#define EvtAddUniqueEventToQueue SysEventAddUniqueToQueue

// function prototypes

extern void    SysEventGet(EventType *, Int32);
extern void    SysEventAddToQueue(EventType *);
extern void    SysEventAddUniqueToQueue(EventType *, UInt32, Boolean);
extern Boolean SysHandleEvent(EventType *);
extern void    FrmSetEventHandler(FormType *, FormEventHandlerType *);
extern Boolean FrmDispatchEvent(EventType *);

// ::PACE/SysEventGet.o
// ::PACE/SysEventAddToQueue.o
// ::PACE/SysEventAddUniqueToQueue.o
// ::PACE/SysHandleEvent.o
// ::PACE/FrmSetEventHandler.o
// ::PACE/FrmDispatchEvent.o

//------------------------------------------------------------------------
// PalmOS: User Interface
//------------------------------------------------------------------------

typedef struct BitmapType   BitmapType;
typedef struct BitmapTypeV3 BitmapTypeV3;

typedef Int16  WinScreenModeOperation;
typedef Int16  WinScreenAttrType;
typedef Int16  WinDrawOperation;
typedef Int16  WindowFormatType;

#define kCoordinatesNative              0
#define kCoordinatesStandard            72

#define kDensityLow                     72
#define kDensityOneAndAHalf             108
#define kDensityDouble                  144
#define kDensityTriple                  216
#define kDensityQuadruple               288 

#define winScreenModeSetToDefaults      2
#define winScreenModeSet                3

#define winScreenWidth                  0
#define winScreenHeight                 1
#define winScreenRowBytes               2
#define winScreenDensity                5

#define winPaint                        0

#define screenFormat                    0
#define genericFormat                   1
#define nativeFormat                    2

extern void        FrmGotoForm(UInt16);
extern FormType   *FrmInitForm(UInt16);
extern void        FrmDrawForm(FormType *);
extern FormType   *FrmGetActiveForm();
extern void        FrmSetActiveForm(FormType *);
extern void        FrmCloseAllForms();
extern void        FrmCustomAlert(UInt16, 
                                  const Char *, const Char *, const Char *);
extern UInt16      WinSetCoordinateSystem(UInt16);
extern Err         WinScreenGetAttribute(WinScreenAttrType, UInt32 *);

extern Err         WinScreenMode(WinScreenModeOperation, 
                                 UInt32 *, UInt32 *, UInt32 *, Boolean *);

extern WinHandle   WinCreateOffscreenWindow(Coord, Coord, WindowFormatType, UInt16 *);
extern void       *BmpGetBits(BitmapType *);

extern BitmapType *WinGetBitmap(WinHandle);
extern void        WinDeleteWindow(WinHandle, Boolean);
extern void        WinCopyRectangle(WinHandle, WinHandle, RectangleType *,
                                    Coord, Coord, WinDrawOperation);
extern void        WinDisplayToWindowPt(Coord *, Coord *);

// ::PACE/FrmGotoForm.o
// ::PACE/FrmInitForm.o
// ::PACE/FrmDrawForm.o
// ::PACE/FrmGetActiveForm.o
// ::PACE/FrmSetActiveForm.o
// ::PACE/FrmCloseAllForms.o
// ::PACE/FrmCustomAlert.o
// ::PACE/WinSetCoordinateSystem.o
// ::PACE/WinScrenGetAttribute.o
// ::PACE/WinScreenMode.o
// ::PACE/WinCreateOffscreenWindow.o
// ::PACE/BmpGetBits.o
// ::PACE/WinGetBitmap.o
// ::PACE/WinDeleteWindow.o
// ::PACE/WinCopyRectangle.o
// ::PACE/WinDisplayToWindowPt.o

//------------------------------------------------------------------------
// PalmOS: Palm DIA (Dynamic Input Area)
//------------------------------------------------------------------------

#define sysNotifyDisplayResizedEvent    'scrs'

#define pinCreator                      'pins'
#define pinFtrAPIVersion                1

#define pinAPIVersion1_0                0x01000000
#define pinAPIVersion1_1                0x01103000

#define frmDIAPolicyCustom              1

#define pinInputTriggerEnabled          0

extern WinHandle WinSetDrawWindow(WinHandle);
extern void      WinGetDisplayExtent(Coord *, Coord *);
extern void      FrmGetFormBounds(const FormType *, RectangleType *);
extern WinHandle WinGetDisplayWindow();
extern void      WinSetBounds(WinHandle, RectangleType *);
extern WinHandle FrmGetWindowHandle(const FormType *);
extern Err       PINSetInputTriggerState(UInt16);
extern Err       WinSetConstraintsSize(WinHandle, Coord, Coord, Coord, 
                                                  Coord, Coord, Coord);
extern Err       FrmSetDIAPolicyAttr(FormType *, UInt16);

// ::PACE/WinSetDrawWindow.o
// ::PACE/WinGetDisplayExtent.o
// ::PACE/FrmGetFormBounds.o
// ::PACE/WinGetDisplayWindow.o
// ::PACE/WinSetBounds.o
// ::PACE/FrmGetWindowHandle.o
// ::PACE/PINSetInputTriggerState.o
// ::PACE/WinSetConstraintsSize.o
// ::PACE/FrmSetDIAPolicyAttr.o

extern Err       PINSetInputTriggerState_PACE(UInt16);
extern Err       WinSetConstraintsSize_PACE(WinHandle, Coord, Coord, Coord, 
                                                       Coord, Coord, Coord);
extern Err       FrmSetDIAPolicyAttr_PACE(FormType *, UInt16);

// ::PACE/PACE.o

//------------------------------------------------------------------------
// PalmOS: System
//------------------------------------------------------------------------

typedef struct 
{
  Int16 second;
  Int16 minute;
  Int16 hour;
  Int16 day;
  Int16 month;
  Int16 year;
  Int16 weekDay;
} DateTimeType;

#define noPreferenceFound               -1

extern Err       SysTaskDelay(Int32);
extern UInt16    SysTicksPerSecond();
extern UInt32    TimGetSeconds();
extern void      TimSecondsToDateTime(UInt32, DateTimeType *);

extern Int16     PrefGetAppPreferences(UInt32, UInt16, 
                                       void *, UInt16 *, Boolean);
extern void      PrefSetAppPreferences(UInt32, UInt16, Int16, 
                                       const void *, UInt16, Boolean);

// ::PACE/SysTaskDelay.o
// ::PACE/SysTicksPerSecond.o
// ::PACE/TimGetSeconds.o
// ::PACE/TimSecondsToDateTime.o
// ::PACE/PrefGetAppPreferences.o
// ::PACE/PrefSetAppPreferences.o

//------------------------------------------------------------------------
// PalmOS: Input
//------------------------------------------------------------------------

#define chrBackspace                    0x0008  // standard characters
#define chrPageUp                       0x000B
#define chrPageDown                     0x000C
#define chrEnter                        0x000D 

#define chrMenu                         0x0105

#define chrThumbWheelUp                 0x012E  // generic thumbwheel
#define chrThumbWheelDown               0x012F
#define chrThumbWheelPush               0x0130

#define chrRockerUp                     0x0132  // 5-way navigator
#define chrRockerDown                   0x0133
#define chrRockerLeft                   0x0134
#define chrRockerRight                  0x0135
#define chrRockerCenter                 0x0136
#define chrHardRockerCenter             0x013D

#define chrNavChange                    0x0503  // palmone navigator
#define navBitLeft                      0x0004
#define navBitRight                     0x0008
#define navBitSelect                    0x0010

#define chrJogUp                        0x1700  // sony jog-dial
#define chrJogDown                      0x1701
#define chrJogPress                     0x1705

extern UInt32    KeyCurrentState();
extern void      EvtGetPen(Coord *, Coord *, Boolean *);
extern void      EvtGetPenNative(WinHandle, Coord *, Coord *, Boolean *);

// ::PACE/KeyCurrentState.o
// ::PACE/EvtGetPen.o
// ::PACE/EvtGetPenNative.o

//------------------------------------------------------------------------
// PalmOS: String
//------------------------------------------------------------------------

extern Int16     StrPrintF(Char *s, const Char *formatStr, ...);

// ::PACE/StrPrintF.o

//------------------------------------------------------------------------
// PalmOS: Sound Manager Interface
//------------------------------------------------------------------------

typedef UInt32   SndStreamRef;
typedef Int8     SndStreamMode;
typedef Int8     SndStreamWidth;
typedef Int16    SndSampleType;

typedef Err (*SndStreamBufferCallback)(void *, SndStreamRef, void *, UInt32);

#define sysFileCSoundMgr                'sndm'
#define sndFtrIDVersion                 0

#define sndInput                        0
#define sndOutput                       1    // SndStreamMode

#define sndMono                         0
#define sndStereo                       1    // SndStreamWidth

#define sndInt8                         0x01
#define sndUInt8                        0x11
#define sndInt16Little                  0x12 // SndSampleType

extern Err     SndStreamCreate(SndStreamRef *, SndStreamMode, 
                               UInt32, SndSampleType, SndStreamWidth, 
                               SndStreamBufferCallback, 
                               void *, UInt32, Boolean);
extern Err     SndStreamStart(SndStreamRef);
extern Err     SndStreamStop(SndStreamRef);
extern Err     SndStreamSetVolume(SndStreamRef, Int32);
extern Err     SndStreamDelete(SndStreamRef);

// ::PACE/SndStreamCreate.o
// ::PACE/SndStreamStart.o
// ::PACE/SndStreamStop.o
// ::PACE/SndStreamSetVolume.o
// ::PACE/SndStreamDelete.o

extern Err     SndStreamCreate_PACE(SndStreamRef *, SndStreamMode, 
                                    UInt32, SndSampleType, SndStreamWidth, 
                                    SndStreamBufferCallback, 
                                    void *, UInt32, Boolean);
extern Err     SndStreamStart_PACE(SndStreamRef);
extern Err     SndStreamStop_PACE(SndStreamRef);
extern Err     SndStreamSetVolume_PACE(SndStreamRef, Int32);
extern Err     SndStreamDelete_PACE(SndStreamRef);

// ::PACE/PACE.o

//------------------------------------------------------------------------
// PalmOS: Virtual File Manager Interface
//------------------------------------------------------------------------

typedef UInt32 FileRef;
typedef UInt16 FileOrigin;

typedef struct FileInfoTag
{
  UInt32  attributes;
  Char   *nameP;
  UInt16  nameBufLen;
} FileInfoType;

#define sysFileCVFSMgr                  'vfsm'
#define vfsFtrIDVersion                 0

#define vfsModeExclusive                0x0001
#define vfsModeRead                     0x0002
#define vfsModeWrite                    (0x0004 | vfsModeExclusive)
#define vfsModeCreate                   0x0008
#define vfsModeTruncate                 0x0010
#define vfsModeReadWrite                (vfsModeRead | vfsModeWrite)

#define vfsFileAttrReadOnly             0x0001
#define vfsFileAttrHidden               0x0002
#define vfsFileAttrSystem               0x0004
#define vfsFileAttrVolumeLabel          0x0008
#define vfsFileAttrDirectory            0x0010
#define vfsFileAttrArchive              0x0020
#define vfsFileAttrLink                 0x0040
#define vfsFileAttrAll                  0x007f

#define vfsOriginBeginning              0
#define vfsOriginCurrent                1
#define vfsOriginEnd                    2

#define vfsIteratorStart                0
#define vfsIteratorStop                 0xffffffffL

extern Err     VFSVolumeEnumerate(UInt16 *, UInt32 *);
extern Err     VFSDirEntryEnumerate(FileRef, UInt32 *, FileInfoType *);

extern Err     VFSDirCreate(UInt16, Char *);
extern Err     VFSFileCreate(UInt16, Char *);
extern Err     VFSFileDelete(UInt16, Char *);
extern Err     VFSFileRename(UInt16, Char *, Char *);
extern Err     VFSFileGetAttributes(FileRef, UInt32 *);

extern Err     VFSFileOpen(UInt16, Char *, UInt16, FileRef *);
extern Err     VFSFileRead(FileRef, UInt32, void *, UInt32 *);
extern Err     VFSFileWrite(FileRef, UInt32, void *, UInt32 *);
extern Err     VFSFileSeek(FileRef, FileOrigin, Int32);
extern Err     VFSFileTell(FileRef, UInt32 *);
extern Err     VFSFileEOF(FileRef);
extern Err     VFSFileClose(FileRef);

// ::PACE/VFSVolumeEnumerate.o
// ::PACE/VFSDirEntryEnumerate.o
// ::PACE/VFSDirCreate.o
// ::PACE/VFSFileCreate.o
// ::PACE/VFSFileDelete.o
// ::PACE/VFSFileRename.o
// ::PACE/VFSFileGetAttributes.o
// ::PACE/VFSFileOpen.o
// ::PACE/VFSFileRead.o
// ::PACE/VFSFileWrite.o
// ::PACE/VFSFileSeek.o
// ::PACE/VFSFileTell.o
// ::PACE/VFSFileEOF.o
// ::PACE/VFSFileClose.o

//------------------------------------------------------------------------
// PalmOS: Kernel Timer 
//------------------------------------------------------------------------

typedef void (*KALTimerProcPtr)(void *userDataP);

extern Err     KALTimerCreate(UInt32 *, UInt32, KALTimerProcPtr, void *);
extern Err     KALTimerDelete(UInt32);
extern Err     KALTimerSet(UInt32, UInt32);

// :: PACE/KALTimerCreate.o
// :: PACE/KALTimerDelete.o
// :: PACE/KALTimerSet.o

//------------------------------------------------------------------------
// PalmOS: PACE Interface
//------------------------------------------------------------------------

typedef UInt32   Call68KFuncType(const void *, UInt32, const void *, UInt32);

/********************************* EOF ***********************************/
