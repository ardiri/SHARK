/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-common.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

/*************************************************************************
 *
 * FUNCTION:    MainFormEventHandler
 *
 * DESCRIPTION: The event handling routine for the _MainForm form.
 *
 * PARAMETERS:  event           - the event to process
 *
 * RETURNED:    true if event processed, false otherwise.
 *
 * REVISION HISTORY:
 *
 *************************************************************************/
static Boolean
MainFormEventHandler(EventType *event)
{
  Boolean   processed = false;
  FormType *frm;
  UInt32    version;

  switch (event->eType)
  {
    case frmOpenEvent:

         // handera - need to tell system that windows are really 240x
         if (gXXXGlobals.device_id == DEVICE_PALM_HANDERA)
           VgaFormModify(FrmGetActiveForm(), vgaFormModify160To240);

         // screen size policy handling?
         if (FtrGet(pinCreator, pinFtrAPIVersion, &version) != ftrErrNoSuchFeature)
         {
#ifdef PALM_SILK_OFF
           PINSetInputAreaState(pinInputAreaClosed);
#else
           PINSetInputAreaState(pinInputAreaOpen);
#endif
           PINSetInputTriggerState(pinInputTriggerDisabled);
         }

    case frmUpdateEvent:
         frm = FrmGetActiveForm();

         FrmDrawForm(frm);
         DALBridge(BRIDGE_GFX_REPAINT); // _GfxRepaint();

         processed = true;
         break;

    default:
         break;
  }

  return processed;
}

/*************************************************************************
 *
 *     Public Functions
 *
 *************************************************************************/

/*************************************************************************
 *
 * FUNCTION:    InitApplication
 *
 * DESCRIPTION: Perform initialization routines, load preferences etc.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    none
 *
 * REVISION HISTORY:
 *
 *************************************************************************/
void
InitApplication()
{
  UInt16 prefSize, prefSizeX;
  UInt32 appid;
  Int16  prefFlag;                                 // preferences variables
  void   *prefs;

  // read the saved preferences/saved state information
  prefs     = (void *)DALBridge(BRIDGE_APPLICATION_GET_PREFS);      // ApplicationGetPreferences();
  prefSize  = (UInt16)DALBridge(BRIDGE_APPLICATION_GET_PREFS_SIZE); // ApplicationGetPreferencesSize();
  appid     = (UInt32)DALBridge(BRIDGE_APPLICATION_GET_UNIQUE_ID);  // ApplicationGetUniqueID();
  prefSizeX = prefSize;
  if (prefSize != 0)
  {
    prefFlag = PrefGetAppPreferences(appid, 0, prefs, &prefSize, true);

    // no preferences
    if ((prefFlag == noPreferenceFound) || (prefSize != prefSizeX))
      MemSet(prefs, prefSizeX, 0); 
  }

  // we must have an active form at all times.
  FrmGotoForm(_MainForm);
}

/*************************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd            - the launch code.
 *              cmdPB          - pointer to a structure associated with
 *                               the launch code.
 *              launchFlags    - flags with information about the launch.
 *
 * RETURNED:    zero if launch successful, error code otherwise
 *
 * REVISION HISTORY:
 *
 *************************************************************************/
UInt32
PilotMain(UInt16 cmd, MemPtr cmdPB, UInt16 launchFlags)
{
  UInt32  error = errNone;
  Boolean ok;
  UInt32  romVersion;
  UInt32  displayDepths;
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

// #define CARD_LOCKING
#ifdef CARD_LOCKING
         //
         // MDM CARD PROTECTION: find /LICENSE.txt file, check card >> SIZE + TYPE <<
         //

         {
           Boolean  valid = false;
           UInt32   version;
           Err      err;
       
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

             StrCopy(filename, "/LICENSE.txt");
  
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
                   valid |= ((vInfo.mediaType == 'mmcd') && (size < 9437184)); // 9Mb
                 }
               }
               else
                 volIterator = vfsIteratorStop;
             }
           }

           // BARF!!
           if (!valid) return errNone;
         }
#endif

         // get the rom version from the device
         FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

         // determine some screen stuff?
         displayDepths = 0x00000001;
         if (romVersion >= palmos3)
           WinScreenMode(winScreenModeGetSupportedDepths,NULL,NULL,&displayDepths,NULL);

         // this architecture requires palmos 3.5+ and // 10001000 (8bpp + 4bpp)
#ifdef _68k_
         if ((romVersion >= palmos3_5) && (displayDepths & 0x88))
#else
         if ((romVersion >= palmos5)   && (displayDepths & 0x88))
#endif
         {
           DALBridgeInitialize();
           {
             MemSet(&gXXXGlobals, sizeof(XXX_GlobalsType), 0);
             MemSet(&_DALGlobals, sizeof(DAL_GlobalsType), 0);
             gDALGlobals = &_DALGlobals;
  
             DALBridge(BRIDGE_GLOBALS_INITIALIZE); // GlobalsInitialize()

             // reflect any changes to system critical values
#ifdef _arm_
             gXXXGlobals.device_id = __byte_swap32(_DALGlobals.device_id);
#else
             gXXXGlobals.device_id = _DALGlobals.device_id;
#endif
  
             ok = (Boolean)DALBridge(BRIDGE_APPLICATION_INITIALIZE); // ApplicationInitialize()
             if (ok)
             {
               // everything seemed to initialize correctly, go!
               InitApplication();
               EventLoop();
               EndApplication();
             }
             DALBridge(BRIDGE_APPLICATION_TERMINATE); // ApplicationTerminate()

             DALBridge(BRIDGE_GLOBALS_TERMINATE); // GlobalsTerminate()
           }
           DALBridgeTerminate();
         }
         else
         {
#ifdef _68k_
           FrmCustomAlert(_OSIncompatible, "3.5", NULL, NULL);
#else
           FrmCustomAlert(_OSIncompatible, "5.0", NULL, NULL);
#endif
           if (romVersion < palmos2)
             AppLaunchWithCommand(sysFileCDefaultApp,
                                  sysAppLaunchCmdNormalLaunch, NULL);
         }
         break;

    default:
         break;
  }

  return error;
}

/*************************************************************************
 *
 * FUNCTION:    EventLoop
 *
 * DESCRIPTION: The event loop for the application.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    none
 *
 * REVISION HISTORY:
 *
 *************************************************************************/
void
EventLoop()
{
  Boolean   processed = false;
  EventType e;
  event     _e;
  UInt32    timeStamp;
#ifndef PALM_TIMING_CHECK
  UInt32    bonusTick;
#endif

  // start application NOW
  MemSet(&_e, sizeof(event), 0);
#ifdef _arm_
  _e.eType = __byte_swap16(_appStartEvent);
#else
  _e.eType = _appStartEvent;
#endif
  DALBridge(BRIDGE_APPLICATION_HANDLE_EVENT, &_e); // ApplicationHandleEvent(&_e)

#ifdef PALM_TIMING_CHECK
  gXXXGlobals.frameCount     = 0;
  gXXXGlobals.timerReference = -1;
#else
  // reset the timer (just in case)
  gXXXGlobals.timerLastFrameUpdate = TimGetTicks() - gXXXGlobals.ticksPerFrame;
  bonusTick = gXXXGlobals.ticksPerFrameE;
#endif

  do
  {
#ifdef PALM_TIMING_CHECK
    EvtGetEvent(&e, 1);  // zero timeout = run the fastest we can
#else
    EvtGetEvent(&e, gXXXGlobals.evtTimeOut);
#endif
    processed = false;

    //
    // SPECIAL OS EVENTS:
    //

    // frmLoad handling (need to configure event handlers/setup)
    if (e.eType == frmLoadEvent)
    {
      UInt32   version;
      UInt16   formID = e.data.frmLoad.formID;
      FormType *frm   = FrmInitForm(formID);

      // screen size policy handling?
      if (FtrGet(pinCreator, pinFtrAPIVersion, &version) != ftrErrNoSuchFeature)
        FrmSetDIAPolicyAttr(frm, frmDIAPolicyCustom);

      FrmSetActiveForm(frm);
      FrmSetEventHandler(frm, (FormEventHandlerPtr)MainFormEventHandler);
    }

    // win entry/exit (ie: form just pop-ed up)?
    if (e.eType == winEnterEvent)
    {
      if (e.data.winEnter.enterWindow == (WinHandle)FrmGetFormPtr(_MainForm)) 
      {
        gXXXGlobals.evtTimeOut = 1;
#ifndef PALM_TIMING_CHECK
        gXXXGlobals.timerLastFrameUpdate = TimGetTicks() - gXXXGlobals.ticksPerFrame;
#endif
        FrmUpdateForm(_MainForm, frmRedrawUpdateCode);
      }
    }

    if (e.eType == winExitEvent)
    {
      if (e.data.winExit.exitWindow == (WinHandle)FrmGetFormPtr(_MainForm)) 
      {
#ifndef PALM_TIMING_CHECK
        gXXXGlobals.timerLastFrameUpdate = TimGetTicks() - gXXXGlobals.ticksPerFrame;
#endif
        gXXXGlobals.evtTimeOut = evtWaitForever;
      }
    }

    // we dont want handera devices to use the silk toggle button in any way
    if ((e.eType == keyDownEvent) && (e.data.keyDown.chr == vchrSilkToggle))
      goto ABORT_EVENT_HANDLING;

    // we have handled all display changed events
    if (e.eType == winDisplayChangedEvent)
      goto ABORT_EVENT_HANDLING;

    //
    // EVENT HANDLING:
    //

    // make sure the active window is the form
    if (WinGetActiveWindow() == (WinHandle)FrmGetFormPtr(_MainForm)) 
    {
      //
      // ANIMATION HANDLING:
      //
  
      // some events need to be 'migrated' into independent structure
      MemSet(&_e, sizeof(event), 0);
      switch (e.eType)
      {
        case nilEvent:

             timeStamp = TimGetTicks();
  
#ifndef PALM_TIMING_CHECK
             // update screen (animation) if possible
             if ((timeStamp - gXXXGlobals.timerLastFrameUpdate) >= gXXXGlobals.ticksPerFrame)
             {
               // animation requirement
               gXXXGlobals.timerLastFrameUpdate = timeStamp;
#endif

#ifdef _arm_
               _e.eType = __byte_swap16(_nilEvent);
#else
               _e.eType = _nilEvent;
#endif
               processed = true;
#ifndef PALM_TIMING_CHECK
             }
#endif
             break;
  
        case penDownEvent:
        case penMoveEvent:
             processed = (Boolean)DALBridge(BRIDGE_PEN_HANDLE_EVENT, &e.penDown, &e.screenX, &e.screenY, &_e); // _PenHandleEvent(e.penDown, e.screenX, e.screenY, &_e)
             break;
  
        case keyDownEvent:
             processed = (Boolean)DALBridge(BRIDGE_KEY_HANDLE_EVENT, &e.data.keyDown.chr, &e.data.keyDown.keyCode, &e.data.keyDown.modifiers, &_e); // _KeyHandleEvent(e.data.keyDown.chr, e.data.keyDown.keyCode, e.data.keyDown.modifiers, &_e)
             break;
  
        case appStopEvent:
#ifdef _arm_
             _e.eType = __byte_swap16(_appStopEvent);
#else
             _e.eType = _appStopEvent;
#endif
             processed = true;
             break;
  
        default:
             break;
      }
  
      // pass it on? or, handle it on device?
      if (processed)
      {
        if (!(Boolean)DALBridge(BRIDGE_APPLICATION_HANDLE_EVENT, &_e)) // ApplicationHandleEvent(&_e)
          SysHandleEvent(&e);
      }
      else 
      if (!SysHandleEvent(&e))
        FrmDispatchEvent(&e);
  
      // platform independent 'processing' of events
      while ((Boolean)DALBridge(BRIDGE_SYS_EVENT_AVAILABLE)) // _SysEventAvailable()
      {
        DALBridge(BRIDGE_SYS_GET_EVENT, &_e); // SysGetEvent(&_e)
        DALBridge(BRIDGE_APPLICATION_HANDLE_EVENT, &_e); // ApplicationHandleEvent(&_e);
  
        // application termination condition
#ifdef _arm_
        if (_e.eType == __byte_swap16(_appStopEvent)) e.eType = appStopEvent;
#else
        if (_e.eType == _appStopEvent) e.eType = appStopEvent;
#endif
      }

      //
      // ANIMATION HANDLING:
      //
  
      // reflect any changes to system critical values
#ifdef _arm_
      gXXXGlobals.evtTimeOut     = __byte_swap32(_DALGlobals.evtTimeOut);
      gXXXGlobals.ticksPerFrame  = __byte_swap32(_DALGlobals.ticksPerFrame);
      gXXXGlobals.ticksPerFrameE = __byte_swap32(_DALGlobals.ticksPerFrameE);
      gXXXGlobals.ticksPerSecond = __byte_swap32(_DALGlobals.ticksPerSecond);
#else
      gXXXGlobals.evtTimeOut     = _DALGlobals.evtTimeOut;
      gXXXGlobals.ticksPerFrame  = _DALGlobals.ticksPerFrame;
      gXXXGlobals.ticksPerFrameE = _DALGlobals.ticksPerFrameE;
      gXXXGlobals.ticksPerSecond = _DALGlobals.ticksPerSecond;
#endif
  
      gXXXGlobals.timerPoint    = TimGetTicks();
#ifdef PALM_TIMING_CHECK
      gXXXGlobals.frameCount++;
      if ((gXXXGlobals.timerReference == -1) ||
          ((gXXXGlobals.timerPoint - gXXXGlobals.timerReference) > gXXXGlobals.ticksPerSecond))
      {
        uint8 str[32];
        StrPrintF(str, "%d FPS   ", gXXXGlobals.frameCount);
        WinSetDrawWindow(WinGetDisplayWindow());
        WinDrawChars(str, StrLen(str), 0, 0);

        gXXXGlobals.timerReference = gXXXGlobals.timerPoint;
        gXXXGlobals.frameCount     = 0;
      }
#else
      if (gXXXGlobals.evtTimeOut != evtWaitForever)
      {
        // calculate the delay required
        gXXXGlobals.timerDiff  = (gXXXGlobals.timerPoint - gXXXGlobals.timerLastFrameUpdate);
        gXXXGlobals.evtTimeOut = (gXXXGlobals.timerDiff >= gXXXGlobals.ticksPerFrame) ?
                                1 : (gXXXGlobals.ticksPerFrame - gXXXGlobals.timerDiff);

        if ((gDALGlobals->ticksPerFrameE != 0) && (--bonusTick)) 
        {
          bonusTick = gXXXGlobals.ticksPerFrameE;
          gXXXGlobals.evtTimeOut++;   // a bonus tick, to fix those timing issues
        }

        // manually add nilEvent if needed (only when pen held down)
        if ((gXXXGlobals.evtTimeOut <= 1) && (e.eType == penMoveEvent))
        {
          EventType e = {};
  
          // lets flush the pen events from the queue (jebus, holding it down)
          EvtFlushPenQueue();
          EvtAddEventToQueue(&e);
        }
      }
#endif
    }
    else

    if (!SysHandleEvent(&e))
      FrmDispatchEvent(&e);

ABORT_EVENT_HANDLING:

  } while (e.eType != appStopEvent);
}

/*************************************************************************
 *
 * FUNCTION:    EndApplication
 *
 * DESCRIPTION: Perform termination routines, save preferences etc.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    none
 *
 * REVISION HISTORY:
 *
 *************************************************************************/
void
EndApplication()
{
  UInt16  prefSize;
  UInt32  appid;
  void   *prefs;

  // ensure all forms are closed (must do before saving prefs)
  FrmCloseAllForms();

  // write the saved preferences/saved state information
  prefSize = (UInt16)DALBridge(BRIDGE_APPLICATION_GET_PREFS_SIZE); // ApplicationGetPreferencesSize();
  appid    = (UInt32)DALBridge(BRIDGE_APPLICATION_GET_UNIQUE_ID);  // ApplicationGetUniqueID()
  prefs    = (void *)DALBridge(BRIDGE_APPLICATION_GET_PREFS);      // ApplicationGetPreferences();
  if (prefSize != 0)
    PrefSetAppPreferences(appid, 0, 0, prefs, prefSize, true);
}


/********************************* EOF ***********************************/
