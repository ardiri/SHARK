/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)DAL.cpp
 *
 * -- Aaron Ardiri (mailto:aaron_ardiri@mobilewizardry.com)
 */

// ---------------------------------------------------------
// Include files
// ---------------------------------------------------------

#include "DAL-prv.h"

// ---------------------------------------------------------
// Modules
// ---------------------------------------------------------

#include "DAL-platform.inc"      // platform specific functions

#include "DAL-system.inc"        // system functions
#include "DAL-memory.inc"        // memory functions
#include "DAL-io.inc"            // input/output functions
#include "DAL-string.inc"        // string functions
#include "DAL-gfx.inc"           // graphics functions
#include "DAL-audio.inc"         // audio functions
#include "DAL-file.inc"          // file i/o functions

void GlobalsInitialize()
{
  // initialize the modules
  _SysInitialize();
  _MemInitialize();
  _IOInitialize();
  _StrInitialize();
  _GfxInitialize();
  _AudioInitialize();
  _FileInitialize();

  // platform specific initialization (load fonts etc)
  PlatformInitialize();
}

void GlobalsTerminate()
{
  // platform specific shutdown (release fonts etc)
  PlatformTerminate();

  // shutdown the modules
  _FileTerminate();
  _AudioTerminate();
  _GfxTerminate();
  _StrTerminate();
  _IOTerminate();
  _MemTerminate();
  _SysTerminate();
}

// ---------------------------------------------------------
// EPOC/Symbian Interface
// ---------------------------------------------------------

GLDEF_C TInt E32Dll(TDllReason)
{
  return KErrNone;
}

EXPORT_C CApaApplication* NewApplication()
{
  return (new Application);
}

// ---------------------------------------------------------
// Application Class
// ---------------------------------------------------------

CApaDocument* Application::CreateDocumentL()
{
  CApaDocument* document = Document::NewL(*this);
  return document;
}

TUid Application::AppDllUid() const
{
  TUid id = { SymbianGetUniqueID() };
  return id;
}

// ---------------------------------------------------------
// Document Class
// ---------------------------------------------------------

Document* Document::NewL(CEikApplication& aApp)
{
  Document* self = NewLC(aApp);
  CleanupStack::Pop();
  return self;
}

Document* Document::NewLC(CEikApplication& aApp)
{
  Document* self = new (ELeave)Document(aApp);
  CleanupStack::PushL(self);
  self->ConstructL();
  return self;
}

void Document::ConstructL()
{
}

Document::Document(CEikApplication& aApp) : CEikDocument(aApp)
{
}

Document::~Document()
{
}

CEikAppUi* Document::CreateAppUiL()
{
  CEikAppUi* appUi = new (ELeave)AppUi;
  return appUi;
}

// ---------------------------------------------------------
// AppUI Class
// ---------------------------------------------------------

void AppUi::ConstructL()
{
  uint16  prefSize;
  void   *prefs;
  FILE   *fpPrefs;
  uint8   fileName[64];

  // allocate globals
  DAL_GlobalsType *gDALGlobals = (DAL_GlobalsType *)malloc(sizeof(DAL_GlobalsType));
  memset(gDALGlobals, 0, sizeof(DAL_GlobalsType));
  Dll::SetTls(gDALGlobals);

  gDALGlobals->appUi = this;

  BaseConstructL();
//#ifdef SYMB_FULLSCREEN
  // hide the status pane (we dont want this - we want full screen!)
  ((CAknAppUi*)iEikonEnv->EikAppUi())->StatusPane()->MakeVisible(EFalse);
//#endif
  iAppView = AppView::NewL(ClientRect());
  AddToStackL(iAppView);

  iSoundSystem = KeySounds();
  iSoundSystem->PushContextL(0x8cc0116); // R_AVKON_SILENT_SKEY_LIST

  // try to start the application
  if (!ApplicationInitialize()) HandleCommandL(EEikCmdExit);
  CAknAppUi::SetKeyBlockMode(ENoKeyBlock);

  // read the saved preferences/saved state information
  prefSize = ApplicationGetPreferencesSize();
  if (prefSize != 0)
  {
    prefs    = (void *)ApplicationGetPreferences();
    sprintf((char *)fileName, "c:\\%s-%s", ApplicationGetName(), DEFAULT_PREF_FILENAME);
    fpPrefs = fopen((char *)fileName, "rb");
    if (fpPrefs == NULL)
      memset(prefs, 0, prefSize);
    else
    {
      fread(prefs, prefSize, 1, fpPrefs);
      fclose(fpPrefs);
    }
  }
}

AppUi::AppUi()
{
}

AppUi::~AppUi()
{
  uint16  prefSize;
  void   *prefs;
  FILE   *fpPrefs;
  uint8   fileName[64];

  // write the saved preferences/saved state information
  prefSize = ApplicationGetPreferencesSize();
  if (prefSize != 0)
  {
    prefs    = (void *)ApplicationGetPreferences();
    sprintf((char *)fileName, "c:\\%s-%s", ApplicationGetName(), DEFAULT_PREF_FILENAME);
    fpPrefs = fopen((char *)fileName, "wb");
    if (fpPrefs != NULL)
    {
      fwrite(prefs, prefSize, 1, fpPrefs);
      fclose(fpPrefs);
    }
  }

  // shutdown the application
  ApplicationTerminate();
  iSoundSystem->PopContext();
  CAknAppUi::SetKeyBlockMode(EDefaultBlockMode);

  if (iAppView)
  {
    RemoveFromStack(iAppView);

    delete iAppView;
    iAppView = NULL;
  }
}

void AppUi::Exit()
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  gDALGlobals->appThread->CancelTimerRequest();
  CAknAppUi::Exit();
}

void AppUi::HandleCommandL(TInt aCommand)
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  switch(aCommand)
  {
    case EEikCmdExit:
    case EAknSoftkeyExit:
         {
           // pass the event onto the system for processing
           _SysEnqueueQuitEvent();
           if (gDALGlobals->sys.fps == 0)
             gDALGlobals->appThread->DelegateEvents();  // no thread, so...
         }
         break;

    default:
         break;
  }
}

void AppUi::HandleForegroundEventL(TBool aForeground)
{
  _IO_DeviceInfo *deviceP;
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  deviceP = (_IO_DeviceInfo *)gDALGlobals->io.info;

  // if we are losing focus, we gotta reset a few things
  if (!aForeground)
    deviceP->raw_key.keyState = 0;

  CAknAppUi::HandleForegroundEventL(aForeground);
}

// ---------------------------------------------------------
// AppView Class
// ---------------------------------------------------------

AppView* AppView::NewL(const TRect& aRect)
{
  AppView* self = AppView::NewLC(aRect);
  CleanupStack::Pop();
  return self;
}

AppView* AppView::NewLC(const TRect& aRect)
{
  AppView* self = new (ELeave)AppView;
  CleanupStack::PushL(self);
  self->ConstructL(aRect);
  return self;
}

void AppView::ConstructL(const TRect& aRect)
{
#include "DAL-gfx-palette.h"
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  CreateWindowL();
  SetRect(aRect);

  iAppThread = new (ELeave)AppThread(iEikonEnv->WsSession(), *(CCoeEnv::Static()->ScreenDevice()), Window());
  iAppThread->ConstructL();

  // save the size of the app view, its our DAL-gfx working area
  gDALGlobals->appView    = this;
  gDALGlobals->gfx.width  = aRect.Width();
  gDALGlobals->gfx.height = aRect.Height();
  gDALGlobals->gfx.focus  = true;
  gDALGlobals->gfx.active = false;
#ifdef GFX_DSARAW
  TScreenInfoV01 screenInfo;
  TPckg<TScreenInfoV01> sInfo(screenInfo);
  UserSvr::ScreenInfo(sInfo);

  gDALGlobals->sys.LCDpointer  = (uint16 *)screenInfo.iScreenAddress;
  gDALGlobals->sys.LCDpointer += 16;  // skip over the palette entries

  // lets offset to the window position
  TPoint pos = Window().Position();
  gDALGlobals->sys.LCDpointer += pos.iX + (pos.iY * gDALGlobals->gfx.width);
#endif

  // initialize the blitter engine (need this for palette matching)
  gDALGlobals->gfx.winBitmap = new (ELeave)CFbsBitmap;
  gDALGlobals->gfx.winBitmap->Create(TSize(gDALGlobals->gfx.width, gDALGlobals->gfx.height), EColor4K);

  // initialize the globals/modules
  GlobalsInitialize();
  _GfxSetPalette(sys_palette);  // set the default system palette

  ActivateL();
  EnableDragEvents();   // we want pointer move events
}

AppView::AppView()
{
}

AppView::~AppView()
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  if (iAppThread)
  {
    delete iAppThread;
    iAppThread = NULL;
  }
  gDALGlobals->appThread = NULL;

  // shutdown the globals/modules
  GlobalsTerminate();
  free(gDALGlobals);
  Dll::SetTls(NULL);
  User::Exit(0);            // bye!
}

void AppView::Draw(const TRect& /*aRect*/) const
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  // coming back into focus? must redraw whole screen :)
  if ((gDALGlobals->gfx.focus != IsFocused()) && (IsFocused()))
  {
    // this is the first point that we know the program is alive! start!
    if (!gDALGlobals->gfx.active)
    {
      event _e;
      memset(&_e, 0, sizeof(event));
      _e.eType = _appStartEvent;
      ApplicationHandleEvent(&_e);
    }
    gDALGlobals->gfx.active = true;

    _GfxRepaint();

    // since we are returning INTO focus, lets pause!
    {
      event e;
      memset(&e, 0, sizeof(event));
      e.eType        = _keyEvent;
      e.data.key.chr = _key_pauseSpecial;
      ApplicationHandleEvent(&e); // INFORM THE GAME ENGINE RIGHT AWAY
    }
  }

  gDALGlobals->gfx.focus  = IsFocused();
  gDALGlobals->appThread->StartInteraction();
}

void AppView::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
  boolean processed;
  event   _e;
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  processed = _PenHandleEvent((TPointerEvent *)&aPointerEvent, &_e);
  if (processed) ApplicationHandleEvent(&_e);

  // platform independent event handling (required)
  if (gDALGlobals->sys.fps == 0)
    gDALGlobals->appThread->DelegateEvents();  // no thread, so...
}

TKeyResponse AppView::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
{
  TKeyResponse result = EKeyWasNotConsumed;
  boolean      processed;
  event        _e;
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  processed = _KeyHandleEvent((TKeyEvent *)&aKeyEvent, aType, &_e);
  if (processed)
    if (ApplicationHandleEvent(&_e)) result = EKeyWasConsumed;

  // platform independent event handling (required)
  if (gDALGlobals->sys.fps == 0)
    gDALGlobals->appThread->DelegateEvents();  // no thread, so...

  return result;
}

TCoeInputCapabilities AppView::InputCapabilities() const
{
  // this class does not implement any 'unusual' input capabilities
  return TCoeInputCapabilities::ENone;
}

// ---------------------------------------------------------
// AppThread Class
// ---------------------------------------------------------

AppThread::AppThread(RWsSession& aClient, CWsScreenDevice& aScreenDevice, RWindow& aWindow)
  : CTimer(CActive::EPriorityHigh),
    iClient(aClient),
    iScreenDevice(aScreenDevice),
    iWindow(aWindow)
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  gDALGlobals->appThread = this;
}

AppThread::~AppThread()
{
  CancelTimerRequest();

  // stop and free the DSA server
  if (iDirectScreenAccess)
  {
    iDirectScreenAccess->Cancel();

    delete iDirectScreenAccess;
    iDirectScreenAccess = NULL;
  }
}

void AppThread::StartInteraction()
{
  // start the DSA server, and, draw away
  iDirectScreenAccess->StartL();
  iGc = iDirectScreenAccess->Gc();

  // start the animations
  CancelTimerRequest();
  After(TTimeIntervalMicroSeconds32(1));  // FORCE A REPAINT ASAP (NOW)
}

void AppThread::StopInteraction()
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  // terminate animations
  CancelTimerRequest();
  gDALGlobals->gfx.drawn = false;
  gDALGlobals->gfx.focus = false;

  // terminate the DSA sever
  iDirectScreenAccess->Cancel();
}

void AppThread::CancelTimerRequest()
{
  if (IsActive()) CTimer::Cancel();
}

void AppThread::DelegateEvents()
{
  event _e;
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  // no application focus? do these events later
  if (!gDALGlobals->gfx.focus) return;

  // platform independent 'processing' of events
  while (_SysEventAvailable())
  {
    _SysGetEvent(&_e);
    ApplicationHandleEvent(&_e);

    // application termination condition
    if (_e.eType == _appStopEvent)
    {
      StopInteraction();
      gDALGlobals->appUi->Exit();
    }
  }
}

void AppThread::ConstructL()
{
  CTimer::ConstructL();

  // create direct screen object
  iDirectScreenAccess = CDirectScreenAccess::NewL(iClient, iScreenDevice, iWindow, *this);

  CActiveScheduler::Add(this);
}

void AppThread::RunL()
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();
  gDALGlobals->gfx.drawn = false;

  // timer request has completed, lets post a nilEvent!
  if (iStatus == KErrNone)
  {
    event  e;
    TTime  timeA, timeB;
    TInt32 timerDiff;

    timeA.HomeTime();

    // add nilEvent to queue and process (if we have focus)
    if (gDALGlobals->gfx.focus)
    {
      _MemSet(&e, sizeof(event), 0);
      e.eType = _nilEvent;
      _SysEnqueueEvent(&e);

      // platform independent event handling (required)
      gDALGlobals->appThread->DelegateEvents();
    }

    timeB.HomeTime();
    timerDiff  = gDALGlobals->evtTimeOut - timeB.MicroSecondsFrom(timeA).Int64().Low();
    timerDiff -= 3333;                 // allow for timer reporting errors (parts of 1/100th of second)
    if (timerDiff < 1) timerDiff = 0;  // must be positive

    // put a request for the next event (if animation is active)
    After(TTimeIntervalMicroSeconds32(timerDiff));
  }
}

void AppThread::DrawToScreen(TPoint aPoint, void *bits, coord width, coord height, TRect aRect) const
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  // abort if we are not allowed to write to the display yet
  if (!gDALGlobals->gfx.active) return;

 /**
  ** Generic Palette (DAL): need to map bitmap -> iBitmap and draw iBitmap
  **/

#ifdef GFX_DSARAW
  {
    uint8  *ptr_src;
    uint16 *ptr_dst;
    uint32  size, i;
#ifndef GFX_FULLSCREEN
    uint32  pixels, j;
#endif
    uint16 *pal;
    uint32  swidth = width;
    uint32  dwidth = gDALGlobals->gfx.width;
    uint32  win1, win2;

    //
    // copy from internal buffer -> direct to screen buffer
    //

    pal = &gDALGlobals->gfx.iPaletteLookup[0];

    // setup everything we need
#ifdef GFX_FULLSCREEN
    ptr_src  = (uint8 *)bits;
    ptr_dst  = gDALGlobals->sys.LCDpointer;
    size     = (uint32)swidth * (uint32)height;

    // byte-for-word palette look up copy (use lock)
    i = size >> 3;
    do
    {
      win1 = ((uint32 *)(ptr_src))[0];
      ((uint32 *)(ptr_dst))[0] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 7) & 0x1fe)) << 16)  +
        *(uint16 *)((uint8 *)(pal) + ((win1 & 0xff) << 1));
      ((uint32 *)(ptr_dst))[1] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 23) & 0x1fe)) << 16) +
        *(uint16 *)((uint8 *)(pal) + ((win1 >> 15) & 0x1fe));
      win1 = ((uint32 *)(ptr_src))[1];
      ptr_src += 8;
      ((uint32 *)(ptr_dst))[2] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 7) & 0x1fe))  << 16) +
        *(uint16 *)((uint8 *)(pal) + ((win1 & 0xff) << 1));
      ((uint32 *)(ptr_dst))[3] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 23) & 0x1fe)) << 16) +
        *(uint16 *)((uint8 *)(pal) + ((win1 >> 15) & 0x1fe));
      ptr_dst += 8;
    }
    while (--i);
#else
    ptr_src  = (uint8 *)bits;
    ptr_src += aRect.iTl.iX + (aRect.iTl.iY * swidth);
    ptr_dst  = gDALGlobals->sys.LCDpointer;
    ptr_dst += aPoint.iX + (aPoint.iY * dwidth);
    size     = (uint32)swidth * (uint32)height;

    //
    // NOTE: no bounds checking here, developer shouldn't mess it up!
    //

    pixels  = aRect.iBr.iX - aRect.iTl.iX;
    j       = aRect.iBr.iY - aRect.iTl.iY;

    swidth -= pixels;
    dwidth -= pixels;
    do
    {
      i = pixels;

      win2 = (4 - ((uint32)(ptr_src))) & 0x3;
      if (win2)
      {
        i -= win2;
        do
        {
          win1 = *ptr_src++;
          *ptr_dst++ = *(pal + win1);
        }
        while (--win2);
      }

      // chunks of 8 writes (faster)
      win2 = i & 0x7;
      if (i >>= 3)
      {
        do 
        {
          win1 = ((uint32 *)(ptr_src))[0];
          ((uint32 *)(ptr_dst))[0] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 7) & 0x1fe)) << 16)  +
            *(uint16 *)((uint8 *)(pal) + ((win1 & 0xff) << 1));
          ((uint32 *)(ptr_dst))[1] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 23) & 0x1fe)) << 16) +
            *(uint16 *)((uint8 *)(pal) + ((win1 >> 15) & 0x1fe));
          win1 = ((uint32 *)(ptr_src))[1];
          ptr_src += 8;
          ((uint32 *)(ptr_dst))[2] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 7) & 0x1fe))  << 16) +
            *(uint16 *)((uint8 *)(pal) + ((win1 & 0xff) << 1));
          ((uint32 *)(ptr_dst))[3] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 23) & 0x1fe)) << 16) +
            *(uint16 *)((uint8 *)(pal) + ((win1 >> 15) & 0x1fe));
          ptr_dst += 8;
        }
        while (--i);
      }

      if (win2)
      {
        do
        {
          win1 = *ptr_src++;
          *ptr_dst++ = *(pal + win1);
        }
        while (--win2);
      }

      ptr_src += swidth;
      ptr_dst += dwidth;
    }
    while (--j);
#endif

    // force changes to real display
    TRawEvent redraw;
    redraw.Set(TRawEvent::ERedraw);
    UserSvr::AddEvent(redraw);
  }
#else
  {
    uint8  *ptr_src;
    uint16 *ptr_dst;
    uint32  size, i;
#ifndef GFX_FULLSCREEN
    uint32  pixels, j;
#endif
    uint16 *pal;
    uint32  swidth = width;
    uint32  dwidth = gDALGlobals->gfx.width;
    uint32  win1, win2;

    //
    // copy from internal buffer -> offscreen window
    //

    pal = &gDALGlobals->gfx.iPaletteLookup[0];

    // setup everything we need
#ifdef GFX_WINDOWSYNC
    TBitmapUtil bitmapUtil = TBitmapUtil(gDALGlobals->gfx.winBitmap);
    bitmapUtil.Begin(TPoint(0,0));  // lock bitmap data
#endif

#ifdef GFX_FULLSCREEN
    ptr_src = (uint8 *)bits;
    ptr_dst = (uint16 *)gDALGlobals->gfx.winBitmap->DataAddress();
    size    = (uint32)swidth * (uint32)height;

    // byte-for-word palette look up copy (use lock)
    i = size >> 3;
    do
    {
      win1 = ((uint32 *)(ptr_src))[0];
      ((uint32 *)(ptr_dst))[0] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 7) & 0x1fe)) << 16)  +
        *(uint16 *)((uint8 *)(pal) + ((win1 & 0xff) << 1));
      ((uint32 *)(ptr_dst))[1] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 23) & 0x1fe)) << 16) +
        *(uint16 *)((uint8 *)(pal) + ((win1 >> 15) & 0x1fe));
      win1 = ((uint32 *)(ptr_src))[1];
      ptr_src += 8;
      ((uint32 *)(ptr_dst))[2] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 7) & 0x1fe))  << 16) +
        *(uint16 *)((uint8 *)(pal) + ((win1 & 0xff) << 1));
      ((uint32 *)(ptr_dst))[3] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 23) & 0x1fe)) << 16) +
        *(uint16 *)((uint8 *)(pal) + ((win1 >> 15) & 0x1fe));
      ptr_dst += 8;
    }
    while (--i);
#else
    ptr_src  = (uint8 *)bits;
    ptr_src += aRect.iTl.iX + (aRect.iTl.iY * swidth);
    ptr_dst  = (uint16 *)gDALGlobals->gfx.winBitmap->DataAddress();
    ptr_dst += aPoint.iX + (aPoint.iY * dwidth);
    size     = (uint32)swidth * (uint32)height;

    //
    // NOTE: no bounds checking here, developer shouldn't mess it up!
    //

    pixels  = aRect.iBr.iX - aRect.iTl.iX;
    j       = aRect.iBr.iY - aRect.iTl.iY;

    swidth -= pixels;
    dwidth -= pixels;
    do
    {
      i = pixels;

      win2 = (4 - ((uint32)(ptr_src))) & 0x3;
      if (win2)
      {
        i -= win2;
        do
        {
          win1 = *ptr_src++;
          *ptr_dst++ = *(pal + win1);
        }
        while (--win2);
      }

      // chunks of 8 writes (faster)
      win2 = i & 0x7;
      if (i >>= 3)
      {
        do 
        {
          win1 = ((uint32 *)(ptr_src))[0];
          ((uint32 *)(ptr_dst))[0] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 7) & 0x1fe)) << 16)  +
            *(uint16 *)((uint8 *)(pal) + ((win1 & 0xff) << 1));
          ((uint32 *)(ptr_dst))[1] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 23) & 0x1fe)) << 16) +
            *(uint16 *)((uint8 *)(pal) + ((win1 >> 15) & 0x1fe));
          win1 = ((uint32 *)(ptr_src))[1];
          ptr_src += 8;
          ((uint32 *)(ptr_dst))[2] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 7) & 0x1fe))  << 16) +
            *(uint16 *)((uint8 *)(pal) + ((win1 & 0xff) << 1));
          ((uint32 *)(ptr_dst))[3] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 23) & 0x1fe)) << 16) +
            *(uint16 *)((uint8 *)(pal) + ((win1 >> 15) & 0x1fe));
          ptr_dst += 8;
        }
        while (--i);
      }

      if (win2)
      {
        do
        {
          win1 = *ptr_src++;
          *ptr_dst++ = *(pal + win1);
        }
        while (--win2);
      }

      ptr_src += swidth;
      ptr_dst += dwidth;
    }
    while (--j);
#endif

#ifdef GFX_WINDOWSYNC
    bitmapUtil.End();
#endif

    //
    // copy to display buffer
    //

#ifdef GFX_WINDOWSYNC
    TRect rect = TRect(iWindow.Size());
    iWindow.Invalidate(rect);
    iWindow.BeginRedraw(rect);
#endif

    iGc->BitBlt(aPoint, gDALGlobals->gfx.winBitmap, aRect);

#ifdef GFX_WINDOWSYNC
    iWindow.EndRedraw();
#endif
  }

  // flush the screen
  iDirectScreenAccess->ScreenDevice()->Update();
  iClient.Flush();
#endif

  // yes, we have drawn to the screen now
  gDALGlobals->gfx.drawn = true;
}

void AppThread::Restart(RDirectScreenAccess::TTerminationReasons /*aReason*/)
{
  // we do nothing
}

void AppThread::AbortNow(RDirectScreenAccess::TTerminationReasons /*aReason*/)
{
  // terminate the thread
  StopInteraction();
}

// ---------------------------------------------------------
// ToneGenerator Class
// ---------------------------------------------------------

ToneGenerator* ToneGenerator::NewL(TInt freq, TInt duration, TInt volume)
{
  ToneGenerator* self = ToneGenerator::NewLC(freq, duration, volume);
  CleanupStack::Pop();
  return self;
}

ToneGenerator* ToneGenerator::NewLC(TInt freq, TInt duration, TInt volume)
{
  ToneGenerator* self = new (ELeave)ToneGenerator;
  CleanupStack::PushL(self);
  self->ConstructL(freq, duration, volume);
  return self;
}

ToneGenerator::~ToneGenerator()
{
  delete iCMdaAudioToneUtility;    
  iCMdaAudioToneUtility = NULL;
}

void ToneGenerator::ConstructL(TInt freq, TInt duration, TInt volume)
{
  playVolume = volume;
  iCMdaAudioToneUtility = CMdaAudioToneUtility::NewL(*this);
  iCMdaAudioToneUtility->PrepareToPlayTone(freq, TTimeIntervalMicroSeconds(duration));
}

ToneGenerator::ToneGenerator()
{
}

void ToneGenerator::MatoPrepareComplete(TInt aError)
{
  if (!aError)
  {
    iCMdaAudioToneUtility->SetVolume((TInt)(iCMdaAudioToneUtility->MaxVolume() * playVolume) / 100);
    iCMdaAudioToneUtility->SetPriority(0, EMdaPriorityPreferenceTime);
    iCMdaAudioToneUtility->Play();
  }
}

void ToneGenerator::MatoPlayComplete(TInt aError)
{
}

/********************************* E O F *********************************/
