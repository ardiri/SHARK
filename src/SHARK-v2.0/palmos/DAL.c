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

// globally shared information
#define g_initialized  g -> initialized

#define g_lcd_a        g -> lcd_a
#define g_lcd_r        g -> lcd_r
#define g_lcd_g        g -> lcd_g
#define g_lcd_b        g -> lcd_b
#define g_lcd_r_mask   g -> lcd_r_mask
#define g_lcd_g_mask   g -> lcd_g_mask
#define g_lcd_b_mask   g -> lcd_b_mask

#define LCD_DEPTH (uint8)(((g_lcd_a+g_lcd_r+g_lcd_g+g_lcd_b) + 15) & ~0xf)

#define g_width        g -> width
#define g_width_rb     g -> width_rb
#define g_height       g -> height
#define g_height_rb    g -> height_rb

#define g_path         g -> path
#define g_path_vol     g -> path_vol

#define g_config       g -> config

extern uint32 PalmOSGetUniqueID();

/*************************************************************************
 *
 * Local Dependencies
 *
 *************************************************************************/

#if (LOG_DISABLE == 0)
static void    _LOGInitialize();
static void    _LOGCreateEntry(char  *str);
static void    _LOGTerminate();
#endif

static void    DAL_DIAInitialize();
static void    DAL_DIAResize(FormType *, boolean);
static void    DAL_DIATerminate();
static Err     DAL_DIANotifyCallback(SysNotifyParamType *);

static void    DAL_BuildPath();

static boolean DAL_HandleEvent(event *);
static void    DAL_DispatchEvents();

#include "DAL-missing.inc"

#include "DAL-memory.inc"
#include "DAL-display.inc"
#include "DAL-system.inc"
#include "DAL-library.inc"
#include "DAL-input.inc"
#include "DAL-preferences.inc"
#include "DAL-file.inc"
#include "DAL-audio.inc"
#include "DAL-resource.inc"

/*************************************************************************
 *
 * Platform Depending Application Framework
 *
 *************************************************************************/

static void    InitApplication();
static void    EventLoop();
static void    QuitApplication();

static Boolean AppHandleEvent(EventType *);
static Boolean MainFormHandleEvent(EventType *);

#if (LOG_DISABLE == 0)
// initialize the logging process
static void
_LOGInitialize()
{
}

// create an entry in the log file
static void
_LOGCreateEntry(char *str)
{
}

// terminate the logging process; close log file
static void
_LOGTerminate()
{
}
#endif

static void
DAL_DIAInitialize()
{
  LocalID dbID;
  UInt32  version;
  Err     err;
 
  // initialized
  g -> ftr.pin_version = 0;

  // we need to get the localid of this application
  SysCurAppDatabase(&dbID);

  // PalmSource PINMgr:
  err = FtrGet(pinCreator, pinFtrAPIVersion, &version);
  if (err == errNone)
  {
    // we have issues with these devices, run in legacy (320x320 mode)
    if (((g -> ftr.comID == 'Palm') && (g -> ftr.devID == 'Arz1')) || 
        ((g -> ftr.comID == 'Tpwv') && (g -> ftr.devID == 'Rdog')) ||
        ((g -> ftr.comID == 'grmn') && (g -> ftr.devID == '3600')) ||
        ((g -> ftr.comID == 'grmn') && (g -> ftr.devID == '3700')))
    {
      g -> ftr.pin_version = 0;
    }
    else
    {
      g -> ftr.pin_version = version;

      err = SysNotifyRegister(dbID, sysNotifyDisplayResizedEvent, 
                              DAL_DIANotifyCallback,
                              sysNotifyNormalPriority, NULL);

      // PIN* are OS 5.3+ API (r9 table) must use pace for < 5.3
      g -> PACE.dia_api = ((g -> ftr.osVer & 0x0ff00000) < 0x05300000);
    }
  }

// TODO:
// -----

  // Sony    SilkLib:      (Sony Clie ARM)

  // we need to know the screen density (for later)
  WinScreenGetAttribute(winScreenDensity, &g -> ftr.scr_density);
}

static void
DAL_DIAResize(FormType *frm, boolean draw)
{
  RectangleType bounds;
  Coord         extentX, extentY;
  WinHandle     oldWin;
  event         e;

  // resize the form
  oldWin = WinSetDrawWindow(FrmGetWindowHandle(frm));
  WinSetCoordinateSystem(kCoordinatesNative);
  WinGetDisplayExtent(&extentX, &extentY);
  if (g -> ftr.pin_version == 0)
  {
    extentX = MIN(320, extentX);
    extentY = MIN(320, extentY);  // force max 320x320 if no PINS
  }
  FrmGetFormBounds(frm, &bounds);
  bounds.extent.x = extentX;
  bounds.extent.y = extentY;
  WinSetBounds(FrmGetWindowHandle(frm), &bounds);
  WinSetCoordinateSystem(kCoordinatesStandard);
  WinSetDrawWindow(oldWin);

  // these are our new dimensions
  g_width     = extentX;
  g_width_rb  = (uint16)(((g_width  + 3) & ~3) * (LCD_DEPTH >> 3));
  g_height    = extentY;
  g_height_rb = (uint16)(((g_height + 3) & ~3) * (LCD_DEPTH >> 3));

  // we need to re-initialize the SHARK LCD driver routines
  _LCDTerminate_RESIZE();
  _LCDInitialize_RESIZE();

  // now, the application must handle the new LCD instance
  memset(&e, 0, sizeof(event));
  e.eType        = _keyEvent;
  e.data.key.chr = _key_gfx_change;
  DAL_HandleEvent(&e);

  // draw the form
  if (draw) FrmDrawForm(frm);
  _LCDRepaint();

}

static void
DAL_DIATerminate()
{
  LocalID dbID;
  Err     err;

  // we need to get the localid of this application
  SysCurAppDatabase(&dbID);

  // PalmSource PINMgr:
  if (g -> ftr.pin_version > 0)
  {
    err = SysNotifyUnregister(dbID, sysNotifyDisplayResizedEvent, 
                              sysNotifyNormalPriority);
  }
}

static Err
DAL_DIANotifyCallback(SysNotifyParamType *notifyParams)
{
  EventType e;

  memset(&e, 0, sizeof(EventType));
  e.eType = winDisplayChangedEvent_CUSTOM;
  EvtAddUniqueEventToQueue(&e, 0, true);

  return errNone;
}

static void
DAL_BuildPath()
{
  Err     err;
  UInt32  version, volIterator;
  UInt16  volRef;
  FileRef f;

  // defaults = no directory
  strcpy(g_path, "");

  // lets determine if there is a   
  err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version);
  if (err == errNone)
  {
    volIterator = vfsIteratorStart;
    while (volIterator != vfsIteratorStop)
    {
      if (VFSVolumeEnumerate(&volRef, &volIterator) == errNone)
      {
        if (VFSFileOpen(volRef, "Palm/Launcher", vfsModeRead, &f) == errNone)
        {
          strcpy(g_path, "/Palm");
          g_path_vol = volRef;
          
          VFSFileClose(f);
          
          goto BUILD_PATH_BREAK;
        }
      }
      else
        volIterator = vfsIteratorStop;
    }
  }
  
BUILD_PATH_BREAK:;
}

static boolean
DAL_HandleEvent(event *e)
{
  boolean processed;

  processed = _LibHandleEvent(e);
  if (!processed)
    processed = ApplicationHandleEvent(e);

  return processed;
}

static void
DAL_DispatchEvents()
{
  event _e;
  int16 eType;

  // platform independent 'processing' of events
  while (_SysEventAvailable()) 
  {
    _SysGetEvent(&_e);
    eType = _e.eType;
    DAL_HandleEvent(&_e);

    // application termination condition
    if (eType == _appStopEvent) 
    { 
      // we must break our event loop, now!
      g_initialized = false;
    }
  }
}

static boolean
GlobalsInitialize()
{
  boolean result;

  // initialize all sub-components
  result = true;

  // initialize all sub-components
  result = true;
  result &= _LCDInitialize();
  result &= _SysInitialize();
  result &= _LibInitialize();
  result &= _InputInitialize();
  result &= _PreferencesInitialize();
  result &= _ResourceInitialize();
  
  // these two components are "optional"
  _FileInitialize();
  _SndInitialize();
  
  result &= _MemInitialize();

  // initialize the default orientation state based on window resolution
  _LCDSetOrientation(lcd_rotate0);

  // load any additional libraries (all optional)
  if (g_config.gfx.active)
    result &= (_GfxInitialize(&g_config.gfx.format) || !g_config.gfx.required);
  if ((g_config.font.active) && (g_config.gfx.active))
    result &= (_FntInitialize(&g_config.gfx.format,
                               g_config.font.encoding,
                               g_config.font.style) || !g_config.font.required);
  if (g_config.sfx.active)
    result &= (_SfxInitialize(g_config.sfx.rate,
                              g_config.sfx.format,
                              g_config.sfx.type,
                              g_config.sfx.frames,
                              g_config.sfx.mix_count) || !g_config.sfx.required);

  return result;
}

static void
GlobalsTerminate()
{
  // shutdown any additional libraries
  if (g_config.sfx.active)   _SfxTerminate();
  if ((g_config.font.active) &&
      (g_config.gfx.active)) _FntTerminate();
  if  (g_config.gfx.active)  _GfxTerminate();

  _MemTerminate();

  // shutdown all sub-components
  _SndTerminate();
  _FileTerminate();

  _ResourceTerminate();
  _PreferencesTerminate();
  _InputTerminate();
  _LibTerminate();
  _SysTerminate();
  _LCDTerminate();             // REQ: must be done second last
}

static void 
InitApplication()
{
  _PreferencesLoad();

  FrmGotoForm(MainForm);
}

static Boolean 
AppHandleEvent(EventType *eventP)
{
  Boolean       processed;
  FormType     *frm;
  UInt16        frmID;
  event         e;

  processed = false;
  switch (eventP -> eType)
  {
    case frmLoadEvent:
         frmID = eventP -> data.frmLoad.formID;
         frm   = FrmInitForm(frmID);
         FrmSetActiveForm(frm);

         // set the size constraints of the form (allow all sizes)
         if (g -> ftr.pin_version > 0)
         {
           if (g -> PACE.dia_api)
           {
             WinSetConstraintsSize_PACE(WinGetDisplayWindow(), 160, 160, 240, 160, 160, 240);
             FrmSetDIAPolicyAttr_PACE(frm, frmDIAPolicyCustom);
             PINSetInputTriggerState_PACE(pinInputTriggerEnabled);
           }
           else
           {
             WinSetConstraintsSize(WinGetDisplayWindow(), 160, 160, 240, 160, 160, 240);
             FrmSetDIAPolicyAttr(frm, frmDIAPolicyCustom);
             PINSetInputTriggerState(pinInputTriggerEnabled);
           }
         }

         if (frmID == MainForm)
           FrmSetEventHandler(frm, MainFormHandleEvent);

         processed = true;
         break;

    case penDownEvent:
    case penMoveEvent:
         processed = _PenHandleEvent(eventP -> screenX, eventP -> screenY, &e);
         if (processed) _SysEnqueueEvent(&e);
         break;

    case keyDownEvent:
         if (eventP -> data.keyDown.chr == chrNavChange)
         {
           e.eType        = _keyEvent;
           e.data.key.chr = _key_unknown;
           if (eventP -> data.keyDown.keyCode & navBitLeft)   e.data.key.chr = _key_padleft;
           if (eventP -> data.keyDown.keyCode & navBitRight)  e.data.key.chr = _key_padright;
           if (eventP -> data.keyDown.keyCode & navBitSelect) e.data.key.chr = _key_padselect;
           if (e.data.key.chr != _key_unknown) _SysEnqueueEvent(&e);
           processed = true;
         }
         else
         {
           processed = _KeyHandleEvent(eventP -> data.keyDown.chr, &e);
           if (processed) _SysEnqueueEvent(&e);
         }
         break;

    case winEnterEvent:

         // ensure that the input trigger is enabled
         if (g -> ftr.pin_version > 0)
         {
           if (g -> PACE.dia_api)
             PINSetInputTriggerState_PACE(pinInputTriggerEnabled);
           else
             PINSetInputTriggerState(pinInputTriggerEnabled);
         }

         _LCDResume();
         processed = true;
         break;

    case winExitEvent:

         _LCDSuspend();
         processed = true;
         break;

    case appStopEvent:
         _SysEnqueueQuitEvent();
         processed = true;
         break;

    default:
         break;
  }

  return processed;
}

static Boolean 
MainFormHandleEvent(EventType *eventP)
{
  Boolean   processed;
  FormType *frm;

  processed = false;
  switch (eventP -> eType)
  {
    case frmOpenEvent:
         frm = FrmGetActiveForm();
         DAL_DIAResize(frm, true);
         processed = true;
         break;

    case nilEvent:
         processed = true;
         break;

    default:
         break;
  }

  return processed;
}

static void
EventLoop()
{
  EventType  e;
  event     _e;

  // basic initialization (and, loading screen)
  memset(&_e, 0, sizeof(event));
  _e.eType = _appStartEvent;
  DAL_HandleEvent(&_e);

  // application is ready to startup - can do stuff like loading here
  memset(&_e, 0, sizeof(event));
  _e.eType = _appGoEvent;
  DAL_HandleEvent(&_e);

  do
  {
    EvtGetEvent(&e, 1); 

    // handle any resizing events
    if (e.eType == winDisplayChangedEvent_CUSTOM)
      DAL_DIAResize(FrmGetActiveForm(), true);
      
    // handle the event
    if (!AppHandleEvent(&e))
      if (!SysHandleEvent(&e))
        FrmDispatchEvent(&e);

    // platform independent 'processing' of events
    DAL_DispatchEvents();
  }
  while ((e.eType != appStopEvent) && (g_initialized));
}

static void 
QuitApplication()
{
  FrmCloseAllForms();

  _PreferencesSave();
}

UInt32
PilotMain(UInt16 cmd, MemPtr cmdPB, UInt16 launchFlags)
{
  uint32  result;
  Coord   extentX, extentY;
  UInt32  depth;

  // lets assume everything will run ok
  result = errNone;
  
  // what type of launch was this?
  switch (cmd)
  {
    case sysAppLaunchCmdNormalLaunch:
 
         // switch to 16bpp mode
         depth = 16;
         WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);

         // obtain the application configuration parameters
         memset(&g_config, 0, sizeof(application_config));
         ApplicationConfig(&g_config);

         DAL_BuildPath();

         // obtain os version + device + company id's
         FtrGet(sysFtrCreator, sysFtrNumROMVersion,   &g -> ftr.osVer);
         FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &g -> ftr.comID);
         FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID,  &g -> ftr.devID);

         // initialized support for DIA
         DAL_DIAInitialize();

         // this is our "setting" for our LCD engine (hard coded)
         g_lcd_a = 0;
         g_lcd_r = 5;
         g_lcd_g = 6;
         g_lcd_b = 5;
         g_lcd_r_mask = ((0xffL >> (8 - g_lcd_r)) << (g_lcd_g + g_lcd_b));
         g_lcd_g_mask = ((0xffL >> (8 - g_lcd_g)) << (g_lcd_b));
         g_lcd_b_mask =  (0xffL >> (8 - g_lcd_b));

#if (LOG_DISABLE == 0)
         // initialize the log file function
         _LOGInitialize();
#endif

         // standard default window size
         WinSetCoordinateSystem(kCoordinatesNative);
         WinGetDisplayExtent(&extentX, &extentY);
         if (g -> ftr.pin_version == 0)
         {
           extentX = MIN(320, extentX);
           extentY = MIN(320, extentY);  // force max 320x320 if no PINS
         }
         WinSetCoordinateSystem(kCoordinatesStandard);
         g_width     = extentX;
         g_width_rb  = (uint16)(((g_width  + 3) & ~3) * (LCD_DEPTH >> 3));
         g_height    = extentY;
         g_height_rb = (uint16)(((g_height + 3) & ~3) * (LCD_DEPTH >> 3));

#if (LOG_DISABLE == 0)
         // LOGGING EVENT
         _LOGCreateEntry("\n:: - hardware abstraction initialize -\n");
#endif

         // application startup
         g_initialized = false;
         if (GlobalsInitialize())
         {
           uint32  diff;
           boolean mem_ok;
            
#if (LOG_DISABLE == 0)
           // LOGGING EVENT
           _LOGCreateEntry("\n:: - application initialize -");
#endif

           // memory failure conditions
           mem_ok = true;
           mem_ok &= (g -> ftr.mem_dynheap_available > g_config.memory.min_heap);
           if (g -> ftr.mem_storage_available < g_config.memory.min_storage)
           {
             diff = g_config.memory.min_storage - g -> ftr.mem_storage_available;
             mem_ok &= (g -> ftr.mem_dynheap_available > (g_config.memory.min_heap + diff));
           }

           // if there are no problems with memory, we can continue
           if (mem_ok)
           {
             if (ApplicationInitialize())
             {
               g_initialized = true;
  
               InitApplication();
               EventLoop();
               QuitApplication();
  
               g_initialized = false;
             }
              
             ApplicationTerminate();
           }
           else result = 0x20000000;    // SHARK: memory requirements not met
           
#if (LOG_DISABLE == 0)
           // LOGGING EVENT
           _LOGCreateEntry(":: - application terminate -\n");
#endif
         }
         else result = 0x10000000;      // SHARK: globals failed to initialize
         GlobalsTerminate();

#if (LOG_DISABLE == 0)
         // LOGGING EVENT
         _LOGCreateEntry("\n:: - hardware abstraction terminate -");

         // shutdown the log file function
         _LOGTerminate();
#endif

         // terminate DIA support
         DAL_DIATerminate();

         // restore display mode
         WinScreenMode(winScreenModeSetToDefaults, NULL, NULL, NULL, NULL);
         break;

    default:
         break;
  }

  return result;
}

/********************************* EOF ***********************************/
