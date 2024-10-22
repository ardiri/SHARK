/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL.c
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

#ifdef _M_68K
static DAL_Globals globals;
DAL_Globals *g;
#endif

DAL_Globals *
DAL_GetGlobals()
{
#ifdef _M_ARM
  GLOBALS_ACCESS;
#endif
  return g;
}

#ifdef _M_68K
#define E_eTYPE_SET(e, v)               ((EventType *)(e))->eType = (v);
#define E_eTYPE_GET(e)                  ((EventType *)(e))->eType
#define E_eTYPE_GET_LOAD_FORMID(e)      ((EventType *)(e))->data.frmLoad.formID
#define E_eTYPE_GET_WIN_ENTER_WIN(e)    ((EventType *)(e))->data.winEnter.enterWindow
#define E_eTYPE_GET_WIN_EXIT_WIN(e)     ((EventType *)(e))->data.winExit.exitWindow
#define E_eType_GET_KEY_DOWN_CHR(e)     ((EventType *)(e))->data.keyDown.chr
#define E_eType_GET_KEY_DOWN_KEYCODE(e) ((EventType *)(e))->data.keyDown.keyCode
#endif

#ifdef _M_ARM
#define E_eTYPE_SET(e, v)               { ((EventType *)(e))->data[0] = ((v) >> 8) & 0xff; ((EventType *)(e))->data[1] = (v) & 0xff; }
#define E_eTYPE_GET(e)                      (Int16)((((EventType *)(e))->data[0]  << 8)  |  ((EventType *)(e))->data[1])
#define E_eTYPE_GET_LOAD_FORMID(e)         (UInt16)((((EventType *)(e))->data[8]  << 8)  |  ((EventType *)(e))->data[9])
#define E_eTYPE_GET_WIN_ENTER_WIN(e)    (WinHandle)((((EventType *)(e))->data[8]  << 24) | (((EventType *)(e))->data[9]  << 16) | \
                                                    (((EventType *)(e))->data[10] << 8)  |  ((EventType *)(e))->data[11])
#define E_eTYPE_GET_WIN_EXIT_WIN(e)     (WinHandle)((((EventType *)(e))->data[12] << 24) | (((EventType *)(e))->data[13] << 16) | \
                                                    (((EventType *)(e))->data[14] << 8)  |  ((EventType *)(e))->data[15])
#define E_eType_GET_KEY_DOWN_CHR(e)         (WChar)((((EventType *)(e))->data[8]  << 8)  |  ((EventType *)(e))->data[9])
#define E_eType_GET_KEY_DOWN_KEYCODE(e)    (UInt16)((((EventType *)(e))->data[10] << 8)  |  ((EventType *)(e))->data[11])
#endif

/*************************************************************************
 *
 * MMC Insert/Removal Handling
 *
 *************************************************************************/

#ifdef _M_68K
Err notifyCardMounted(SysNotifyParamType *notifyParamsP)  { notifyParamsP->handled |= vfsHandledUIAppSwitch; return 0; }
Err notifyCardRemoval(SysNotifyParamType *notifyParamsP)  {                                                  return 0; }
Err notifyDIAResized(SysNotifyParamType *notifyParamsP)   { EventType e = {}; e.eType = winDisplayChangedEvent; EvtAddEventToQueue(&e); return 0; }
#endif

/*************************************************************************
 *
 * Platform Depending Application Framework
 *
 *************************************************************************/

boolean
DAL_Initialize()
{
  boolean result;
  GLOBALS_ACCESS;

  result = false;

  // allocate the information block
  g->platform        = (void *)SYSTEM_MemPtrNew(sizeof(PALM_PlatformInfo));
  g->device.platform = (void *)NULL;
  if (g->platform != NULL)
  {
    _MemSet(g->platform, sizeof(PALM_PlatformInfo), 0);

    // load configuration options
    DAL_Configuration();
    result = true;
  }

  return result;
}

void
DAL_Configuration()
{
  PALM_PlatformInfo *platform;
#ifdef _M_68K
  UInt32            *depthPtr, depthsToTry[] = { 8, 4, 2, 1 };  // max depth is 8bpp on 68k
#endif
#ifdef _M_ARM
  UInt32            *depthPtr, depthsToTry[] = { 16, 8, 4, 2, 1 };
//UInt32            *depthPtr, depthsToTry[] = { 8, 4, 2, 1 };
#endif
  UInt32             width, height, version;
  Err                err;
  UInt16             libRef;
#ifdef GFX_FULL_SCREEN
  Coord              extentX, extentY;
//UInt32             datachunk;
#endif
  UInt32             companyID, deviceID;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // HANDERA
    if (FtrGet(0x54524720, 2, &version) != ftrErrNoSuchFeature)   // 'TRG ' = 0x54524720
    {
      platform->device.type = DEVICE_HANDERA;

      // we want 1:1 display co-ordinates (yes, we are native handera)
      VgaSetScreenMode(screenMode1To1, rotateModeNone);

      // do we have a silk screen?
#ifdef GFX_FULL_SCREEN
      if (FtrGet(0x54524720, 7, &version) != ftrErrNoSuchFeature) // 'TRG ' = 0x54524720
      {
        platform->device.silk.available = true;
        platform->device.silk.maximized = SilkWindowMaximized();

        // we want full screen access, minimize the silk screen right now
        SilkMinimizeWindow();
      }
      else
#endif
        platform->device.silk.available = false;

      // we know what this device is, lets move on!
      goto DEV_ABORT_DETECT;
    }

    // SONY HANDHELD
    err = SysLibFind("Sony HR Library", &libRef);
    if (err != errNone) err = SysLibLoad(0x6c696272, 0x536c4872, &libRef);   // 'libr' = 0x6c696272, 'SlHr' = 0x536c4872
    if (err == errNone) err = HROpen(libRef);
    if (err == errNone)
    {
      platform->device.type      = DEVICE_SONY;
      platform->device.libRef[0] = libRef;

      // do we have a silk screen?
#ifdef GFX_FULL_SCREEN
      err = SysLibFind("Sony Silk Library", &libRef);
      if (err != errNone) err = SysLibLoad(0x6c696272, 0x53655369, &libRef); // 'libr' = 0x6c696272, 'SeSi' = 0x53655369 (arm)
      if (err != errNone) err = SysLibLoad(0x6c696272, 0x536c5369, &libRef); // 'libr' = 0x6c696272, 'SlSi' = 0x536c5369 (68k)
      if (err == errNone) err = SilkLibOpen(libRef);  // same as VskOpen
      if (err == errNone)
      {
        platform->device.silk.available = true;
        platform->device.silk.maximized = false;  // its not possible to detect this :(
        platform->device.libRef[1]      = libRef;

        // lets go full screen!
        if (FtrGet(0x536F4E79, 0x0102, &version) != errNone) // 'SoNy' = 0x536F4E79, vsk feature
        {
          SilkLibEnableResize(platform->device.libRef[1]);
          SilkLibResizeDispWin(platform->device.libRef[1], silkResizeMax);
          SilkLibDisableResize(platform->device.libRef[1]);
        }
        else
        {
          // version == 0x000200 = NX/NZ (portrait), else UX50 (landscape)
          if (version == 0x00020000) 
            VskSetState(platform->device.libRef[1], vskStateEnable, vskResizeVertically);
          else
            VskSetState(platform->device.libRef[1], vskStateEnable, vskResizeHorizontally);
          VskSetState(platform->device.libRef[1], vskStateResize, vskResizeNone);
          VskSetState(platform->device.libRef[1], vskStateEnable, vskResizeDisable);
        }
      }
      else
#endif
        platform->device.silk.available = false;

      // we know what this device is, lets move on!
      goto DEV_ABORT_DETECT;
    }

    // PALM HDD
    FtrGet(sysFtrCreator, sysFtrNumWinVersion, &version);
    if (version >= 4)
    {
      platform->device.type = DEVICE_PALM_HDD;

      // do we have a silk screen?
      if (FtrGet(0x70696e73, 1, &version) != ftrErrNoSuchFeature) // 'pins' = 0x70696e73
      {
        platform->device.libRef[0] = (version > pinAPIVersion1_0);

        platform->device.silk.available = true;
        platform->device.silk.maximized = (PINGetInputAreaState() == pinInputAreaClosed);

        // we want full screen access, minimize the silk screen right now
#ifdef GFX_FULL_SCREEN
/**
        FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &companyID);
        FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID,  &deviceID);
        if ((companyID == 0x50616C6D) && (deviceID == 0x41727a31)) // 'Palm' = 0x50616C6D, 'Arz1' = 0x41727a31
        {
          // force portrait mode on T|T3 = faster graphics
          SysSetOrientation(sysOrientationPortrait);
        }
 **/
        PINSetInputAreaState(pinInputAreaClosed);
        PINSetInputTriggerState(pinInputTriggerDisabled);
        if (PALM_HDD_PINS_NEW) 
        {
          SysSetOrientationTriggerState(sysOrientationTriggerDisabled);
          StatHide();   // all new 1.1 PINS allow hiding of STAT
        }
#else
        // tapwave zodiac (we want to force 480x320)
        if (FtrGet(0x54707776, 0, &version) != ftrErrNoSuchFeature) // 'Tpwv' == 0x54707776
        {
          PINSetInputAreaState(pinInputAreaClosed);
          PINSetInputTriggerState(pinInputTriggerDisabled);
          if (PALM_HDD_PINS_NEW) 
          {
            SysSetOrientationTriggerState(sysOrientationTriggerDisabled);
            StatHide();   // all new 1.1 PINS allow hiding of STAT
          }
        }
        else
        {
          PINSetInputAreaState(pinInputAreaOpen);
          PINSetInputTriggerState(pinInputTriggerDisabled);
          if (PALM_HDD_PINS_NEW) 
            SysSetOrientationTriggerState(sysOrientationTriggerDisabled);
        }
#endif
      }
      else
        platform->device.silk.available = false;

      // tapwave zodiac (we want to force 480x320)
      if (FtrGet(0x54707776, 0, &version) != ftrErrNoSuchFeature) // 'Tpwv' == 0x54707776
      {
        if (FtrGet(0x70696e73, 1, &version) != ftrErrNoSuchFeature) // 'pins' = 0x70696e73
        {
          platform->device.silk.available = true;
          platform->device.silk.maximized = (PINGetInputAreaState() == pinInputAreaClosed);
  
          // we want full screen access, minimize the silk screen right now
          PINSetInputAreaState(pinInputAreaClosed);
          PINSetInputTriggerState(pinInputTriggerDisabled);
          platform->device.libRef[0] = (version > pinAPIVersion1_0);
          if (PALM_HDD_PINS_NEW) 
          {
            SysSetOrientationTriggerState(sysOrientationTriggerDisabled);
            StatHide();   // all new 1.1 PINS allow hiding of STAT
          }
        }
      }

      // we know what this device is, lets move on!
      goto DEV_ABORT_DETECT;
    }

    platform->device.type = DEVICE_GENERIC;

DEV_ABORT_DETECT:

    // screen depth/resolution determination
    depthPtr = depthsToTry;

/**
    if (!platform->device.silk.available)
      depthPtr++;   // lets skip 16bit if resolution silk not available
 **/

    switch (platform->device.type)
    {
      case DEVICE_GENERIC:
           width  = 160;
           height = 160;
           while (WinScreenMode(winScreenModeSet, NULL, NULL, depthPtr, NULL))
             depthPtr++;

           platform->device.density = DENSITY_SINGLE;
           break;

      case DEVICE_HANDERA:
           width  = 240;
           height = 240;
           while (WinScreenMode(winScreenModeSet, NULL, NULL, depthPtr, NULL))
             depthPtr++;

           // lets determine the latest screen resolution sizes
           if (platform->device.silk.available)
             height = 320;  // we must override this value, handera doesn't provide :(

           platform->device.density = DENSITY_ONEANDHALF;
           break;

      case DEVICE_SONY:
           width  = 320;
           height = 320;
           while (HRWinScreenMode(platform->device.libRef[0],
                                  winScreenModeSet, &width, &height, depthPtr, NULL))
             depthPtr++;

           // lets determine the latest screen resolution sizes
           if (platform->device.silk.available)
           {
#ifdef GFX_FULL_SCREEN
             HRWinGetDisplayExtent(platform->device.libRef[0], &extentX, &extentY);
             width  = extentX;
             height = extentY;
#else
             width  = MIN(width, height);
             height = width;                                    // assume square             
#endif
           }
           else
           {
             width  = MIN(width, height);
             height = width;                                    // assume square
           }

           platform->device.density = DENSITY_DOUBLE;
           break;

      case DEVICE_PALM_HDD:
           WinScreenGetAttribute(winScreenWidth,  &width);
           WinScreenGetAttribute(winScreenHeight, &height);
           while (WinScreenMode(winScreenModeSet, NULL, NULL, depthPtr, NULL))
             depthPtr++;

           // lets determine the latest screen resolution sizes
           if (platform->device.silk.available)
           {
#ifdef GFX_FULL_SCREEN
             // not PINS 1.1?
             if (!PALM_HDD_PINS_NEW)
             {
// ** HEIGHT = 32 
//             StatGetAttribute(statAttrDimension, &datachunk);
//             height -= (datachunk & 0xffff);                  // lower bits = height

               height -= 32;                                    // status bar height
             }
#ifdef DEV_DYNAMIC_RESIZE
             else
             if ((width == 320) && (height == 352)) // special case! (T|T3 closed)
             {
               StatShow();
               height = width;
             }
#endif
#else
             // force 320x320 - only if not tapwave
             if (FtrGet(0x54707776, 0, &version) == ftrErrNoSuchFeature) // 'Tpwv' == 0x54707776
             {
               width  = MIN(width, height);
               height = width;                                  // assume square 
             }
#endif
           }
           else 
           {
             width  = MIN(width, height);
             height = width;                                    // assume square
           }

                if (width < 240) platform->device.density = DENSITY_SINGLE;
           else if (width < 320) platform->device.density = DENSITY_ONEANDHALF;
           else                  platform->device.density = DENSITY_DOUBLE;
 
           // SPECIAL:: due to a bug on T|T5, we cannot go full screen (todo)
           FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &companyID);
           FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID,  &deviceID);
           if ((companyID == 0x50616c6d) && (deviceID == 0x546e5435)) // 'Palm' / 'TnT5'
             height -= 32;

           break;

      default:
           break;
    }

    g->device.display.depth  = (uint8)*depthPtr;
    g->device.input.stylus   = true;       // every palm has a stylus
    g->device.display.width  = width;
    g->device.display.height = height;
#ifdef _M_68K
    g->device.display.bits   = BmpGetBits(WinGetBitmap(WinGetDisplayWindow())); // DSA (68k = ok)
#else
    g->device.display.bits   = NULL; // we will define this later in DAL_LCDInitialize()
#endif
    g->device.input.analog   = false;

    // volume settings
    platform->audio.volume = sndMaxAmp;
//  platform->audio.volume = MIN(platform->audio.volume, PrefGetPreference(prefSysSoundVolume)); 
    platform->audio.volume = MIN(platform->audio.volume, PrefGetPreference(prefGameSoundVolume)); 
  }
}

void
DAL_Terminate()
{
  PALM_PlatformInfo *platform;
  UInt32             version;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // we need to shutdown our DAL configuration
    switch (platform->device.type)
    {
      case DEVICE_GENERIC:
           WinScreenMode(winScreenModeSetToDefaults, NULL, NULL, NULL, NULL);
           break;

      case DEVICE_HANDERA:
           WinScreenMode(winScreenModeSetToDefaults, NULL, NULL, NULL, NULL);

           // ensure the silk screen is restored as appropriate
           if (platform->device.silk.available)
           {
             if (platform->device.silk.maximized)
               SilkMaximizeWindow();
           }
           break;

      case DEVICE_SONY:
           HRWinScreenMode(platform->device.libRef[0],
                           winScreenModeSetToDefaults, NULL, NULL, NULL, NULL);
           HRClose(platform->device.libRef[0]);

           // ensure the silk screen is restored as appropriate
           if (platform->device.silk.available)
           {
             if (platform->device.silk.maximized) ; // ignored :(

             // restore back to normal and shutdown
             if (FtrGet(0x536F4E79, 0x0102, &version) != errNone) // 'SoNy' = 0x536F4E79, vsk feature
             {
               SilkLibEnableResize(platform->device.libRef[1]);
               SilkLibResizeDispWin(platform->device.libRef[1], silkResizeNormal);
               SilkLibDisableResize(platform->device.libRef[1]);
             }
             else
             {
               // version == 0x000200 = NX/NZ (portrait), else UX50 (landscape)
               if (version == 0x00020000) 
                 VskSetState(platform->device.libRef[1], vskStateEnable, vskResizeVertically);
               else
                 VskSetState(platform->device.libRef[1], vskStateEnable, vskResizeHorizontally);
               VskSetState(platform->device.libRef[1], vskStateResize, vskResizeMax);
               VskSetState(platform->device.libRef[1], vskStateEnable, vskResizeDisable);
             }

             SilkLibClose(platform->device.libRef[1]); // same as VskClose
           }
           break;

      case DEVICE_PALM_HDD:
           WinScreenMode(winScreenModeSetToDefaults, NULL, NULL, NULL, NULL);

           // ensure the silk screen is restored as appropriate
           if (platform->device.silk.available)
           {
             if (platform->device.silk.maximized)
               PINSetInputAreaState(pinInputAreaClosed);
             else
               PINSetInputAreaState(pinInputAreaOpen);

             PINSetInputTriggerState(pinInputTriggerEnabled);
             if (PALM_HDD_PINS_NEW) 
               SysSetOrientationTriggerState(sysOrientationTriggerEnabled);
           }
           break;

      default:
           break;
    }
  }

  // restore the system palette
  WinPalette(winPaletteSetToDefault, 0, 0, NULL);

  // release information blocks
  if (g->platform != NULL) SYSTEM_MemPtrFree(g->platform);
  g->platform = NULL;
}

boolean
GlobalsInitialize()
{
  boolean result;

  // initialize all sub-components
  result = true;
  result &= _MemInitialize();  // REQ: must be done first
  result &= _LCDInitialize();  // REQ: must be done second
  result &= _FileInitialize();
  result &= _InpInitialize();
  result &= _SndInitialize();
  result &= _SysInitialize();
  result &= PlatformInitialize();

  result &= _GfxInitialize();
  result &= _StrInitialize();
  result &= _Gfx16Initialize();
  result &= _UIInitialize();

  return result;
}

void
GlobalsTerminate()
{
  // shutdown all sub-components
  _UITerminate();
  _Gfx16Terminate();
  _StrTerminate();
  _GfxTerminate();

  PlatformTerminate();
  _SysTerminate();
  _SndTerminate();
  _InpTerminate();
  _FileTerminate();
  _LCDTerminate();             // REQ: must be done second last
  _MemTerminate();             // REQ: must be done last
}

Boolean
MainFormEventHandler(EventType *event)
{
  PALM_PlatformInfo *platform;
  RectangleType      bounds;
  FormType          *frm;
  Boolean            processed = false;
  UInt32             romVersion;
#ifndef GFX_FULL_SCREEN
  UInt32             version;
#endif
#ifdef DEV_FIX_PORTRAIT_T3
  UInt32             companyID, deviceID;
#endif
  GLOBALS_ACCESS;

  frm = FrmGetActiveForm();
  switch (E_eTYPE_GET(event))
  {
    case frmOpenEvent:

         // access device/platform information block
         platform = (PALM_PlatformInfo *)g->platform;
         if (platform != NULL)
         {
           switch (platform->device.type)
           {
             case DEVICE_HANDERA:
                  // HANDERA: ensure windows are really 240x
                  VgaFormModify(frm, vgaFormModify160To240);
                  break;

             case DEVICE_PALM_HDD:

#ifdef DEV_FIX_PORTRAIT_T3
                  FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &companyID);
                  FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID,  &deviceID);
                  if ((companyID == 0x50616C6D) && (deviceID == 0x41727a31)) // 'Palm' = 0x50616C6D, 'Arz1' = 0x41727a31
                  {
                    // force portrait mode on T|T3 = faster graphics
                    SysSetOrientation(sysOrientationPortrait);
                  }
#endif

                  // we want full screen access, minimize the silk screen right now
                  if (platform->device.silk.available)
                  {
                    // bug in T|T5 requires this be done in frmOpenEvent
                    FrmSetDIAPolicyAttr(frm, frmDIAPolicyCustom);

#ifdef GFX_FULL_SCREEN
                    PINSetInputAreaState(pinInputAreaClosed);
                    PINSetInputTriggerState(pinInputTriggerDisabled);
                    if (PALM_HDD_PINS_NEW) 
                    {
                      SysSetOrientationTriggerState(sysOrientationTriggerDisabled);
#ifdef DEV_DYNAMIC_RESIZE
                      if (g->device.display.width != g->device.display.height)  // special case!?
#endif
                      StatHide();   // all new 1.1 PINS allow hiding of STAT
                    }
#else
                    // tapwave zodiac (we want to force 480x320)
                    if (FtrGet(0x54707776, 0, &version) != ftrErrNoSuchFeature) // 'Tpwv' == 0x54707776
                    {
                      PINSetInputAreaState(pinInputAreaClosed);
                      PINSetInputTriggerState(pinInputTriggerDisabled);
                      if (PALM_HDD_PINS_NEW) 
                      {
                        SysSetOrientationTriggerState(sysOrientationTriggerDisabled);
                        StatHide();   // all new 1.1 PINS allow hiding of STAT
                      }
                    }
                    else
                    {
                      PINSetInputAreaState(pinInputAreaOpen);
                      PINSetInputTriggerState(pinInputTriggerDisabled);
                      if (PALM_HDD_PINS_NEW) 
                        SysSetOrientationTriggerState(sysOrientationTriggerDisabled);
                    }
#endif
                  }
                  break;

             default:
                  break;
           }
         }

         // get the rom version from the device
         FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

         // set the window boundaries (preparing for DIA)
         if (romVersion >= sysMakeROMVersion(4,0,0,sysROMStageDevelopment,0))
         {
           WinGetBounds(WinGetDisplayWindow(), &bounds);
           WinSetWindowBounds(FrmGetWindowHandle(frm), &bounds);
         }

    case frmUpdateEvent:
         FrmDrawForm(frm);
         _GfxRepaint();

         processed = true;
         break;

    default:
         break;
  }

  return processed;
}

void
InitApplication()
{
  // load the application preferences
  PlatformLoadPreferences();

  // we must have an active form at all times.
  FrmGotoForm(MainForm);
}

void
EventLoop()
{
  PALM_PlatformInfo *platform;
  event              _e;
  uint16             eType;
  EventType          e;
  boolean            processed;
  UInt32             timeStamp;
  UInt32             bonusTick;
  UInt32             companyID, deviceID;
  Boolean            handleDIA;
  GLOBALS_ACCESS;

  FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &companyID);
  FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID,  &deviceID);
  if (((companyID == 0x67726D6E) && (deviceID == 0x33323030)) ||  // 'grmn' = 0x67726D6E, '3200' = 0x33323030
      ((companyID == 0x67726D6E) && (deviceID == 0x33363030)))    // 'grmn' = 0x67726D6E, '3600' = 0x33363030
  {
    handleDIA = true;
  }
  else
    handleDIA = false;

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    _MemSet(&_e, sizeof(event), 0);
    _e.eType = _appStartEvent;
    DAL_ApplicationHandleEvent(&_e);

    // CHEAT:: post an extra resize to force resizing right! (Garmin iQue)
    if (handleDIA)
    {
      E_eTYPE_SET(&e, winDisplayChangedEvent)
      EvtAddEventToQueue(&e);
    }

    // reset the timer (just in case)
    platform->system.timerLastFrameUpdate = TimGetTicks() - platform->system.ticksPerFrame;
    bonusTick = platform->system.ticksPerFrameE;

    do
    {
      EvtGetEvent(&e, 0);   // platform->system.evtTimeOut  (timer is controlled below)
      processed = false;

      //
      // SPECIAL OS EVENTS:
      //

      // form load handling (need to configure event handlers/setup)
      if (E_eTYPE_GET(&e) == frmLoadEvent)
      {
        UInt16   formID = E_eTYPE_GET_LOAD_FORMID(&e);
        FormType *frm   = FrmInitForm(formID);

        // screen size policy handling?
        if ((platform->device.type == DEVICE_PALM_HDD) && (platform->device.silk.available))
          FrmSetDIAPolicyAttr(frm, frmDIAPolicyCustom);

        FrmSetActiveForm(frm);
        FrmSetEventHandler(frm, NULL);  // we dont want an event handler, handle manually
      }

      // win entry/exit (ie: form just pop-ed up)?
      if (E_eTYPE_GET(&e) == winEnterEvent)
      {
        if (E_eTYPE_GET_WIN_ENTER_WIN(&e) == (WinHandle)FrmGetFormPtr(MainForm))
        {
          platform->system.evtTimeOut = 1;
          platform->system.timerLastFrameUpdate = TimGetTicks() - platform->system.ticksPerFrame;

          FrmUpdateForm(MainForm, frmRedrawUpdateCode);
        }

        goto ABORT_EVENT_HANDLING;
      }

      if (E_eTYPE_GET(&e) == winExitEvent)
      {
        if (E_eTYPE_GET_WIN_EXIT_WIN(&e) == (WinHandle)FrmGetFormPtr(MainForm))
        {
          platform->system.timerLastFrameUpdate = TimGetTicks() - platform->system.ticksPerFrame;
          platform->system.evtTimeOut = evtWaitForever;
        }

        goto ABORT_EVENT_HANDLING;
      }

      // we dont want handera devices to use the silk toggle button in any way
      if ((E_eTYPE_GET(&e) == keyDownEvent) && (E_eType_GET_KEY_DOWN_CHR(&e) == vchrSilkToggle)) goto ABORT_EVENT_HANDLING;

      // we have handled all display changed events
      if (E_eTYPE_GET(&e) == winDisplayChangedEvent) 
      {
        // do we need to handle this event? (Garmin iQue)
        if (handleDIA)
        {
#ifdef GFX_FULL_SCREEN
           PINSetInputAreaState(pinInputAreaClosed);
           PINSetInputTriggerState(pinInputTriggerDisabled);
#else
           PINSetInputAreaState(pinInputAreaOpen);
           PINSetInputTriggerState(pinInputTriggerDisabled);
#endif
        }

        goto ABORT_EVENT_HANDLING;
      }

      //
      // EVENT HANDLING
      //

      // make sure the active window is the form
      if (WinGetActiveWindow() == (WinHandle)FrmGetFormPtr(MainForm))
      {
        // some events need to be 'migrated' into independent structure
        _MemSet(&_e, sizeof(event), 0);
        switch (E_eTYPE_GET(&e))
        {
          case nilEvent:
               timeStamp = TimGetTicks();

               // update screen (animation) if possible
               if ((timeStamp - platform->system.timerLastFrameUpdate) >= platform->system.ticksPerFrame)
               {
                 // animation requirement
                 platform->system.timerLastFrameUpdate = timeStamp;

                 _e.eType = _nilEvent;
                 processed = true;
               }
               break;

          case penDownEvent:
          case penMoveEvent:
               processed = _PenHandleEvent(&_e);
               break;

          case keyDownEvent:
               // special power processing
               if (E_eType_GET_KEY_DOWN_CHR(&e) == 286)  // vchrPowerOff
               {
                 _e.eType        = _keyEvent;
                 _e.data.key.chr = _key_focuslost;
                 DAL_ApplicationHandleEvent(&_e);
     
                 // we want to force this onto the system (to turn off)
                 SysHandleEvent(&e);
                 goto ABORT_EVENT_HANDLING;
               }
               else

               // special d-pad processing of key
               if (E_eType_GET_KEY_DOWN_CHR(&e) == vchrNavChange)
               {
                 if (E_eType_GET_KEY_DOWN_KEYCODE(&e) & navBitLeft)
                 {
                   if ((g->input.keyMask & KeyPadLeft) == 0)
                   {
                     _e.eType        = _keyEvent;
                     _e.data.key.chr = _key_padleft;
                   }
                 }
                 else
                 if (E_eType_GET_KEY_DOWN_KEYCODE(&e) & navBitRight)
                 {
                   if ((g->input.keyMask & KeyPadRight) == 0)
                   {
                     _e.eType        = _keyEvent;
                     _e.data.key.chr = _key_padright;
                   }
                 }
                 else
                 if (E_eType_GET_KEY_DOWN_KEYCODE(&e) & navBitSelect)
                 {
                   if ((g->input.keyMask & KeyPadSelect) == 0)
                   {
                     _e.eType        = _keyEvent;
                     _e.data.key.chr = _key_padselect;
                   }
                 }
                 processed = true;
               }
               else
                 processed = _KeyHandleEvent(E_eType_GET_KEY_DOWN_CHR(&e), &_e);
               break;

          case appStopEvent:
               _e.eType = _appStopEvent;
               processed = true;
               break;

          default:
               break;
        }

        // pass it on? or, handle it on device?
        if (processed)
        {
          if (!DAL_ApplicationHandleEvent(&_e))
            SysHandleEvent(&e);
        }
        else
        if (!SysHandleEvent(&e))
          MainFormEventHandler(&e);  // let the form handler handle it

ABORT_EVENT_HANDLING:

        // platform independent 'processing' of events
        while (_SysEventAvailable())
        {
          _SysGetEvent(&_e);
          eType = _e.eType;
          DAL_ApplicationHandleEvent(&_e);

          // application termination condition
          if (eType == _appStopEvent) E_eTYPE_SET(&e, appStopEvent)
        }
      }
      else
      if (!SysHandleEvent(&e))
        MainFormEventHandler(&e);  // let the form handler handle it

      //
      // ANIMATION HANDLING
      //

      platform->system.timerPoint = TimGetTicks();
      if (platform->system.evtTimeOut != evtWaitForever)
      {
        // calculate the delay required
        platform->system.timerDiff  = (platform->system.timerPoint - platform->system.timerLastFrameUpdate);
        platform->system.evtTimeOut = (platform->system.timerDiff >= platform->system.ticksPerFrame) ?
                                        1 : (platform->system.ticksPerFrame - platform->system.timerDiff);

        // manually add nilEvent if needed (only when pen held down)
        if ((platform->system.evtTimeOut <= 1) && (E_eTYPE_GET(&e) == penMoveEvent))
        {
          EventType e;

          _MemSet(&e, sizeof(EventType), 0);
          E_eTYPE_SET(&e, nilEvent)

          // lets flush the pen events from the queue (jebus, holding it down)
          EvtFlushPenQueue();
          EvtAddEventToQueue(&e);
        }

        if ((platform->system.ticksPerFrameE != 0) && (--bonusTick))
        {
          bonusTick = platform->system.ticksPerFrameE;
          platform->system.evtTimeOut++; // a bonus tick, to fix those timing issues
        }
      }
    }
    while (E_eTYPE_GET(&e) != appStopEvent);
  }
}

void
EndApplication()
{
  // ensure all forms are closed (must do before saving prefs)
  FrmCloseAllForms();

  // save the application preferences
  PlatformSavePreferences();
}

/*************************************************************************
 *
 * Main Entry Point
 *
 *************************************************************************/

UInt32
PilotMain(UInt16 cmd, MemPtr cmdPB, UInt16 launchFlags)
{
#ifdef _M_68K
  UInt32  romVersion;
  UInt32  displayDepths;
  UInt32  version;
  LocalID dbID;
  UInt16  card;
  GLOBALS_ACCESS;
#endif

#ifdef _M_68K
  // we need to initialize the globals data pointer
  g = (DAL_Globals *)&globals;
#endif

  // what type of launch was this?
  switch (cmd)
  {
#ifdef _M_68K
    case sysAppLaunchCmdDisplayAlarm:
         SysCurAppDatabase(&card, &dbID);
         SysUIAppSwitch(card, dbID, sysAppLaunchCmdNormalLaunch, NULL);
         break;
#endif

    case sysAppLaunchCmdNormalLaunch:

#ifdef _M_68K
         // get the rom version from the device
         FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

         // determine some screen stuff?
         displayDepths = 0x00000001;
         if (romVersion >= sysMakeROMVersion(3,0,0,sysROMStageDevelopment,0))
           WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &displayDepths, NULL);

         // LIMITATION #1: this code is not optimized for ARM devices at all (OS5+)
         if ((romVersion >= sysMakeROMVersion(5,0,0,sysROMStageDevelopment,0)) || ((displayDepths & 0x88) == 0))
         {
           FrmCustomAlert(OSIncompatibleAlert, "This application has not been optimized for your Palm OS\256 handheld, install the appropriate version for your device.", NULL, NULL);
         }
         else

         // LIMITATION #2: eek - prior to OS 3.5? due to memory requirements, this cannot be :)
         if (romVersion < sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0))
         {
           FrmCustomAlert(OSIncompatibleAlert, "Palm OS\256 version 3.5 or greater is required to run this application.", NULL, NULL);
           if (romVersion < sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0))
             AppLaunchWithCommand(sysFileCDefaultApp,
                                  sysAppLaunchCmdNormalLaunch, NULL);
         }
         else
#endif

         // we passed all the requirements - lets do it!
         {
#ifdef _M_68K
           // reset global memory space
           _MemSet(g, sizeof(DAL_Globals), 0);

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
#endif

           // application startup
           if (DAL_Initialize())
           {
             if (GlobalsInitialize())
             {
               if (DAL_ApplicationValidateDRM())
               {
                 if (ApplicationInitialize())
                 {
                   InitApplication();
                   EventLoop();
                   EndApplication();
                 }
                 ApplicationTerminate();
               }
             }
             GlobalsTerminate();

             DAL_Terminate();
           }

#ifdef _M_68K
           // notification registration for external media card removal
           if (FtrGet(sysFtrCreator, sysFtrNumNotifyMgrVersion, &version) == errNone)
           {
             SysCurAppDatabase(&card, &dbID);

             // mmc insert/removal notification
             SysNotifyUnregister(card, dbID, sysNotifyVolumeMountedEvent,  sysNotifyNormalPriority);
             SysNotifyUnregister(card, dbID, sysNotifyCardRemovedEvent,    sysNotifyNormalPriority);
             SysNotifyUnregister(card, dbID, sysNotifyDisplayResizedEvent, sysNotifyNormalPriority);
           }
#endif
         }
         break;

    default:
         break;
  }

  return errNone;
}

/*************************************************************************
 *
 * Internal Handling
 *
 *************************************************************************/

boolean
DAL_ApplicationHandleEvent(event *e)
{
  boolean processed = false;

  processed = _UIHandleEvent(e);
  if (!processed)
    processed = ApplicationHandleEvent(e);

  return processed;
}

boolean
DAL_ApplicationValidateDRM()
{
  boolean valid;

  // innocent until proven guilty?
  valid = true;

  // CARD PROTECTION:
  if (PlatformResourceExists(9999))
  {
    UInt32   version;
    Err      err;
    uint8   *info;

    // lets lock down the DRM resource
    info = (uint8 *)PlatformGetResource(9999);

    err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version);
    if (err == errNone)
    {
      Char           filename[32];
      UInt16         volRef;
      UInt32         volIterator;
      UInt32         used, size;
      FileRef        f;
      Boolean        found;
      VolumeInfoType vInfo;

      _StrCopy(filename, "/LICENSE.txt");

      // we need to assume they have not met the requirements
      valid = false;

      // loop through all the mounted volumes.
      found = false;
      volIterator = vfsIteratorStart;
      while ((volIterator != vfsIteratorStop) && (!found))
      {
        err = VFSVolumeEnumerate(&volRef, &volIterator);
        if (err == errNone)
        {
          // look for the file on this volume.
          err = VFSFileOpen(volRef, filename, vfsModeRead, &f);

          // did we find the resource?
          if (err == errNone)
          {
            VFSFileClose(f); found = true;  // found file :) thanks

            VFSVolumeInfo(volRef, &vInfo);
            VFSVolumeSize(volRef, &used, &size);

            // verification check here!
            valid |= ((vInfo.mediaType == 0x6d6d6364) && (size < (uint32)((*info+1) * 1048576L)));  // 'mmcd'
          }
        }
        else
          volIterator = vfsIteratorStop;
      }
    }

    // release the DRM resource
    PlatformReleaseResource(info);
  }

  // something went horribly wrong?
  if (!valid)
    _SysDebugMessage("DRM Validation Failed.", true);

  return valid;
}

/********************************* EOF ***********************************/
